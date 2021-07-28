# 计算hash和consistent hash的数据迁移率。

# 依赖库
- go get stathat.com/c/consistent

# 使用
- go run ./hash.go -keys 10000000 -nodes 10 -new-nodes 11
- go run ./consistent-hash.go -keys 10000000 -nodes 10 -new-nodes 11