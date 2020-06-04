package client

import (
	"encoding/binary"
	"log"
	"server/network"
	loginpb "server/proto/login"

	"google.golang.org/protobuf/proto"
)

func (client *Client) ProcessLoginMsg(msg []byte) {
	if len(msg) < network.MsgHeadLen {
		return
	}
	var cmd = binary.LittleEndian.Uint16(msg[network.MsgLenLen:network.MsgHeadLen])
	var msgData = msg[network.MsgHeadLen:]
	switch loginpb.PbType_TypeEnum(cmd) {
	case loginpb.PbType_LoginAck:
		client.onLogin(msgData)
	case loginpb.PbType_RegisterAck:
		client.onRegister(msgData)
	case loginpb.PbType_ServerListAck:
		client.onServerSlist(msgData)
	}
}

func (client *Client) onLogin(msg []byte) {
	var pb loginpb.LoginAck
	if proto.Unmarshal(msg, &pb) != nil {
		log.Printf("Unmarshal pb error\n")
	}
	if pb.Err != loginpb.ErrorCode_NoError {
		client.conn.Close()
		log.Printf("login error:%d\n", pb.Err)
		return
	}
	log.Printf("account id:%d login ok\n", pb.AccountID)
	client.AccountID = pb.AccountID
	client.Session = pb.Session
	client.SendMsgHead(uint16(loginpb.PbType_ServerListReq))
}

func (client *Client) onRegister(msg []byte) {
	var pb loginpb.RegisterAck
	if proto.Unmarshal(msg, &pb) != nil {
		log.Printf("Unmarshal pb error\n")
	}
	if pb.Err != loginpb.ErrorCode_NoError {
		client.conn.Close()
		log.Printf("register error:%d\n", pb.Err)
		return
	}
	var sendMsg loginpb.LoginReq
	sendMsg.Name = client.Name
	sendMsg.Password = client.Password
	client.SendLoginMsg(&sendMsg)
}

func (client *Client) onServerSlist(msg []byte) {

}

func (client *Client) ProcessGameMsg(msg []byte) {

}

func (client *Client) OnLoginIdle() {

}

func (client *Client) OnGameIdle() {

}
