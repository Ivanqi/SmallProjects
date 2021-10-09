package mr

import (
	"fmt"
	"log"
	"net/rpc"
	"hash/fnv"
)

type KeyValue struct {
	Key string
	Value string
}

func ihash(key string) int {
	h := fnv.New32a()
	h.Write([]byte(key))
	return int(h.Sum32() & 0x7fffffff)
}

func Worker(mapf func(string, string) []KeyValue, reducef func(string, []string) string) {

}

func CallExample() {
	args := ExampleArgs{}

	args.X = 99

	reply := ExampleReply{}

	call("Coordinator.Example", &args, &reply)

	fmt.Printf("reply.Y %v\n", reply.Y)
}

func call(rpcname string, args interface{}, reply interface{}) bool {
	sockname := coordinatorSock()

	c, err := rpc.DialHTTP("unix", sockname)

	if err != nil {
		log.Fatal("dialing:", err)
	}

	defer c.Close()

	err = c.Call(rpcname, args, reply)
	if err == nil {
		retrun true
	}

	fmt.Println(err)
	return false
}