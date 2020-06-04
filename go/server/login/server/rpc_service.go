package login

import (
	"log"
	loginpb "server/proto/login"
	rpcpb "server/proto/rpc"
	rpcUtil "server/rpc"
	"sort"
	"time"

	"github.com/golang/protobuf/proto"
)

func (server *Server) StartRpcService() bool {
	server.rpc = rpcUtil.StartRPCServer(server.config.RpcIp)
	if server.rpc == nil {
		log.Printf("Create rpc error\n")
		return false
	}
	server.rpc.RegisterCallBack(rpcpb.PbType_GetLoginSessArg, server.GetSession)
	server.rpc.RegisterCallBack(rpcpb.PbType_GameServerInfo, server.GameServerInfo)
	return true
}

func (server *Server) GameServerInfo(arg proto.Message, reply *proto.Message) error {
	var info gameServerInfo
	var pb = arg.(*rpcpb.GameServerInfo)
	info.ip = pb.ServerIp
	info.name = pb.ServerName
	info.orderID = int(pb.OrderId)
	info.userNum = int(pb.UserNum)
	info.updateTime = time.Now().Unix()
	server.serverList.addServerList(&info)
	return nil
}

func (server *Server) GetSession(arg proto.Message, reply *proto.Message) error {
	var userId = arg.(*rpcpb.GetLoginSessArg).UserId
	var session = server.userMgr.getUserSession(userId)
	var sessReply = (*reply).(*rpcpb.GetLoginSessReply)
	sessReply.UserId = userId
	sessReply.Session = session
	return nil
}

func (list *gameServerList) addServerList(info *gameServerInfo) {
	var have bool
	var needSort bool
	info.updateTime = time.Now().Unix()
	list.mutex.Lock()
	for k := range list.gameServerInfo {
		if list.gameServerInfo[k].ip == info.ip {
			if list.gameServerInfo[k].orderID != info.orderID {
				needSort = true
			}
			list.gameServerInfo[k] = *info
			have = true
			break
		}
	}
	if !have {
		list.gameServerInfo = append(list.gameServerInfo, *info)
		needSort = true
	}
	if needSort {
		list.updateServerList()
	}
	list.mutex.Unlock()
}

type gameInfos []gameServerInfo

func (info gameInfos) Len() int {
	return len(info)
}

func (info gameInfos) Less(i, j int) bool {
	return info[i].orderID < info[j].orderID
}

func (info gameInfos) Swap(i, j int) {
	info[i], info[j] = info[j], info[i]
}

func (list *gameServerList) updateServerList() {
	sort.Sort(gameInfos(list.gameServerInfo))
	list.pbServerList.Infos = list.pbServerList.Infos[:0]
	for _, v := range list.gameServerInfo {
		var info loginpb.ServerInfo
		info.Ip = v.ip
		info.Name = v.name
		list.pbServerList.Infos = append(list.pbServerList.Infos, &info)
	}
}

func (list *gameServerList) getServerList(info *loginpb.ServerListAck) {
	list.mutex.Lock()
	info.Infos = list.pbServerList.Infos
	list.mutex.Unlock()
}
