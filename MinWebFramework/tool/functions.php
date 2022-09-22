<?php
/**
 * 环境函数实现
 */
function env($name = null, $default = null) {
    static $_env = [];
    if(empty($name)){
        return false;
    }
    if(is_string($name)){
        if(isset($_env[$name])){
            return  $_env[$name];
        }else if(is_null($default)){
            return '';
        }
        return $default;
    }
    
    if(is_array($name)){
        $_env = array_merge($_env, $name);
        return ;
    }
}

/**
 * 环境变量参数获取
 */

function handlerEnvFile($envPath)
{
    $finalEnvData = [];
    if (file_exists($envPath)) {
        $envData = file($envPath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach($envData as $env){
            $env = trim($env);
            // 跳过注释
            if ( empty($env{0}) || $env{0} == '#' ) {
                continue;
            }
            @list($key,$value) = explode('=',$env, 2);

            if ($value == 'true') {
                $value = true;
            } else if ($value == 'false'){
                $value = false;
            }
            $finalEnvData[$key] = $value;
        }
    }
    env($finalEnvData);
}

/**
 * 载入配置
 */
function loadAllConfig($configArr)
{
    foreach($configArr as $config){
        config(include $config);
    }
}

/**
 * 获取文件中所有文件和文件夹
 */
function getDirInfo($path, &$files)
{
    if (is_dir($path)) {
        $dp = dir($path);
        while ($file = $dp ->read()){
            if($file !="." && $file !=".."){
                getDirInfo($path."/".$file, $files);
            }
        }
        $dp ->close();
    } else if (is_file($path)) {
        $pathArr = explode('/', $path);
        $name = end($pathArr);
        $files[$name] =  $path;
    }
}


/**
 * 加载和获取配置
 */
function config($name = null, $value = null, $default = null)
{
    static $_config = [];
    if(empty($name)){
        return $_config;
    }
    if(is_string($name)){
        if(!strpos($name, '.')) {
            if(is_null($value)) {
                return isset($_config[$name]) ? $_config[$name] : $default;
            }
        }
        $name = explode('.', $name);
        if(is_null($value)) {
            return isset($_config[$name[0]]) ? $_config[$name[0]][$name[1]] : $default;
        }
        return $_config[$name[0]][$name[1]] = $value;
    } else if(is_array($name)) {
        return $_config = array_merge($_config, $name);

    } else if ($name === true) {
        $_config = [];
    } else {
        return $default;
    }
}

function _addslashes($arr)
{
    foreach($arr as $k =>$v){
        if(is_string($v)){
            $arr[$k] = addslashes($v);
        }else if(is_array($v)){
            $arr[$k] = _addslashes($v);
        }
    }
    return $arr;
}

function curlGet($url, $params = array(), $timeOut=60) {
    if (! trim ( $url )) {
        return false;
    }
    $ch = curl_init ();
    curl_setopt ( $ch, CURLOPT_URL, $url );
    curl_setopt ( $ch, CURLOPT_HTTPHEADER, $params );
    curl_setopt ( $ch, CURLOPT_RETURNTRANSFER, 1 );
    curl_setopt ( $ch, CURLOPT_HEADER, 0 );
    curl_setopt ( $ch, CURLOPT_TIMEOUT, $timeOut );
    curl_setopt ( $ch, CURLOPT_SSL_VERIFYPEER, false );
    $result = curl_exec ( $ch );
    curl_close ( $ch );
    return $result;
}


function curlPost($url, $params, $header = array(), $timeOut=60) {
    if (! trim ( $params )) {
        return false;
    }
    $ch = curl_init ();
    curl_setopt ( $ch, CURLOPT_URL, $url );
    curl_setopt ( $ch, CURLOPT_RETURNTRANSFER, 1 );
    curl_setopt ( $ch, CURLOPT_POST, 1 );
    curl_setopt ( $ch, CURLOPT_POSTFIELDS, $params );
    curl_setopt ( $ch, CURLOPT_HTTPHEADER, $header );
    curl_setopt ( $ch, CURLOPT_TIMEOUT, $timeOut );
    curl_setopt ( $ch, CURLOPT_SSL_VERIFYPEER, false );
    $result = curl_exec ( $ch );
    curl_close ( $ch );
    return $result;
}



/**
 * 判断是否为post
 *
 * @return boolean
 */
function isPost() {
    return (strtolower ( $_SERVER ['REQUEST_METHOD'] ) == 'post');
}

/**
 * 判断是否为get
 *
 * @return boolean
 */
function isGet() {
    return (strtolower ( $_SERVER ['REQUEST_METHOD'] ) == 'get');
}

function getCookie($key) {
    if (isset ( $_COOKIE [$key] )) {
        return $_COOKIE [$key];
    }
    return false;
}

function getSession($key) {
    if ($key == 'agentName') {
        $res = $_SESSION ['agent'];
    } elseif ($key == "serverID") {
        $res = $_SESSION ['server'];
    } else {
        $res = $_SESSION [$key];
    }
    return $res;
}

function setSession($key, $value) {
    if (isset ( $_SESSION [$key] )) {
        unset ( $_SESSION [$key] );
    }
    $_SESSION [$key] = $value;
    return true;
}


function getLocalIp()
{
    $preg = "/\A((([0-9]?[0-9])|(1[0-9]{2})|(2[0-4][0-9])|(25[0-5]))\.){3}(([0-9]?[0-9])|(1[0-9]{2})|(2[0-4][0-9])|(25[0-5]))\Z/";
    exec("ifconfig", $out, $stats);
    if (!empty($out)) {
        if (isset($out[1]) && strstr($out[1], 'addr:')) {
            $tmpArray = explode(":", $out[1]);
            $tmpIp = explode(" ", $tmpArray[1]);
            if (preg_match($preg, trim($tmpIp[0]))) {
                return trim($tmpIp[0]);
            }
        }
    }
    return 'localhost';
}

/**
 * curl 发送请求 delete 删除 tfs上的图片文件
 * $url 请求地址
 * @rturn string 返回请求信息
 */
function CurlDeleteWithHttpHeader($url){
    if(empty($url)){
        return false;
    }
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL,$url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HEADER, 1); //取得返回头信息
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "DELETE");
    $output = curl_exec($ch);
    curl_close($ch);
    return $output;
}


