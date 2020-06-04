package network

import (
	"golang.org/x/net/websocket"
)

func (wsNet *NetWork) wsHandler(ws *websocket.Conn) {
	var session = wsNet.newSession(ws)
	go session.wsReadMsg()
	for msg := range session.sendBuf {
		if len(msg) > 0 {
			if err := websocket.Message.Send(ws, msg); err != nil {
				break
			}
		}
	}
	//log.Printf("exit wsHandler\n")
}

func (sess *Session) wsReadMsg() {
	var msg []byte
	defer sess.Close()
	for {
		if websocket.Message.Receive(sess.conn.(*websocket.Conn), &msg) != nil {
			break
		}
		sess.net.msgBuf <- MsgData{Sess: sess, Msg: msg}
	}
}
