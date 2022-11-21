package main

import (
	"flag"
	"fmt"
)

// flag 包标志实现命令行标志解析
var keysPtr = flag.Int("keys", 10000000, "key number")
var nodesPtr = flag.Int("nodes", 3, "node number of old cluster")
var newNodesPtr = flag.Int("new-nodes", 4, "node number of new cluster")

func hash(key int, nodes int) int {
	return key % nodes
}

// 命令执行: go run ./hash.go -keys 10000000 -nodes 3 -new-nodes 4
func main() {

	flag.Parse()

	var keys = *keysPtr
	var nodes = *nodesPtr
	var newNodes = *newNodesPtr

	migrate := 0

	for i := 0; i < keys; i++ {
		if hash(i, nodes) != hash(i, newNodes) {
			migrate++
		}
	}

	migrateRatio := float64(migrate) / float64(keys)
	// 数据迁移比率
	fmt.Printf("%f%%\n", migrateRatio*100)
}
