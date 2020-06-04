package network

import (
	"errors"
	"fmt"
	"net"
	"net/http"

	"golang.org/x/net/websocket"
)

const (
	Tcp           = 1
	Websocket     = 2
	MaxRecvBufLen = 1024
	MaxSendBufLen = 16

	//协议包头四个字节，包长两个字节，包类型两个字节
	MsgHeadLen = 4    //包头大小
	MsgLenLen  = 2    //包长度大小
	MsgTypeLen = 2    //包头里面表示协议类型数据大小
	MaxMsgLen  = 8192 //消息的最大长度
)

type NetWork struct {
	socketType int
	address    string
	msgBuf     chan MsgData
	//procFuns   map[uint16]ProcessMsgCB
}

type MsgData struct {
	Sess *Session
	Msg  []byte
}

func NewNetWork(socketType int, address string) *NetWork {
	if socketType != Tcp && socketType != Websocket {
		return nil
	}
	var network = new(NetWork)
	network.address = address
	network.socketType = socketType
	network.msgBuf = make(chan MsgData, MaxRecvBufLen)
	//network.procFuns = make(map[uint16]ProcessMsgCB)
	return network
}

func (data *NetWork) Start() error {
	if data.socketType == Tcp {
		netListen, err := net.Listen("tcp", data.address)
		defer netListen.Close()
		if err != nil {
			return err
		}
		for {
			conn, err := netListen.Accept()
			if err != nil {
				continue
			}
			fmt.Printf("connect:ip %s\n", conn.RemoteAddr().String())
			go data.tcpHandler(conn)
		}
	} else if data.socketType == Websocket {
		http.Handle("/", websocket.Handler(data.wsHandler))
		err := http.ListenAndServe(data.address, nil)
		if err != nil {
			return err
		}
	} else {
		return errors.New("error netwrok type")
	}
	return nil
}

func (network *NetWork) GetRecvMsg() *chan MsgData {
	return &(network.msgBuf)
}

// func (network *NetWork) RegisterProcFun(cmd uint16, cb ProcessMsgCB) {
// 	if _, ok := network.procFuns[cmd]; ok {
// 		log.Printf("cmd %d is already register\n", cmd)
// 	}
// 	network.procFuns[cmd] = cb
// }
