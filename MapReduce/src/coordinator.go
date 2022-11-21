package mr

import (
	"log"
	"net"
	"os"
	"net/rpc"
	"net/http"
)

type Coordinator struct {

}

func (c *Coordinator) Example(args *ExampleArgs, reply *ExampleReply) error {
	reply.Y = args.X + 1
	return nil
}

func (c *Coordinator) server() {
	rpc.Register(c)
	rpc.HandleHTTP()

	sockname := coordinatorSock()

	// Remove 函数会删除 name 指定的文件或目录。如果出错，会返回 *PathError 底层类型的错误
	os.Remove(sockname)

	l, e := net.Listen("unix", sockname)
	if e != nil {
		log.Fatal("listen error:", e)
	}

	go http.Serve(l, nil)
}