function array_get($array, $key, $default = null)
{
    $key = explode('.', $key);
    $data = $array;
    foreach ($key as $k) {
        if (is_array($data) && array_key_exists($k, $data)) {
            $data = $data[$k];
        } else {
            return $default;
        }
    }
    return $data;
}

function execCmd($cmd)
{
    $result = shell_exec($cmd . " 2>&1");
    $res = commmonError($result);
    if ($res == 1) {
        return returnSqlArr($result);
    } else {
        return [];
    }
}

function returnMysqlArr($result)
{
    $array = preg_split("/[\r\n,]+/", $result);
    $fieldArr = [];
    $mysqlConfig = [];
    $key = 0;
    $errorTag = 'mysql';
    foreach($array as $k => $val) {
        if (empty($val)){
            continue;
        }
        if (strpos($val, $errorTag) !== false) {
            $key++;
            continue;
        }
        $data = preg_split("/[\t]+/", $val);
        if ($k == $key) {
            $fieldArr = $data;
            continue;
        }
        $combineArr = array_combine($fieldArr, $data);
        $dbName = array_get($combineArr, 'db_name', 'default');
        $mysqlConfig[$dbName] = $combineArr;
    }
    return $mysqlConfig;
}

function returnSqlArr($result)
{
    $data = [];
    if (is_string($result)) {
        $array = preg_split("/[\r\n,]+/", $result);
        $fieldArr = [];
        $key = 0;
        $errorTag = 'mysql';
        foreach($array as $k => $val) {
            if (empty($val)){
                continue;
            }
            if (strpos($val, $errorTag) !== false) {
                $key++;
                continue;
            }
            $valData = preg_split("/[\t]+/", $val);
            if ($k == $key) {
                $fieldArr = $valData;
                continue;
            }
            $combineArr = array_combine($fieldArr, $valData);
            $data[] = $combineArr;
        }
    }
    return $data;
}

function commmonError($result)
{
    $res = 0;
    $result = strtolower($result);
    if(strpos($result,'error') !== false) {
        $res = 0;
    } else {
        $res = 1;
    }
    return $res;
}


function handleError($errno, $errstr, $errfile, $errline){
    $data = [
        '文件:'. $errfile,
        '异常代码:'. $errno,
        '行号:'. $errline,
        '异常信息:'. $errstr
    ];
    print_r($data).PHP_EOL;
}

function handleException($exception){
    if ($exception){
        $data = [
            '文件:' . $exception->getFile(),
            '异常代码:'. $exception->getCode(),
            '行号:'. $exception->getLine(),
            '异常信息:'. $exception->getMessage(),
        ];
        print_r($data).PHP_EOL;
    }   
}

function customShutdownHandler()
{
    $error = error_get_last();
    if ($error) {
        $data = [
            '文件:' . $error['file'],
            '异常代码:'. $error['code'],
            '行号:'. $error['line'],
            '异常信息:'. $error['message'],
        ];
        print_r($data);
    }
}