package network

import (
	"encoding/binary"
	"log"
	"net"
)

type Session struct {
	net     *NetWork
	conn    net.Conn
	isClose bool
	sendBuf chan []byte
	user    interface{}
}

func (net *NetWork) newSession(conn net.Conn) *Session {
	var sess = new(Session)
	sess.net = net
	sess.conn = conn
	sess.sendBuf = make(chan []byte, MaxSendBufLen)
	return sess
}

func (sess *Session) SetUser(user interface{}) {
	sess.user = user
}
func (sess *Session) GetUser() interface{} {
	return sess.user
}

func (sess *Session) SendMsg(msg []byte) {
	if len(msg) <= 0 {
		return
	}
	sess.sendBuf <- msg
}

func (sess *Session) SendMsgHead(cmd uint16) {
	if cmd == 0 {
		log.Printf("SendMsgHead cmd error")
		return
	}
	var msg = make([]byte, MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[MsgLenLen:MsgHeadLen], cmd)
	sess.sendBuf <- msg
}

func (sess *Session) Close() {
	if sess.IsClose() {
		return
	}
	sess.isClose = true
	sess.conn.Close()
	close(sess.sendBuf)
	//var msg = make([]byte, MsgHeadLen)
	sess.net.msgBuf <- MsgData{Sess: sess, Msg: nil}
}

func (sess *Session) IsClose() bool {
	return sess.isClose
}
