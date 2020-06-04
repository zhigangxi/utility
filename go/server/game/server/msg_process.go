package game

import (
	"encoding/binary"
	"log"
	"net/rpc"
	"runtime/debug"
	"server/network"
	gamepb "server/proto/game"
	rpcpb "server/proto/rpc"
	rpcUtil "server/rpc"

	"github.com/golang/protobuf/proto"
)

var (
	packName = "gamepb."
	packLen  = len(packName)
)

type ProcessMsgCB func(*network.Session, []byte)

func (server *Server) processMsg() {
	server.RegisterProcFun(0, server.onLogout)
	server.RegisterProcFun(uint16(gamepb.PbType_EnterGameReq), server.onEnterGame)
	server.RegisterProcFun(uint16(gamepb.PbType_CreateUserReq), server.onCreateUser)
	server.RegisterProcFun(uint16(gamepb.PbType_PackageReq), server.onPackage)
	server.RegisterProcFun(uint16(gamepb.PbType_EquipmentReq), server.onEquipment)
	server.RegisterProcFun(uint16(gamepb.PbType_AdminCmdReq), server.onAdminCmd)

	go func() {
		for {
			server.despatch()
		}
	}()
}

func (server *Server) despatch() {
	defer func() {
		x := recover()
		if x != nil {
			log.Printf("%v\n%s", x, string(debug.Stack()))
		}
	}()

	var rcpCall = server.rpc.GetCall()
	var recvMsg = server.net.GetRecvMsg()
	var dbResult = server.db.GetResult()
	select {
	case msg := <-*recvMsg:
		server.despatchMsg(&msg)
	case call := <-*rcpCall:
		server.despatchRpcMsg(call)
	case result := <-*dbResult:
		result.ResultCB()
	}
}

func (server *Server) despatchRpcMsg(call *rpc.Call) {
	var reply = call.Reply.(*rpcUtil.Args)
	switch rpcpb.PbType_TypeEnum(reply.CallBackType) {
	case rpcpb.PbType_GetLoginSessReply:
		server.onGetSession(reply.Data, call.Error)
	}
}

func (server *Server) despatchMsg(msg *network.MsgData) {
	if len(msg.Msg) >= network.MsgHeadLen || msg.Msg == nil {
		var cmd uint16
		if msg.Msg != nil {
			cmd = binary.LittleEndian.Uint16(msg.Msg[network.MsgLenLen:network.MsgHeadLen])
			if cmd == 0 {
				log.Printf("can not process cmd:%d\n", cmd)
				msg.Sess.Close()
				return
			}
		}
		fun, ok := server.procFuns[cmd]
		if !ok {
			log.Printf("can not process cmd:%d\n", cmd)
			msg.Sess.Close()
			return
		}
		if msg.Msg == nil {
			fun(msg.Sess, nil)
		} else {
			fun(msg.Sess, msg.Msg[network.MsgHeadLen:])
		}
	} else {
		log.Printf("recv error msg data\n")
		msg.Sess.Close()
	}
}

func (server *Server) RegisterProcFun(cmd uint16, cb ProcessMsgCB) {
	if _, ok := server.procFuns[cmd]; ok {
		log.Printf("cmd %d is already register\n", cmd)
	}
	server.procFuns[cmd] = cb
}

func unPbPanic(data []byte, msg proto.Message) {
	if proto.Unmarshal(data, msg) != nil {
		log.Panic("Unmarshal pb error")
	}
}

func marshalPbPanic(msg proto.Message) []byte {
	data, err := proto.Marshal(msg)
	if err != nil {
		log.Panicf("Marshal pb error:%s\n", err.Error())
	}
	return data
}

func sendPbMsg(sess *network.Session, pb proto.Message) {
	var name = proto.MessageName(pb)
	if len(name) <= packLen {
		log.Printf("pb name:%s error", name)
		return
	}
	name = name[packLen:]
	var cmd = gamepb.PbType_TypeEnum_value[name]
	var data = marshalPbPanic(pb)
	var dataLen = len(data)
	if dataLen > network.MaxMsgLen {
		log.Panicf("send msg:%s len is too long\n", name)
		return
	}
	var msg = make([]byte, dataLen+network.MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[:network.MsgLenLen], uint16(dataLen))
	binary.LittleEndian.PutUint16(msg[network.MsgLenLen:network.MsgHeadLen], uint16(cmd))
	copy(msg[network.MsgHeadLen:], data)
	sess.SendMsg(msg)
}
