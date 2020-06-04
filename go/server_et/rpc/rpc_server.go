package rpcUtil

import (
	"log"
	"net"
	"net/rpc"
	"reflect"
	"runtime/debug"
	rpcpb "server/proto/rpc"

	"github.com/golang/protobuf/proto"
)

//ServiceData rcp data
type ServiceData struct {
	server *Server
}

//CallBack rpc call back
type CallBack func(arg proto.Message, reply *proto.Message) error

//Args rpc call args
type Args struct {
	CallBackType int32
	Data         []byte
}

func onTimer(arg proto.Message, reply *proto.Message) error {
	return nil
}

//RPCService rcp service
func (sd *ServiceData) RPCService(arg Args, reply *Args) error {
	defer func() {
		x := recover()
		if x != nil {
			log.Printf("%v\n%s", x, string(debug.Stack()))
		}
	}()

	if arg.CallBackType == int32(rpcpb.PbType_RpcTimer) {
		return nil
	}

	fun, ok := sd.server.rpcCallBacks[arg.CallBackType]
	if !ok {
		log.Printf("RPCService can not find callback:%d", arg.CallBackType)
		return nil
	}
	var pbmsg proto.Message
	name, ok := rpcpb.PbType_TypeEnum_name[arg.CallBackType]
	if ok {
		name = packname + name
		var msg = proto.MessageType(name)
		if msg == nil {
			log.Printf("RPCService can not find msg name :%s", name)
			return nil
		}
		pbmsg = reflect.Indirect(reflect.New(msg.Elem())).Addr().Interface().(proto.Message)
	}
	var pb proto.Message
	proto.Unmarshal(arg.Data, pbmsg)
	var e = fun(pbmsg, &pb)
	if pb != nil {
		name = proto.MessageName(pb)
		if len(name) > packLen {
			name = name[packLen:]
		}
		reply.CallBackType = rpcpb.PbType_TypeEnum_value[name]
		var err error
		reply.Data, err = proto.Marshal(pb)
		if err != nil {
			log.Printf("RPCService proto.Marshal :%v error", pb)
			return nil
		}
	}
	return e
}

//RegisterCallBack register call back
func (server *Server) RegisterCallBack(callBackType rpcpb.PbType_TypeEnum, fun CallBack) {
	if callBackType == rpcpb.PbType_RpcTimer {
		log.Panicln("can not register RpcTimer callback")
		return
	}
	server.rpcCallBacks[int32(callBackType)] = fun
}

type Server struct {
	rpcServer    *rpc.Server
	address      string
	rpcCallBacks map[int32]CallBack
}

//StartRPCServer start rpc server in address
func StartRPCServer(address string) *Server {
	rpcServer := rpc.NewServer()
	serviceData := new(ServiceData)
	rpcServer.Register(serviceData)
	l, e := net.Listen("tcp", address) // any available address
	if e != nil {
		log.Fatalf("net.Listen tcp :0: %v", e)
		return nil
	}
	go rpcServer.Accept(l)
	server := new(Server)
	server.rpcServer = rpcServer
	server.address = address
	server.rpcCallBacks = map[int32]CallBack{
		0: onTimer,
	}
	serviceData.server = server
	return server
}
