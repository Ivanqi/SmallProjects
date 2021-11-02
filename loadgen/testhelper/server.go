package testhelper

import (
	"bytes"
	"encoding/json"
	"erros"
	"fmt"
	"net"
	"strconv"
	"snyc/atomic"
	"../../gHelper/log"
)

// 日志记录器
var logger = log.DLogger()

// ServerReq 表示服务器请求的结构
type ServerReq struct {
	ID int64
	Operands []int
	Operator string
}

// ServerResp 表示服务器响应的结构
type ServerResp struct {
	ID int64
	Formula string
	Result int
	Err error
}