package login

import (
	"encoding/binary"
	"log"
	"runtime/debug"
	"server/network"
	loginpb "server/proto/login"

	"github.com/golang/protobuf/proto"
	"github.com/google/uuid"
)

var (
	packName = "loginpb."
	packLen  = len(packName)
)

const (
	processMsgGoNum = 16
)

type ProcessMsgCB func(*network.Session, []byte)

func (server *Server) processMsg() {
	server.RegisterProcFun(0, server.onLogout)
	server.RegisterProcFun(uint16(loginpb.LoginPbType_LoginReq), server.onLogin)
	server.RegisterProcFun(uint16(loginpb.LoginPbType_RegisterReq), server.onRegister)
	server.RegisterProcFun(uint16(loginpb.LoginPbType_ServerListReq), server.onServerList)

	for i := 0; i < processMsgGoNum; i++ {
		go server.despatch()
	}
}

func (server *Server) despatch() {
	recvMsg := server.net.GetRecvMsg()
	for msg := range *recvMsg {
		server.despatchMsg(&msg)
	}
}

func (server *Server) despatchMsg(msg *network.MsgData) {
	defer func() {
		x := recover()
		if x != nil {
			log.Printf("%v\n%s", x, string(debug.Stack()))
		}
	}()

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

func (server *Server) onLogin(sess *network.Session, msg []byte) {
	var pbMsg loginpb.LoginReq
	unPbPanic(msg, &pbMsg)
	var user = User{Name: pbMsg.Name}
	var sendMsg loginpb.LoginAck
	sendMsg.Err = loginpb.LoginErrorCode_LoginError
	if server.db.Get(nil, &user) {
		if user.Password == pbMsg.Password {
			sendMsg.Err = loginpb.LoginErrorCode_NoError
			sendMsg.AccountID = user.ID
			sendMsg.Session = uuid.Must(uuid.NewRandom()).String()
			server.userMgr.addUser(user, sess)
			server.userMgr.addUserSession(user.ID, sendMsg.Session)
		}
	}
	sendMsg.RpcId = pbMsg.RpcId
	sendPbMsg(sess, &sendMsg)
}

func (server *Server) onRegister(sess *network.Session, msg []byte) {
	var pbMsg loginpb.RegisterReq
	unPbPanic(msg, &pbMsg)
	var user = User{Name: pbMsg.Name, Password: pbMsg.Password}
	var sendMsg loginpb.RegisterAck
	sendMsg.Err = loginpb.LoginErrorCode_UserNameExisted
	if server.db.Insert(&user) {
		sendMsg.Err = loginpb.LoginErrorCode_NoError
	}
	sendMsg.RpcId = pbMsg.RpcId
	sendPbMsg(sess, &sendMsg)
}

func (server *Server) onServerList(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var pbMsg loginpb.ServerListReq
	unPbPanic(msg, &pbMsg)
	var sendMsg loginpb.ServerListAck
	server.serverList.getServerList(&sendMsg)
	sendMsg.RpcId = pbMsg.RpcId
	sendPbMsg(sess, &sendMsg)
}

func (server *Server) onLogout(sess *network.Session, msg []byte) {
	var user = sess.GetUser()
	if user == nil {
		return
	}
	server.userMgr.delUser(user.(User).ID)
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
	var cmd = loginpb.LoginPbType_TypeEnum_value[name]
	var data = marshalPbPanic(pb)
	var dataLen = len(data)
	if dataLen > network.MaxMsgLen {
		log.Panicf("send msg:%s len is too long\n", name)
		return
	}
	var msg = make([]byte, dataLen+network.MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[:network.MsgLenLen], uint16(dataLen+network.MsgLenLen))
	binary.LittleEndian.PutUint16(msg[network.MsgLenLen:network.MsgHeadLen], uint16(cmd))
	copy(msg[network.MsgHeadLen:], data)
	sess.SendMsg(msg)
}
