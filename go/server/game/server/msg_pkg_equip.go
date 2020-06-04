package game

import (
	"server/network"
	gamepb "server/proto/game"
)

func (server *Server) onPackage(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	var sendMsg gamepb.PackageAck
	copyUserPackToPb(&user.packages, &sendMsg)
	sendPbMsg(sess, &sendMsg)
}

func copyUserPackToPb(userPack *UserPackage, pb *gamepb.PackageAck) {
	if len(*userPack) <= 0 {
		return
	}
	for _, v := range *userPack {
		var info gamepb.PackageInfo
		info.ClassID = v.Item.Class.ID
		info.Id = v.Item.ID
		info.Num = v.Num
		info.Pos = v.Pos
		pb.Infos = append(pb.Infos, &info)
	}
}

func (server *Server) onEquipment(sess *network.Session, msg []byte) {
	if sess.GetUser() == nil {
		return
	}
	var user = sess.GetUser().(*User)
	var sendMsg gamepb.EquipmentAck
	copyUserEquipToPb(&user.equipments, &sendMsg)
	sendPbMsg(sess, &sendMsg)
}

func copyUserEquipToPb(userPack *UserEquipments, pb *gamepb.EquipmentAck) {
	if len(*userPack) <= 0 {
		return
	}
	for _, v := range *userPack {
		var info gamepb.EquipmentInfo
		info.ClassID = v.Item.Class.ID
		info.Id = v.Item.ID
		info.Pos = v.Pos
		pb.Equips = append(pb.Equips, &info)
	}
}
