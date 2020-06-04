package game

import (
	"log"
	database "server/db"
	"server/network"
	rpcpb "server/proto/rpc"
	rpcUtil "server/rpc"
	"server/utility"
	"time"
)

type Config struct {
	Database    string `json:"database"`
	BindIp      string `json:"bind_ip"`
	ClientUseIp string `json:"client_use_ip"`
	ServerName  string `json:"server_name"`
	LoginRpc    string `json:"login_rpc"`
	OrderId     int    `json:"order_id"`
	ConfigPath  string `json:"config_path"`
}

type Server struct {
	config        Config
	net           *network.NetWork
	rpc           *rpcUtil.Client
	db            *database.Database
	userMgr       *UserMgr
	procFuns      map[uint16]ProcessMsgCB
	onGetUserSess map[int64]userSession
	onlineUserNum int
}

var (
	server *Server
)

const (
	maxUserNum = 20000
)

func init() {
	server = new(Server)
	server.userMgr = NewUserMgr(maxUserNum)
	server.procFuns = make(map[uint16]ProcessMsgCB)
	server.onGetUserSess = make(map[int64]userSession)
}

func GetGameServer() *Server {
	return server
}

func (server *Server) Start(configFile string) {
	if !utility.ReadJSONFile(configFile, &(server.config)) {
		log.Printf("read login_config.json error\n")
		return
	}
	server.rpc = rpcUtil.StartRPCClient(server.config.LoginRpc)
	if server.rpc == nil {
		log.Printf("Create rcp error\n")
		return
	}
	var err error
	if server.db, err = database.NewDB(server.config.Database); err != nil {
		log.Printf("init db error:%v", err)
		return
	}

	server.net = network.NewNetWork(network.Tcp, server.config.BindIp)
	if server.net == nil {
		log.Printf("Create network error\n")
		return
	}
	go server.CommitServerInfo()

	server.processMsg()

	err = server.net.Start()
	if err != nil {
		log.Println(err)
	}
}

func (server *Server) CommitServerInfo() {
	ticker := time.NewTicker(time.Second * 5)
	var serverInfo rpcpb.GameServerInfo
	serverInfo.OrderId = int32(server.config.OrderId)
	serverInfo.ServerIp = server.config.ClientUseIp
	serverInfo.ServerName = server.config.ServerName
	for {
		<-ticker.C
		serverInfo.UserNum = int32(server.onlineUserNum)
		server.rpc.CallServer(&serverInfo, nil)
	}
}
