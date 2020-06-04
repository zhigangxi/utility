package game

import (
	"server/network"
	gamepb "server/proto/game"
	"strings"
)

const (
	minCmdLen = 10
)

func (server *Server) onAdminCmd(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	var pbMsg gamepb.AdminCmdReq
	unPbPanic(msg, &pbMsg)
	if len(pbMsg.Cmd) < minCmdLen {
		return
	}

	var sendMsg gamepb.AdminCmdAck
	if pbMsg.UserID != 0 && user.AdminLevel < AdminLvCmdOther {
		sendMsg.Success = false
		sendMsg.ErrStr = "权限不足"
		sendPbMsg(sess, &sendMsg)
		return
	} else if (pbMsg.UserID == 0 || pbMsg.UserID == user.ID) && user.AdminLevel < AdminLvCmdMe {
		sendMsg.Success = false
		sendMsg.ErrStr = "权限不足"
		sendPbMsg(sess, &sendMsg)
		return
	}
	var cmds = strings.Fields(pbMsg.Cmd)
	var errStr string
	switch cmds[0] {
	case "AddPackage":
		errStr = server.adminAddPackage(&cmds, pbMsg.UserID)
	default:
		errStr = "未知命令"
	}
	if errStr == "" {
		sendMsg.Success = true
	} else {
		sendMsg.Success = false
		sendMsg.ErrStr = errStr
	}
	sendPbMsg(sess, &sendMsg)
}

func (server *Server) adminAddPackage(cmds *[]string, userID int64) string {
	return ""
}
