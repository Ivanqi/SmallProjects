# 内核链表
在Linux内核中，提供了一个用来创建双向循环链表的结构 list_head

虽然linux内核是用C语言写的，但是list_head的引入，使得内核数据结构也可以拥有面向对象的特性，通过使用操作list_head 的通用接口很容易实现代码的重用
# 参考资料
- [图文讲解｜玩转内核链表 list](https://zhuanlan.zhihu.com/p/450778696)