package main

import (
	"flag"
	"fmt"
	"log"
	"strconv" // 包 strconv 实现与基本数据类型的字符串表示之间的转换

	"stathat.com/c/consistent" // 一致性hash包
)

// flag 包标志实现命令行标志解析
var keysPtr = flag.Int("keys", 10000000, "key number")
var nodesPtr = flag.Int("nodes", 3, "node number of old cluster")
var newNodesPtr = flag.Int("new-nodes", 4, "node number of new cluster")

func hash(key int, nodes int) int {
	return key % nodes
}

func main() {

	flag.Parse()
	var keys = *keysPtr
	var nodes = *nodesPtr
	var newNodes = *newNodesPtr

	c := consistent.New()
	for i := 0; i < nodes; i++ {
		c.Add(strconv.Itoa(i))
	}

	newC := consistent.New()
	for i := 0; i < newNodes; i++ {
		newC.Add(strconv.Itoa(i))
	}

	migrate := 0

	for i := 0; i < keys; i++ {
		server, err := c.Get(strconv.Itoa(i))
		if err != nil {
			log.Fatal(err)
		}

		newServer, err = newC.Get(strconv.Itoa(i))
		if err != nil {
			log.Fatal(err)
		}

		if server != newServer {
			migrate++
		}
	}

	migrateRatio := float64(migrate) / float64(keys)
	fmt.Printf("%f%%\n", migrateRatio*100)
}
