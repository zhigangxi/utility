#ifndef _SOCKET_H_
#define _SOCKET_H_
#include <string>
#include <stdint.h>
#include <boost/asio/io_service.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "session.h"
#include <stdint.h>

using namespace std;

class CUser;
class CPackageDeal;
class CMsgHead;

typedef boost::function<void (UserSession userSess,bool)> OnConnectFun;

class CSocket
{
public:
	CSocket(CPackageDeal *pPack);
	bool Accept(uint16_t port,const	char *ip=NULL);
	void Run();
	void SendMsg(UserSession,SharedMsg);
	void SendMsg(UserSession, uint32_t msgType);
	void Stop()
	{
		m_exit = true;
	}
	void Connect(const char *ip,int port,OnConnectFun fun);
private:
	void OnConnect(UserSession sess,const boost::system::error_code &err,OnConnectFun fun);
	void StartAccept();
	void HandleAccept(UserSession,const boost::system::error_code&);
	void RecvHead(UserSession);
	void HandleRecvHead(UserSession,std::shared_ptr<CMsgHead> msgHead,const boost::system::error_code&);
	void HandleRecvBody(UserSession, std::shared_ptr<CMsgHead> msgHead, std::shared_ptr<uint8_t> msgData,const boost::system::error_code&);
	void HandleSend(UserSession,SharedData msgData,const boost::system::error_code&);
	std::shared_ptr<CMsgHead> GetMsgHead(SharedMsg &msg);
		
	boost::asio::io_service m_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
	CPackageDeal *m_pPack;
	bool m_printMsg;
	boost::mutex m_mutex;
	bool m_exit;
	const static int MAX_MSG_LEN = 65536;
};

#endif
