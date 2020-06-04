package game

import (
	"server/network"
	gamepb "server/proto/game"
	rpcpb "server/proto/rpc"

	"xorm.io/xorm"
)

type queryUser struct {
	session *network.Session
	user    *User
}

func (query *queryUser) QueryCB(engine *xorm.Engine) {
	engine.ID(query.user.ID).Get(query.user)
}

func (query *queryUser) ResultCB() {
	var sendMsg gamepb.EnterGameAck
	query.session.SetUser(query.user)
	server.userMgr.AddUser(query.user)
	if len(query.user.Nick) == 0 {
		sendMsg.Error = gamepb.ErrorCode_NotCreateUser
		sendPbMsg(query.session, &sendMsg)
		return
	}

	server.onlineUserNum = len(*server.userMgr)
	sendMsg.Error = gamepb.ErrorCode_NoError
	sendMsg.Info = new(gamepb.UserInfo)
	copyUserToPb(query.user, sendMsg.Info)
	sendPbMsg(query.session, &sendMsg)
}

func (server *Server) onGetSession(data []byte, err error) {
	if err != nil {
		var sendMsg gamepb.EnterGameAck
		sendMsg.Error = gamepb.ErrorCode_GameSessionError
		for k, v := range server.onGetUserSess {
			sendPbMsg(v.session, &sendMsg)
			delete(server.onGetUserSess, k)
		}
		return
	}
	var pbMsg rpcpb.GetLoginSessReply
	unPbPanic(data, &pbMsg)
	if pbMsg.UserId == 0 || len(pbMsg.Session) <= 0 {
		return
	}
	userSess, ok := server.onGetUserSess[pbMsg.UserId]
	if !ok {
		return
	}

	delete(server.onGetUserSess, pbMsg.UserId)
	var sendMsg gamepb.EnterGameAck
	if pbMsg.Session != userSess.sessStr {
		sendMsg.Error = gamepb.ErrorCode_GameSessionError
		sendPbMsg(userSess.session, &sendMsg)
		return
	}
	var query = new(queryUser)
	query.user = new(User)
	query.session = userSess.session
	query.user.ID = pbMsg.UserId
	server.db.Query(query)
}

type userSession struct {
	session *network.Session
	sessStr string
}

func (server *Server) onEnterGame(sess *network.Session, msg []byte) {
	var pbMsg gamepb.EnterGameReq
	unPbPanic(msg, &pbMsg)
	if pbMsg.UserID == 0 || len(pbMsg.Session) <= 0 {
		var sendMsg gamepb.EnterGameAck
		sendMsg.Error = gamepb.ErrorCode_GameSessionError
		sendPbMsg(sess, &sendMsg)
		return
	}
	server.onGetUserSess[pbMsg.UserID] = userSession{session: sess, sessStr: pbMsg.Session}
	var rpcMsg rpcpb.GetLoginSessArg
	rpcMsg.UserId = pbMsg.UserID
	server.rpc.GoCall(&rpcMsg)
}

type insertUser struct {
	session *network.Session
	user    *User
}

func (query *insertUser) QueryCB(engine *xorm.Engine) {
	_, err := engine.InsertOne(query.user)
	if err != nil {
		query.user.Nick = ""
	}
}

func (query *insertUser) ResultCB() {
	var sendMsg gamepb.CreateUserAck
	if len(query.user.Nick) <= 0 {
		sendMsg.Error = gamepb.ErrorCode_NickAlreadyExist
		sendPbMsg(query.session, &sendMsg)
		return
	}
	sendMsg.Info = new(gamepb.UserInfo)
	copyUserToPb(query.user, sendMsg.Info)
	sendPbMsg(query.session, &sendMsg)
}

func copyUserToPb(user *User, pbUser *gamepb.UserInfo) {
	pbUser.Exp = user.Exp
	pbUser.Gold = user.Gold
	pbUser.Level = user.Level
	pbUser.Nick = user.Nick
	pbUser.Score = user.Score
}

func (server *Server) onCreateUser(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}

	var user = sess.GetUser().(*User)
	var pbMsg gamepb.CreateUserReq
	unPbPanic(msg, &pbMsg)
	var nickLen = len([]rune(pbMsg.Nick))
	if len(user.Nick) > 0 {
		var sendMsg gamepb.CreateUserAck
		sendMsg.Error = gamepb.ErrorCode_UserIsCreated
		sendPbMsg(sess, &sendMsg)
		return
	} else if nickLen < 3 {
		var sendMsg gamepb.CreateUserAck
		sendMsg.Error = gamepb.ErrorCode_NickIsTooShort
		sendPbMsg(sess, &sendMsg)
		return
	} else if nickLen > 12 {
		var sendMsg gamepb.CreateUserAck
		sendMsg.Error = gamepb.ErrorCode_NickIsTooLong
		sendPbMsg(sess, &sendMsg)
		return
	}
	var query = new(insertUser)
	query.user = user
	query.user.Nick = pbMsg.Nick
	query.user.Level = 1
	server.db.Query(query)
}

func (server *Server) onLogout(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	user.Save(server.db)
	server.userMgr.DelUser(user.ID)
	server.onlineUserNum = len(*server.userMgr)
}
