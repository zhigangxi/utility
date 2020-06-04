package client

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"log"
	"net"
	"server/network"
	gamepb "server/proto/game"
	loginpb "server/proto/login"
	"time"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/websocket"
)

type Client struct {
	//ws           *websocket.Conn
	conn         net.Conn
	Name         string
	Password     string
	AccountID    int64
	Session      string
	UserID       int64
	GameServerIp string
	connType     string
}

var (
	loginPkName = "loginpb."
	gamePkName  = "gamepb."
	loginPkLen  = len(loginPkName)
	gamePkLen   = len(gamePkName)
)

func (client *Client) Connect(connType string, address string) error {
	var err error
	if connType == "ws" {
		client.conn, err = websocket.Dial(address, "ws", "/")
	} else if connType == "tcp" {
		addr, err := net.ResolveTCPAddr("tcp", address)
		if err != nil {
			return err
		}
		client.conn, err = net.DialTCP("tcp", nil, addr)
	} else {
		return errors.New("error connect type")
	}
	client.connType = connType
	// websocket.Message.Send(ws, msg); err != nil {
	//if websocket.Message.Receive(sess.conn.(*websocket.Conn), &msg) != nil {
	return err
}

func (client *Client) RecvMsg(timeOut int) ([]byte, error) {
	if client.connType == "ws" {
		client.conn.(*websocket.Conn).SetReadDeadline(time.Now().Add(time.Duration(timeOut) * time.Millisecond))
		var msg []byte
		err := websocket.Message.Receive(client.conn.(*websocket.Conn), &msg)
		return msg, err
	} else {
		client.conn.(*net.TCPConn).SetReadDeadline(time.Now().Add(time.Duration(timeOut) * time.Millisecond))
		reader := bufio.NewReader(client.conn)
		var msgHead = make([]byte, network.MsgHeadLen)
		var msgLen uint16
		_, err := io.ReadFull(reader, msgHead)
		if err != nil {
			return nil, err
		}
		msgLen = binary.LittleEndian.Uint16(msgHead[:network.MsgLenLen])
		if msgLen > network.MaxMsgLen {
			var errmsg = fmt.Sprintf("recv msg is to length:%d\n", msgLen)
			return nil, errors.New(errmsg)
		}
		var msg = make([]byte, msgLen+network.MsgHeadLen)
		copy(msg, msgHead)
		_, err = io.ReadFull(reader, msg[network.MsgHeadLen:])
		if err != nil {
			return nil, err
		}
		return msg, nil
	}
}

func (client *Client) SendLoginMsg(pb proto.Message) {
	if client.conn == nil {
		return
	}
	var name = proto.MessageName(pb)
	if len(name) <= loginPkLen {
		log.Printf("pb name:%s error", name)
		return
	}
	name = name[loginPkLen:]
	var cmd = loginpb.PbType_TypeEnum_value[name]
	data, err := proto.Marshal(pb)
	if err != nil {
		log.Printf("Marshal %s error %s\n", name, err.Error())
		return
	}
	var dataLen = len(data)
	if dataLen > network.MaxMsgLen {
		log.Printf("send msg:%s len is too long\n", name)
		return
	}
	var msg = make([]byte, dataLen+network.MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[:network.MsgLenLen], uint16(dataLen))
	binary.LittleEndian.PutUint16(msg[network.MsgLenLen:network.MsgHeadLen], uint16(cmd))
	copy(msg[network.MsgHeadLen:], data)
	if client.connType == "ws" {
		websocket.Message.Send(client.conn.(*websocket.Conn), msg)
	} else {
		client.conn.Write(msg)
	}
	//client.ws.Write(msg)
}

func (client *Client) SendGameMsg(pb proto.Message) {
	if client.conn == nil {
		return
	}
	var name = proto.MessageName(pb)
	if len(name) <= gamePkLen {
		log.Printf("pb name:%s error", name)
		return
	}
	name = name[gamePkLen:]
	var cmd = gamepb.PbType_TypeEnum_value[name]
	data, err := proto.Marshal(pb)
	if err != nil {
		log.Printf("Marshal %s error %s\n", name, err.Error())
		return
	}
	var dataLen = len(data)
	if dataLen > network.MaxMsgLen {
		log.Printf("send msg:%s len is too long\n", name)
		return
	}
	var msg = make([]byte, dataLen+network.MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[:network.MsgLenLen], uint16(dataLen))
	binary.LittleEndian.PutUint16(msg[network.MsgLenLen:network.MsgHeadLen], uint16(cmd))
	copy(msg[network.MsgHeadLen:], data)
	//client.ws.Write(msg)
	//websocket.Message.Send(client.ws, msg)
	if client.connType == "ws" {
		websocket.Message.Send(client.conn.(*websocket.Conn), msg)
	} else {
		client.conn.Write(msg)
	}
}

func (client *Client) SendMsgHead(cmd uint16) {
	var msg = make([]byte, network.MsgHeadLen)
	binary.LittleEndian.PutUint16(msg[network.MsgLenLen:network.MsgHeadLen], cmd)
	//websocket.Message.Send(client.ws, msg)
	if client.connType == "ws" {
		websocket.Message.Send(client.conn.(*websocket.Conn), msg)
	} else {
		client.conn.Write(msg)
	}
}
