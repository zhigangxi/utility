package login

import (
	"log"
	database "server/db"
	"server/network"
	loginpb "server/proto/login"
	rpcUtil "server/rpc"
	"server/utility"
	"sync"
)

type Config struct {
	Database string `json:"database"`
	ServerIp string `json:"server_ip"`
	RpcIp    string `json:"rpc_ip"`
}

type Server struct {
	config     Config
	net        *network.NetWork
	rpc        *rpcUtil.Server
	userMgr    *userMgr
	db         *database.Database
	procFuns   map[uint16]ProcessMsgCB
	serverList *gameServerList
}

type gameServerList struct {
	gameServerInfo []gameServerInfo
	pbServerList   loginpb.ServerListAck
	mutex          sync.Mutex
}

type gameServerInfo struct {
	ip         string
	name       string
	userNum    int
	orderID    int
	updateTime int64
}

var (
	server *Server
)

func init() {
	server = new(Server)
	server.serverList = new(gameServerList)
	server.userMgr = newUserMgr()
	server.procFuns = make(map[uint16]ProcessMsgCB)
}

func GetLoginServer() *Server {
	return server
}

func (server *Server) Start() {
	if !utility.ReadJSONFile("login_cfg.json", &server.config) {
		log.Printf("read login_config.json error\n")
		return
	}
	if !server.StartRpcService() {
		return
	}
	var err error
	if server.db, err = database.NewDB(server.config.Database); err != nil {
		log.Printf("init db error:%v", err)
		return
	}

	server.net = network.NewNetWork(network.Tcp, server.config.ServerIp)
	if server.net == nil {
		log.Printf("Create network error\n")
		return
	}
	server.processMsg()

	err = server.net.Start()
	if err != nil {
		log.Println(err)
	}
}
