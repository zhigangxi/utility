package main

import (
	login "server/login/server"

	"github.com/golang/protobuf/proto"
)

func testRPC(arg proto.Message, reply *proto.Message) error {
	// var prcArg = arg.(*rpcpb.TestRpcArg)
	// var rep rpcpb.TestRpcReply
	// rep.Id = prcArg.Id
	// *reply = &rep
	return nil
}

type callBack func(arg interface{}, reply interface{}) error

type converToInterface struct {
}

func call(i interface{}, j interface{}) error {
	return nil
}

func testCallFun(fun *callBack) {
	var i converToInterface
	var in interface{}
	in = i
	(*fun)(1, &in)
}

func testRpcServer() {
	//rpcServer := rpc.StartRPCServer(":1234")
	//rpcServer.RegisterCallBack(rpcpb.PbType_TestRpcArg, testRPC)
}

func main() {
	login.GetLoginServer().Start()
}
