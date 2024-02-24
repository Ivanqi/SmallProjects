# 脚本使用方式
```
php record2sql2.php -f ${recordFile} -s ${sqlFile} -h ${hqlFile} -d ${database}
```
- recordFile: 输入recode 文件
- sqlFile: 输出的sql文件
- hqlFile: 输出的hql 文件
- database: 数据库名称

```
php record2sql2.php -f ./log_ban.txt -s ./log_ban.sql -h ./log_ban.hql -d db
```

# record规则格式
## record 自定义格式如下:

```
%% role_id;用户ID;bigint
%% role_name;用户名;
%% activation_key;激活码;
%% activity_id;活动ID;int
%% platform;平台ID;int
%% award_list;奖励道具;
%% total_pay_gold;消费元宝;int
%% mtime;激活时间;
%% 激活码激活日志
-record(log_activation_code, {role_id=0, role_name="", activation_key="", activity_id=0, platform=0, award_list="", total_pay_gold=0, mtime=0}).
```

## record 自定义规则说明
1. 字段定义说明
   <br />【role_id;用户ID;bigint】 代表是 字段名称;字段说明;字段类型
   字段类型一般情况可以不写，默认为 varchar(255)
   <br />如果数字类型过长(会溢出),请在后面的字段类型中填写bigint
2. 字段默认值
    <br />record中的字段默认值，比如 role_name="" 或者 gains = []，将会自动把role_name和gains设置为TEXT类型；因此如果存在默认值可以不写字段类型。
    <br />但如果是数字类型，并且长度过长,就必须指定类型为bigint；比如role_id，一般需要使用bigint
## 特别注意
1. 请后端一定把所有字段都写上字段说明 和 表的中文名。除非是以下常用字段
    - agnet_id
    - server_id
    - pf
    - role_id
    - upf
    - account_name
    - role_name
    - is_internal (是否内部号)
    - mtime

## 定义"常量"
> 所谓常量，其实就是字符串替换。

```
# 这里是注释
# 定义了ITEM_JSON常量，之后会替换为'单引号里面的字符串内容。
# 因此如果有相同的字段信息，请使用常量；而不是使用"同上"这样的描述
-define( ITEM_JSON, 'JSON，结构[{"type": xx, "type_id": xx, "num":xx}]，type为1.元宝；2；铜钱；3；经验；4.二级货币；5.道具；type_id为道具或积分ID，其他情况为0；num为数量;text')

# 字段说明处可以使用
%% consumes; 消耗，ITEM_JSON
%% gains; 获得，ITEM_JSON
```

## 默认的字段解释
> 可以自定义默认的字段解释。
比如多个表都存在字段'role_level'字段，并且字段类型和描述也一样；这样"默认的字段解释"就可以避免重复定义
```
# 使用默认字段解释，之后表结构如果存在以下字段，并且没有给出字段说明，将使用以下的默认说明
# 这样一些常用的字段的解释就可以不用重复写，直接定义默认的字段解释即可
-field(agent_id, '代理ID;int:10')
-field(role_id, 'ROLE_ID')
-field(server_id, '区服ID;int')
-field(account_name, '帐号;varchar:100')
-field(role_name, '角色名;varchar:100')
-field(upf, 'United pf')
-field(pf, '渠道; int')
-field(is_internal, '是否内部号')
-field(mtime, '时间')
```

## recode例子
```
#############################################
## 下面的所有字段如无说明，
## 该字段类型即为 int类型
## 字段类型：int bigint char varchar text/string float double 其中char/varchar需要指定长度
## int:10 char:10 float:7,3
## 注意：不支持换行
#############################################
# 常量定义
-define( ROLE_ID, '角色ID;BIGINT:20')
-define( ITEM_JSON, 'JSON，结构[{"type": xx, "type_id": xx, "num":xx}]，type为1.元宝；2；铜钱；3；经验；4.二级货币；5.道具；type_id为道具或积分ID，其他情况为0；num为数量;text')
-define( GAINS, '获得奖励，ITEM_JSON')
-define( CONSUMES, '消耗，ITEM_JSON' )
# 默认的字段解释 之后的表结构如果存在该字段就会使用定义的字段解释，不用重新写
-field(agent_id, '代理ID;int:10')
-field(role_id, 'ROLE_ID')
-field(server_id, '区服ID;int')
-field(account_name, '帐号;varchar:100')
-field(role_name, '角色名;varchar:100')
-field(upf, 'United pf')
-field(pf, '渠道;int')
-field(platform, '渠道;int')
-field(is_internal, '是否内部号')
-field(mtime, '时间')
-field(level, '角色等级')
-field(regrow, '角色转生')
-field(client_oid, 'client_oid;varchar:50')
-field(client_version, '客户端版本;varchar:50')
-field(server_version, '后端版本;varchar:50')


%% 封禁
%% ban_agent_id;代理ID;int:10
%% ban_pf_id;平台ID;int
%% ban_type;封禁类型;int
%% ban_key;封禁内容;varchar:255
%% drop_rate;不绑定产出概率降低;int
%% end_time;结束时间;int
%% reason;封禁内容;varchar:255
%% admin;封禁内容;varchar:50
%% admin;封禁内容;varchar:50

-record(log_ban, {ban_agent_id, ban_pf_id, ban_type, ban_key, drop_rate, end_time, mtime=0, reason, admin}).
```

