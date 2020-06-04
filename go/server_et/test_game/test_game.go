package main

import (
	"fmt"
	"io"
	"log"
	"net"
	loginpb "server/proto/login"
	"server/test_game/client"
	"sync/atomic"
	"time"
)

var (
	clientNum int32
)

func createClient(connType string, addr string) {
	client := new(client.Client)
	err := client.Connect(connType, addr)
	if err != nil {
		log.Printf("%s\n", err.Error())
		return
	}
	var sendMsg loginpb.RegisterReq
	sendMsg.Name = fmt.Sprintf("%d", time.Now().UnixNano()/1000)
	sendMsg.Password = sendMsg.Name
	client.Name = sendMsg.Name
	client.Password = sendMsg.Password
	client.SendLoginMsg(&sendMsg)

	atomic.AddInt32(&clientNum, 1)
	for {
		msg, err := client.RecvMsg(1000)
		neterr, ok := err.(net.Error)
		if ok && neterr.Timeout() {
			client.OnLoginIdle()
			continue
		}
		if err == io.EOF {
			log.Printf("login connect is closed\n")
			break
		}
		if err == nil {
			client.ProcessLoginMsg(msg)
		} else {
			log.Println(err)
		}
	}
	if len(client.GameServerIp) >= 0 {
		log.Printf("can not get game server ip\n")
		atomic.AddInt32(&clientNum, -1)
		return
	}
	if connType == "ws" {
		err = client.Connect(connType, "ws://"+client.GameServerIp)
	} else {
		err = client.Connect(connType, client.GameServerIp)
	}
	if err != nil {
		log.Printf("%s\n", err.Error())
		atomic.AddInt32(&clientNum, -1)
		return
	}
	for {
		msg, err := client.RecvMsg(1000)
		neterr, ok := err.(net.Error)
		if ok && neterr.Timeout() {
			client.OnGameIdle()
			continue
		}
		if err == io.EOF {
			log.Printf("game connect is closed\n")
			break
		}
		if err == nil {
			client.ProcessGameMsg(msg)
		}
	}
	atomic.AddInt32(&clientNum, -1)
}

func main() {
	createClient("tcp", "127.0.0.1:6000")
}
