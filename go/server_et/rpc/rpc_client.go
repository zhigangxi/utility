package rpcUtil

import (
	"log"
	"net"
	"net/rpc"
	"reflect"
	rpcpb "server/proto/rpc"
	"time"

	"github.com/golang/protobuf/proto"
)

var (
	packname = "rpcpb."
	packLen  = len(packname)
)

type Client struct {
	rpcClient *rpc.Client
	addr      *net.TCPAddr
	tcpCon    *net.TCPConn
	call      chan *rpc.Call
}

//StartRPCClient start rpc client
func StartRPCClient(addr string) *Client {
	address, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		return nil
	}
	conn, netErr := net.DialTCP("tcp", nil, address)

	if netErr != nil {
		log.Printf("connect error:%v", netErr)
		return nil
	}
	var client = new(Client)
	client.addr = address
	client.tcpCon = conn
	client.rpcClient = rpc.NewClient(conn)
	client.call = make(chan *rpc.Call, 10)
	go client.timerCall()
	return client
}

func (client *Client) timerCall() {
	ticker := time.NewTicker(time.Second * 5)
	var timeReply proto.Message
	for {
		<-ticker.C
		if client.CallServer(nil, &timeReply) == rpc.ErrShutdown {
			for {
				conn, netErr := net.DialTCP("tcp", nil, client.addr)
				if netErr != nil {
					time.Sleep(time.Second)
					continue
				} else {
					client.tcpCon = conn
					client.rpcClient = rpc.NewClient(conn)
					break
				}
			}
		}
	}
}

//CallServer call rpc
func (client *Client) CallServer(pbArg proto.Message, pbReply *proto.Message) error {
	var arg Args
	var reply Args
	var name = proto.MessageName(pbArg)
	if len(name) > packLen {
		name = name[packLen:]
		arg.CallBackType = rpcpb.PbType_TypeEnum_value[name]
		arg.Data, _ = proto.Marshal(pbArg)
	}
	err := client.rpcClient.Call("ServiceData.RPCService", arg, &reply)
	if err == nil && reply.CallBackType != 0 {
		name, _ = rpcpb.PbType_TypeEnum_name[reply.CallBackType]
		name = packname + name
		var msg = proto.MessageType(name)
		if msg == nil {
			log.Printf("RPCService can not find msg name :%s", name)
			return nil
		}
		var pbmsg = reflect.Indirect(reflect.New(msg.Elem())).Addr().Interface().(proto.Message)
		proto.Unmarshal(reply.Data, pbmsg)
		*pbReply = pbmsg
	}
	if err == rpc.ErrShutdown {
	}
	return err
}

func (client *Client) GoCall(pbArg proto.Message) {
	var arg Args
	var reply Args
	var name = proto.MessageName(pbArg)
	if len(name) > packLen {
		name = name[packLen:]
		arg.CallBackType = rpcpb.PbType_TypeEnum_value[name]
		arg.Data, _ = proto.Marshal(pbArg)
	}
	client.rpcClient.Go("ServiceData.RPCService", arg, &reply, client.call)
}

func (client *Client) GetCall() *chan *rpc.Call {
	return &client.call
}
