<?php
class Record2Sql {
    private $log = null;
    private static $defineArr = [];
    private static $fieldArr = [];
    private static $erlangNotesArr = [];
    private static $recordTableArr = [];
    private static $fieldDefaultType = 'varchar(255)';
    private static $splitStr = ",\r\n";
    private static $wrapTag = "\r\n";
    private static $replaceTypeFuc = [
        'self_varchar' => 'varchar:100',
        'self_char' => 'char:100',
        'self_bigint' => 'bigint',
        'self_int' => 'int',
        'self_tinyint' => 'tinyint',
        'self_string' => 'varchar:100',
        'self_text' => 'text',
        'self_float' => 'float',
        'self_double' => 'double',
        'self_long' => 'bigint',
        'is_float' => 'float',
        'is_double' => 'double',
        'is_int' => 'int',
        'is_numeric' => 'int',
        'is_string' => 'text',
    ];
    private static $errorMsg = [];
    private static $defaultKeyWord = [
        'define' => '-define',
        'field' => '-field'
    ];
    private static $recordTempStorageArea = [];
    private static $defaultTableField = [
        'pid' => [
            'type' => 'bigint',
            'comment' => 'PID'
        ],
        'agent_id' => [
            'type' => 'int',
            'comment' => '代理ID'
        ],
        'server_id' => [
            'type' => 'int',
            'comment' => '区服ID'
        ]
    ];
    private static $tableKey = [
        'pid' => 'PRIMARY KEY (`pid`)',
        'role_id' => 'KEY `role_id`(`role_id`)',
        'mtime' => 'KEY `mtime`(`mtime`)'
    ];
    private static $tableTemplate =
<<<EOT
CREATE TABLE IF NOT EXISTS %s (
%s
  %s
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='%s';
EOT;

    private static $hqlTemplate =
<<<EOT
CREATE EXTERNAL TABLE IF NOT EXISTS %s (
%s
)
  COMMENT '%s'
PARTITIONED BY (
  sdt STRING COMMENT '服务端时间分区，按天的分区: 格式 yyyy-MM-dd'
)
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\\t'
STORED AS TEXTFILE
;
EOT;

    public function __construct(Log $log)
    {
        $this->log = $log;
    }

    /**
     * 函数入口
     * @param $filePath recode 文件
     * @param $sqlFile 输出的sql 文件
     * @param $hqlFile 输出的hql 文件
     * @param $database 数据库名称
     * @param $tablePrefix 表前缀
     * @return void
     */
    public function start($filePath, $sqlFile, $hqlFile, $database, $tablePrefix)
    {
        if (!file_exists($filePath)) {
            $this->log->echoErrorInfo('文件不存在，请重新操作');
        }

        try {
            $recordArr = file($filePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            $this->handleRecordArr($recordArr);
            $this->handleDefineFieldData();

            // 数据表语句生成
            $this->createSql($tablePrefix, $sqlFile);
            $this->createHql($tablePrefix, $hqlFile, $database);

            if (!empty(self::$errorMsg)) {
                // 错误信息收集
                $this->log->handleErrorMsg(self::$errorMsg);
            }

        } catch(Exception $e) {
            $this->log->echoErrorInfo($e->getMessage());
        }
    }

    /**
     * 生成数据表
     * @param $tablePrefix 表前缀
     * @param $sqlFile sql文件
     * @return void
     */
    private function createSql($tablePrefix, $sqlFile)
    {
        if (empty(self::$fieldArr) && empty(self::$recordTableArr)) {
            return false;
        }

        $echoStr = '';
        foreach (self::$recordTableArr as $recordName => $recordData) {
            $keyIndexes = [];
            $tmpStr = '';
            foreach ($recordData['record'] as $rName => $rInfo) {
                if (isset(self::$tableKey[$rName])) {
                    $keyIndexes[$rName] = self::$tableKey[$rName];
                }

                $rInfo = $this->changeRecordValueForField($rName, $rInfo, $recordName);
                $typeInfo = strtoupper((isset($rInfo['accuracy']) && $rInfo['accuracy']) ?
                    $rInfo['type'].'('.$rInfo['accuracy'].')': $rInfo['type']);
                $tmpStr .= "  `{$rName}` {$typeInfo} NOT NULL COMMENT '{$rInfo['comment']}'".self::$splitStr;
            }

            $tableKey = implode(self::$splitStr."  ", $keyIndexes);
            $tmpStr = substr($tmpStr, 0, strlen($tmpStr) - (empty($keyIndexes) ? strlen(self::$splitStr) : strlen(self::$wrapTag)));
            $echoStr .= sprintf(self::$tableTemplate,'`'.$tablePrefix.$recordName.'`', $tmpStr, $tableKey, $recordData['record_comment']).str_repeat(self::$wrapTag, 2);
        }

        echo "生成sql文件：" . $sqlFile . PHP_EOL;
        file_put_contents($sqlFile, $echoStr);
        echo $echoStr.PHP_EOL;
    }

     /**
     * 生成数据表
     * @param $tablePrefix 表前缀
     * @param $hqlFile hql文件
     * @param $database 数据库名字
     * @return void
     */
    private function createHql($tablePrefix, $hqlFile, $database)
    {
        if (empty(self::$fieldArr) && empty(self::$recordTableArr)) {
            return false;
        }

        $echoStr = '';
        $keyIndexes = [];
        foreach (self::$recordTableArr as $recordName => $recordData) {
            $tmpStr = '';
            foreach ($recordData['record'] as $rName => $rInfo) {
                $rInfo = $this->changeRecordValueForField($rName, $rInfo, $recordName);
                $typeInfo = strtoupper($rInfo['type']);
                $tmpStr .= "  {$rName} {$typeInfo} COMMENT '{$rInfo['comment']}'".self::$splitStr;
            }

            $tmpStr = substr($tmpStr, 0, strlen($tmpStr) - (empty($keyIndexes) ? strlen(self::$splitStr) : strlen(self::$wrapTag)));
            $tmpStr = str_replace(['VARCHAR', 'CHAR', 'TEXT', 'LONGTEXT', 'MEDIUMTEXT'], 'STRING', $tmpStr);
            $echoStr .= sprintf(self::$hqlTemplate, $database . '.' . $tablePrefix . $recordName, $tmpStr, $recordData['record_comment']).str_repeat(self::$wrapTag, 2);
        }

        echo "生成hql文件：" . $hqlFile . PHP_EOL;
        file_put_contents($hqlFile, $echoStr);
        echo $echoStr.PHP_EOL;
    }

    /**
     * 使用field 替换 record 中的数值
     * @param $rName 字段名称
     * @param $rInfo 字段数据。类型;精度;注释
     * @param $recordName recode 名称
     * @return 返回替换后的 字段数据
     */
    private function changeRecordValueForField($rName, $rInfo, $recordName)
    {
        if (!isset(self::$fieldArr[$rName])) {
            if (!$rInfo['type']) {
                $rInfo['type'] = self::$fieldDefaultType;
                $this->insertErrorMsg('lackOfDataValueTypes', $recordName, $rName);
            }
            return $rInfo;
        }

        @list($fieldComment, $fieldDataType) = explode(';', self::$fieldArr[$rName]);
        $fieldDataType = trim($fieldDataType);
        @list($fieldType, $fieldAccur) = explode(':', $fieldDataType);
        if (!empty($fieldType)) {
            $rType = $rInfo['type'];
            $rInfo = $this->dataTeypDetection($fieldType, is_null($fieldAccur) ? false : $fieldAccur );
            if ($rType && $rInfo['type']) {
                if ($rInfo['type'] != $rType) {
                    $this->insertErrorMsg('typeInconsistency', $recordName, $rName);
                }
            }
        } else {
            if(!$rInfo['type']) {
                $rInfo['type'] = self::$fieldDefaultType;
                if(empty($fieldComment)) {
                    $this->insertErrorMsg('lackOfDataValueTypes', $recordName, $rName);
                }
            }
        }

        $rInfo['comment'] = $fieldComment;
        return $rInfo;
    }

    /**
     * 填入错误信息
     * @param 错误类型
     * @param $recordName record 中名称 
     * @param $rName record 字段名称
     */
    private function insertErrorMsg($key, $recordName, $rName)
    {
        if (!isset(self::$errorMsg[$key][$recordName])) {
            self::$errorMsg[$key][$recordName][] = $rName;
        } else {
            array_push(self::$errorMsg[$key][$recordName], $rName);
            array_unique(self::$errorMsg[$key][$recordName]);
        }
    }

    /**
     * 处理 define 和 field 数据
     *  1. 处理 define 自身值转换
     *  2. 处理 define 和 field 值转换
     * @return void
     */
    private function handleDefineFieldData()
    {
        if (empty(self::$defineArr) || empty(self::$fieldArr)) {
            return false;
        }

        $this->recursionProcessingDefine(self::$defineArr);
        $this->changeFieldValueForDefine();
    }

    /**
     * 处理 define 和 field 值转换
     *  1. 遍历field数组，然后检测define 数组是否存在相同值，如存在，就替换值
     *  2. 如果define数组key检索不成功，就需要字符串检索define key
     * @return void
     */
    private function changeFieldValueForDefine()
    {
        foreach (self::$fieldArr as $fieldName => $fieldValue) {
            if (isset(self::$defineArr[$fieldValue])) {
                $fieldValue = self::$defineArr[$fieldValue];
            } else {
                foreach (self::$defineArr as $definename => $defineValue) {
                    $pos = strpos($fieldValue, $definename);
                    if ($pos !== false) {
                        $fieldValue  = str_replace ($definename, $defineValue, $fieldValue);
                        break;
                    }
                    continue;
                }
            }
            self::$fieldArr[$fieldName] = $fieldValue;
        }
    }

    /**
     * 处理 define 自身值转换
     *  1. 用于define 自身变量的替换
     * @param $defineArr define 数组
     * @return void
     */
    private function recursionProcessingDefine($defineArr)
    {
        foreach ($defineArr as $defineName => $defineValue) {
            foreach ($defineArr as $defineNameF => $defineValueF) {
                if ($defineName == $defineNameF) {
                    continue;
                }

                if (strpos($defineValueF, $defineName) !== false) {
                    self::$defineArr[$defineNameF] = str_replace($defineName, $defineValue, $defineValueF);
                }
            }
        }
    }

    /**
     * 统一提取数据
     *  1. record 校验 () 代表一个完整的 record
     *  2. record 处理
     * @param $recordArr recode 数组
     */
    private function handleRecordArr($recordArr)
    {
        foreach ($recordArr as $record) {
            if ($this->fileringAnnotation($record)) {
                continue;
            }

            foreach (self::$defaultKeyWord as $keyword => $searchWord) {
                $this->gainDefaultInfo($record, $searchWord, $keyword);
            }

            $this->getErlangNotes($record);
            $this->gainRecordInfo($record);
        }
    }

    /**
     * record 校验 () 代表一个完整的 record
     * @param $recode recode语句
     * @param $search 待校验的关键词对应的语句
     */
    private function checkTheIntegrityOfRecord($record, $search)
    {
        $startTag = '(';
        $endTag = ')';
        $startPos = strpos($record, $startTag);
        $endPost = strrpos($record, $endTag);
        if (empty(self::$recordTempStorageArea)) {
            if ($startPos !== false && $endPost !== false) {
                return true;
            } else {
                self::$recordTempStorageArea[] = $record;
                return false;
            }
        } else {
            self::$recordTempStorageArea[] = $record;
            if ($endPost !== false) {
                $record = '';
                $commentTag = '%';
                foreach (self::$recordTempStorageArea as $str) {
                    $pos = strpos($str, $commentTag);
                    if ( $pos !== false) {
                        $str = trim(substr($str, 0, $pos));
                    }
                    $record .= $str;
                }
                $this->handleRecordInfo($record, $search);
                self::$recordTempStorageArea = [];
            }
            return false;
        }
    }

    /**
     * 获取record信息
     * @param $record record 语句
     * @param $search 获取信息的关键词
     * @return void
     */
    private function gainRecordInfo($record, $search= 'record')
    {
        $isFind = strpos(strtolower($record), $search);
        if(empty(self::$recordTempStorageArea) &&  $isFind === false){
            return true;
        }
        if ($this->checkTheIntegrityOfRecord($record, $search)) {
            $this->handleRecordInfo($record, $search);
        }
    }

    /**
     * record 处理
     *  1. 对record 内的字段进行处理
     *  2. 处理record类型和备注类型
     * @param $record record 语句
     * @param $search 语句关键词
     * @return void
     */
    private function handleRecordInfo($record, $search)
    {
        $isFind = strpos(strtolower($record), $search);
        $recordStr = substr($record, strlen($search) + $isFind + 1);
        // 正则表达式
        $pattern = '/^(\w+)[,，]*\s*{*(.+)}*/i';
        if (preg_match($pattern, $recordStr, $match)) {
            // $origin: 原始record
            // $recordName: record 名称
            // $recordInfo: record 内字段
            @list($origin, $recordName, $recordInfo ) = $match;
            // 字段数组
            $recordInfo = explode(',', $this->replaceStr($recordInfo,'[\s，}).\r\n]+'));
            $recordFieldArr = [];
            foreach ($recordInfo as $filterRecord) {
                @list($field, $type) = explode('=', $filterRecord);
                // 判断record内字段是否重复
                if (isset($recordFieldArr[$field])) {
                    $this->insertErrorMsg('fieldRepetitionDefinition', $recordName, $field);
                }
                $recordFieldArr[$field] = $type;
            }
            // 处理record类型和备注类型
            $this->handleRecordDataType($recordName, $recordFieldArr);
        }
    }

    /**
     * 处理record类型和备注类型
     *  1. record 内字段与注释信息匹配
     *  2. 检查字段类型
     *  3. 生成 record 字段数组
     * @param $recordName record 名称
     * @param $recordFieldArr record 内字段
     * @return void
    */
    private function handleRecordDataType($recordName, $recordFieldArr)
    {
        $handleData = [];
        $typeInconsistency = [];
    
        foreach ($recordFieldArr as $field => $type) {
            $accuracy = false;
            $remark = '';
            // record 内字段与注释信息匹配
            if (isset(self::$erlangNotesArr[$field])) {
                // 得到注释信息
                $notesArr = self::$erlangNotesArr[$field];
                // 检查字段类型
                if ($notesArr['fieldType']) {
                    // $typeOther: 字段类型
                    // $typeAccuracy: 类型精度
                    @list($typeOther, $typeAccuracy) = preg_split('/[:]/', $notesArr['fieldType']);
                    $accuracy = $typeAccuracy ? $typeAccuracy : false ;
                    $typeRemark = $this->dataTeypDetection($type, $accuracy);
                    $fileTypeRemark = $this->dataTeypDetection($typeOther);
                    
                    // 保险措施。为了判断类型不一致
                    if ($typeRemark['type'] && $fileTypeRemark['type']){
                        if ($typeRemark['type'] != $fileTypeRemark['type']){
                            $typeInconsistency[] = $field;
                        }
                    }

                    $notesArr['fieldType'] = $typeOther;
                }

                $type =  $notesArr['fieldType'] ? $notesArr['fieldType'] : $type;
                // 获取record 字段注释信息
                $remark = $notesArr['fieldRemark'];
                // 注释匹配完成，并删除
                unset(self::$erlangNotesArr[$field]);
            }

            $fiterRes = $this->dataTeypDetection($type, $accuracy);
            $type = $fiterRes['type'];
            $accuracy = $fiterRes['accuracy'];
            $handleData[$field] = [
                'type' => $type,         // 类型
                'accuracy' => $accuracy, // 类型精度
                'comment' => $remark     // 字段注释
            ];
        }
       
        // 合并默认自带字段
        $handleData = array_merge(self::$defaultTableField, $handleData);
        
        // 把剩下没有匹配上字段的注释，一股脑放到表注释中
        $tableCommnet = implode(" ", array_map(function ($str) {
            $str = str_replace('。', '.', $str);
            $pos = strpos($str, '.') ;
            if($pos !== false) {
                $str = substr($str,$pos + 1);
            }
            return $str;
        }, array_keys(self::$erlangNotesArr)));

        // 错误检查
        if (!empty($typeInconsistency)) {
            self::$errorMsg['typeInconsistency'][$recordName] = array_unique($typeInconsistency);
        }

        // 清空注释，同时生成 recode 字段数组
        self::$erlangNotesArr = [];
        self::$recordTableArr[$recordName] = [
            'record' => $handleData,
            'record_comment' => $tableCommnet
        ];
    }

    /**
     * 类型检测
     *  1. 类型校验函数分为两个部分, self前缀是自定义函数。
     *  2. is前缀为php字段的类型校验函数
     *  3. 主要是为了增加类型校验的范畴，避免php内置函数的不足
     *  4. 自定义类型校验优先
     * @param $type 字段类型
     * @param $accuracy 类型精度 
     * @return 返回类型装饰数组
     */
    private function dataTeypDetection($type, $accuracy = false)
    {
        if ($type === false || is_null($type)) {
            return [ 'type' => false, 'accuracy' => $accuracy ];
        }

        // 遍历类型校验方法
        foreach (self::$replaceTypeFuc as $checkFunc => $rebackType) {
            // 自定义类型校验函数入口
            if (!function_exists($checkFunc)) {
                @list($self, $judgeStr) = explode('_', $checkFunc);
                $res = $this->selfCommonFuc($type, $judgeStr, $rebackType, $accuracy);
                if ($res) {
                    return $res;
                }

                continue;
            }

            // php 内置校验函数入口
            if ($checkFunc((is_numeric($type) ? ($type + 0) : $type))) {
                return ['type' => $rebackType, 'accuracy' => $accuracy];
            }
        }
    }

    /**
     * 自定义类型校验函数入口
     * @param $field 字段类型
     * @param $judgeStr 待校验的字段
     * @param $rebackType 待替换的类型
     * @param $accuracy 类型精度 
     * @return 返回类型装饰数组
     */
    private function selfCommonFuc($field, $judgeStr, $rebackType, $accuracy)
    {
        // $rebackTypeStr: 待替换的类型
        // $rebackTypeAcc: 待替换的类型精度
        @list($rebackTypeStr, $rebackTypeAcc) = explode(':', $rebackType);
        @list($fieldType, $fieldAcc) = explode(':', $field);
        // 判断是否需要精度。一般是用于字符串
        $accuracy = $accuracy ?  $accuracy : ($fieldAcc ? $fieldAcc : $rebackTypeAcc);
        return (strtolower($fieldType) == strtolower($judgeStr) ? ['type' => $rebackTypeStr, 'accuracy' => $accuracy] : false );
    }

    /**
     * 获取erlang注释信息
     * @param $record record 语句
     * @param $notes 过滤语句
     * @param $num 过滤语句限制数量
     * @return void
     */

    private function getErlangNotes($record, $notes= '%', $num= 2)
    {
        $isNotNotes = (substr_count(substr($record, 0, 4), $notes) >= $num ? false : true);
        if ($isNotNotes) {
            return true;
        }

        $recordStr = $this->replaceStr($record,'['.$notes.'\'\"]+');
        $pattern = ';';
        @list($fieldName, $fieldRemark, $fieldType)= preg_split('/'.$pattern.'/i', trim($recordStr));
        self::$erlangNotesArr[$fieldName] = [
            'fieldName' => trim($fieldName),
            'fieldRemark' => trim($fieldRemark),
            'fieldType' => strtolower(trim($fieldType))
        ];
    }

    /**
     * 获取默认定义信息
     * @param $record record语句
     * @param $search 搜索语句
     * @param $keyword 搜索语句对应的关键词，用于开启临时存储数据，存储对应的record 的字段数据
     * @return void
     */
    private function gainDefaultInfo($record, $search, $keyword)
    {
        $isFind = strpos(strtolower($record), $search);
        if ($isFind === false) {
            return true;
        }

        $recordStr = substr($record,strlen($search) + $isFind + 1, -1);
        $pattern = '/\s*[\'\"‘“]*(\w+)[\'\"’”]*\s*[,，]\s*[\'\"]*(.+)*\s*/i';
        if (preg_match($pattern, $recordStr, $match)) {
            $endTag = "'";
            list($origin, $filed, $value) = $match;
            $field = $this->replaceStr($filed);

            if (strrpos($value, $endTag) !== false) {
                $value = substr($value, 0, strrpos($value, $endTag));
            }
            $value = trim($this->replaceStr($value, '[\'\"]'));

            // 把数据存储到类静态数组
            $tmp = $keyword.'Arr';
            self::$$tmp = array_merge(self::$$tmp, [$field => $value]);
        }
    }

    /**
     * 替换特殊字符串
     * @param $subject 需要替换的字符串
     * @param $pattern 需要替换的规则
     * @param $replacement 语句的关键词
     * @return 返回替换后的语句
     */
    private function replaceStr($subject, $pattern='[,\'\"]+', $replacement = '')
    {
        return preg_replace('/'.$pattern.'/i', $replacement, $subject);
    }

    /**
     * 过滤注释 #
     * @param $record record 语句
     * @param $notes 过滤语句
     * @param $num 过滤语句限制数量
     * @return bool
     */
    private function fileringAnnotation($record, $notes= '#', $num = 1)
    {
        return (substr_count(substr($record,0, 4), $notes) >= $num ? true :false);
    }
}

// 日志类
class Log {
    private static $outMsgColor = [
        'SUCCESS' => '[42m',
        'FAILURE' => '[41m',
        'WARNING' => '[43m',
        'NOTE' => '[44m'
    ];

    private static $defaultErrorType = [
        'lackOfDataValueTypes' => [
            'msg' => '默认类型缺失'
        ],
        'typeInconsistency' => [
            'msg' => '默认类型与定义类型不一致'
        ],
        'fieldRepetitionDefinition' => [
            'msg' => 'record 字段重复定义'
        ]
    ];

    public function handleErrorMsg($errorMsg)
    {
        foreach ($errorMsg as $errorType => $errorData) {
            if (isset(self::$defaultErrorType[$errorType])) {
                $errorArr = self::$defaultErrorType[$errorType];
                print_r(PHP_EOL . $errorArr['msg'] . PHP_EOL);
                print_r($errorData);
            }
        }
    }

    private function createInfoFile($explain, $list, $fileName)
    {
        $msg = '====== '.$explain."-".date('Y-m-d H:i:s', time())." ========\r\n";
        $msg .= var_export($list, true);
        file_put_contents($fileName, $msg);
    }

    public function echoWarningInfo($msg= '', $esc= true)
    {
        if (empty($msg)) {
            return false;
        }
        $this->commentCreateMsg('WARNING', $msg, $esc);
    }

    public function echoErrorInfo($msg = '', $esc = true)
    {
        if (empty($msg)) {
            return false;
        }
        $this->commentCreateMsg('FAILURE', $msg, $esc);
    }

    public function echoParamteInfo() 
    {
        $msg = 'php '. basename(__FILE__) . ' -f record文件名 -p 表前缀[可选]';
        $this->echoNoteInfo($msg);
    }

    public function echoSuccessInfo($msg = '', $esc= true)
    {
        if (empty($msg)) {
            return false;
        }

        $this->commentCreateMsg('SUCCESS', $msg, $esc);
    }

    public function echoNoteInfo($msg = '', $esc = true) 
    {
        if (empty($msg)) {
            return false;
        }
        
        $this->commentCreateMsg('NOTE', $msg, $esc);
    }

    private function commentCreateMsg($status, $msg,$esc = true)
    {
        $out = self::$outMsgColor[$status];
        echo chr(27) . "$out" . "$msg" . chr(27) . "[0m"."\r\n";
        if ($esc) {
            exit();
        }
    }
}

$options = getopt('f:s:h:d:p::');
$logObj = new Log();
if (empty($options)  || !isset($options['f'])) {
    $logObj->echoErrorInfo('缺少路径参数', false);
    $logObj->echoParamteInfo();
    exit();
}

if (empty($options['s'])) {
    $logObj->echoErrorInfo('缺少sql输出文件', false);
    $logObj->echoParamteInfo();
    exit();
}

if (empty($options['h'])) {
    $logObj->echoErrorInfo('缺少hql输出文件', false);
    $logObj->echoParamteInfo();
    exit();
}

if (empty($options['d'])) {
    $logObj->echoErrorInfo('缺少数据库名', false);
    $logObj->echoParamteInfo();
    exit();
}

// 程序入口
$tablePrefix = (isset($options['p']) && !empty($options['p'])) ? $options['p'] : 't_';
$obj = new Record2Sql($logObj);
$obj->start($options['f'], $options['s'], $options['h'], $options['d'], $tablePrefix);
