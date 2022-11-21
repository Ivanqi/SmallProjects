# 代码设计与实现
代码的设计及实现主要是三个部分
- Coordinator 与 Worker 间的 RPC 通信
- Coordinator 调度逻辑
- Worker 计算逻辑

# RPC 通信
Coordinator 与 Worker 间的需要进行的通信主要有两块
- Worker 在空闲时向 Coordinator 发起 Task 请求，Coordinator 响应一个分配给该 Worker 的 Task
- Worker 在上一个 Task 运行完成后向 Coordinator 汇报

考虑到上述两个过程总是交替进行的，且 Worker 在上一个 Task 运行完成后总是立刻会需要申请一个新的 Task，在实现上这里我把它们合并为了一个 ApplyForTask RPC 调用
- 由 Worker 向 Coordinator 发起，申请一个新的 Task，同时汇报上一个运行完成的 Task（如有）
- Coordinator 接收到 RPC 请求后将同步阻塞，直到有可用的 Task 分配给该 Worker 或整个 MR 作业已运行完成

参数
- Worker ID
- 上一个完成的 Task 的类型及 Index。可能为空

响应
- 新 Task 的类型及 Index。若为空则代表 MR 作业已完成，Worker 可退出
- 运行新 Task 所需的其他信息，包括
  - 如果是 MAP Task，需要对应的输入文件名
  - 总 REDUCE Task 数量，用于生成中间结果文件
- 如果是 REDUCE Task，需要总 MAP Task 数量，用于生成对应中间结果文件的文件名

# Coordinator
由于涉及整个 MR 作业的运行过程调度以及 Worker Failover 的处理，Coordinator 组件的逻辑会相对复杂

首先，Coordinator 需要维护以下状态信息：
- 基础配置信息，包括 总 MAP Task 数量、总 Reduce Task 数量
- 调度所需信息，包括当前所处阶段，是 MAP 还是 REDUCE
- 所有仍未完成的 Task 及其所属的 Worker 和 Deadline（若有），使用 Golang Map 结构实现
- 所有仍未分配的 Task 池，用于响应 Worker 的申请及 Failover 时的重新分配，使用 Golang Channel 实现


然后，Coordinator 需要实现以下几个过程
- 在启动时，基于指定的输入文件生成 MAP Task 到可用 Task 池中
- 处理 Worker 的 Task 申请 RPC，从池中分配一个可用的 Task 给 Worker 并响应
- 处理 Worker 的 Task 完成通知，完成 Task 最终的结果数据 Commit
- 在 MAP Task 全部完成后，转移至 REDUCE 阶段，生成 REDUCE Task 到可用 Task 池
- 在 REDUCE Task 全部完成后，标记 MR 作业已完成，退出
- 周期巡检正在运行的 Task，发现 Task 运行时长超出 10s 后重新分配其到新的 Worker 上运行

# Worker
Worker 的核心逻辑比较简单，主要是一个死循环，不断地向 Coordinator 调用 ApplyForTask RPC
- Coordinator 返回空响应，代表 MR 作业已完成，则退出循环，结束 Worker 进程
- Coordinator 返回 MAP Task，则读取对应输入文件的内容
- 传递至 MR APP 指定的 Map 函数，得到对应的中间结果
- 按中间结果 Key 的 Hash 值进行分桶，保存至中间结果文件
- Coordinator 返回 REDUCE Task，则读取所有属于该 REDUCE Task 的中间结果文件数据
- 对所有中间结果进行排序，并按 Key 值进行归并传递归并后的数据至 MR APP 指定的 REDUCE 函数，得到最终结果
- 写出到结果文件