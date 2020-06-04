package network

// import (
// 	"encoding/binary"
// )

// func (network *NetWork) callCmdMsg(sess *Session, msg []byte) {
// 	if msg != nil && len(msg) < msgHeadLen {
// 		return
// 	}

// 	var cmd uint16
// 	if len(msg) >= msgHeadLen {
// 		cmd = binary.LittleEndian.Uint16(msg[msgLenLen:msgHeadLen])
// 	}
// 	if fun, ok := network.procFuns[cmd]; ok {
// 		if msg == nil {
// 			fun(sess, nil) //socket disconnect
// 		} else {
// 			fun(sess, msg[msgHeadLen:])
// 		}
// 	}
// }

// func (network *NetWork) ProcessMsg() bool {
// 	select {
// 	case msg := <-network.recvBuf:
// 		network.callCmdMsg(msg.sess, msg.buf)
// 		return true
// 	default:
// 		return false
// 	}
// }
