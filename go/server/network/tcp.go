package network

import (
	"bufio"
	"encoding/binary"
	"io"
	"log"
	"net"
)

func (tcpNet *NetWork) tcpHandler(conn net.Conn) {
	var session = tcpNet.newSession(conn)
	go session.tcpReadMsg()
	for msg := range session.sendBuf {
		if len(msg) > 0 {
			if _, err := session.conn.Write(msg); err != nil {
				break
			}
		}
	}
}

func (sess *Session) tcpReadMsg() {
	defer sess.Close()
	reader := bufio.NewReader(sess.conn)
	var msgHead = make([]byte, MsgHeadLen)
	var msgLen uint16
	for {
		//sess.conn.(*net.TCPConn).Read()
		_, err := io.ReadFull(reader, msgHead)
		if err != nil {
			return
		}
		msgLen = binary.LittleEndian.Uint16(msgHead[:MsgLenLen])
		if msgLen > MaxMsgLen {
			log.Printf("recv msg is to length:%d\n", msgLen)
			return
		}
		var msg = make([]byte, msgLen+MsgHeadLen)
		copy(msg, msgHead)
		_, err = io.ReadFull(reader, msg[MsgHeadLen:])
		if err != nil {
			return
		}
		sess.net.msgBuf <- MsgData{Sess: sess, Msg: msg}
	}
}
