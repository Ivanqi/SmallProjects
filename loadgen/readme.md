# 解决的问题

一个软件可以跑多快？

在高负载的情况下，该软件是否还能保证正确性。或者说，载荷数量与软件正确性之间的关系是怎样的？载荷数量是一个笼统的词，可以是HTTP请求的数量，也可以是API调用的次数

在保证一定正确性的条件下，该软件的可伸缩性是怎样的？

在软件可以正常工作的情况下，负载与系统资源(包括CPU，内存和各种I/O资源等)使用率之间的关系是怎样的？


# QPS (Query PerSecond, 每秒查询量)
对服务器上数据的读取操作

# TPS (Transactions Per Second, 每秒事务处理量)
针对的是对服务器上数据的写入或修改操作

# 载荷与请求
这里把载荷和请求归为同一事物，它们都代表了软件使用者为了获得某种结果而向为之服务的软件发送一段数据

把每秒发送的载荷数量作为参数，其意义是控制载荷发生器向软件发送载荷的频率，这样就可以控制被测软件在一段时间之内的负载情况

# 处理超时时间
从软件发出请求到接收到软件返回的响应的最大耗时

超过这个最大耗时就会被认为是不可接受的，当次处理就被认为为无效的处理。设置处理超时时间，可以让我们更加精确地计算出在给定每秒载荷量荷负载持续时间的情况下软件的正确比率

# 输出的结果
某一个载荷的结果至少包含3块内容
- 请求(载荷)和响应的内容
- 响应的状态
- 请求处理耗时

请求(载荷)和响应的内容可以让我们精细地检查响应内容的正确性

响应的状态则反映出处理此请求过程中的绝大多数问题，而不仅仅是成功或失败那么简单

请求耗时，则需要真实地体现从向软件发送请求，到接收到软件响应的精准耗时，并且不夹杂任何其他操作的进行时间

# 并发量
一个调用过程总体上包含两个操作
- 一个是向被测软件发送一个载荷(或者说对被测软件的API进行一次调用)的操作
- 另一个是等待并从被测软件哪里接收一个响应(或者说等待并获取被测软件API的返回结果)的操作

并发公式:
- 并发量 = 单个载荷的响应超时时间 / 载荷的发送间隔时间

# 测试语句
开始: go test -v -run=TestStart

结束: go test -v -run=TestStop