# consistent hash 测试
- go test -v example_test.go/consistent_test.go -run xxx方法

# 计算hash和consistent hash的数据迁移率
- go run ./hash.go -keys 10000000 -nodes 10 -new-nodes 11
- go run ./consistent-hash.go -keys 10000000 -nodes 10 -new-nodes 11

# 测试节点数据论衡度
- go run consistent-hash-vnode.go