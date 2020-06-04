package game

import (
	"log"
	"server/network"
	gamepb "server/proto/game"
	"time"
)

func (server *Server) onPlayerMove(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	if user.scene == nil {
		log.Printf("user:%d not enter scene\n", user.ID)
		return
	}
	var pbMsg gamepb.PlayerMove
	unPbPanic(msg, &pbMsg)
	user.X = pbMsg.X
	user.Y = pbMsg.Y
	var sendMsg gamepb.OtherPlayerMove
	sendMsg.Timestamp = time.Now().UnixNano() / 1e6
	sendMsg.Speed = pbMsg.Speed
	sendMsg.UserID = user.ID
	sendMsg.X = user.X
	sendMsg.Y = user.Y
	user.scene.BrocastMsg(&sendMsg)

}

func (server *Server) inJumpPoint(user *User) {
	if user.scene.JumpPoint == nil {
		return
	}
	for _, v := range user.scene.JumpPoint {
		if v.InJumpPoint(user.X, user.Y) {
			server.EnterScene(user, v.ToID)
			var sendMsg gamepb.JumpScene
			sendMsg.X = user.X
			sendMsg.Y = user.Y
			sendMsg.ToSceneID = v.ToID
			sendPbMsg(user.Session, &sendMsg)
			return
		}
	}
}

func (server *Server) onEnterScene(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	var pbMsg gamepb.EnterScene
	unPbPanic(msg, &pbMsg)
	if !server.EnterScene(user, pbMsg.SceneID) {
		log.Printf("user:%d enter scene:%d error\n", user.ID, pbMsg.SceneID)
		return
	}
	var sendMsg gamepb.PlayerEnterScene
	sendMsg.SceneID = pbMsg.SceneID
	sendMsg.UserID = user.ID
	sendMsg.X = user.X
	sendMsg.Y = user.Y
	sendMsg.Timestamp = time.Now().UnixNano() / 1e6
	user.scene.BrocastMsg(&sendMsg)
}
