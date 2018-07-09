#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include "user.h"
#include "socket.h"
#include "PackageDeal.h"
#include "ini.h"
#include "utility.h"
#include <thread>
#include "GlobalVal.h"
#include "MsgHead.h"
#include "NetMsg.pb.h"
#include <google/protobuf/text_format.h>

CSocket::CSocket(CPackageDeal *pPack):m_acceptor(m_service)
{
	m_pPack = pPack;
}

bool CSocket::Accept(uint16_t port,const char *ip)
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	if(ip != NULL)
		endpoint.address(boost::asio::ip::address::from_string(ip));
	m_acceptor.open(endpoint.protocol());
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	boost::system::error_code err;
	err = m_acceptor.bind(endpoint,err);
	if(err)
	{
		return false;
	}
	m_acceptor.listen(128,err);	
	if(err)
	{
		return false;
	}
	StartAccept();
	return true;
}

void CSocket::OnConnect(UserSession sess,const boost::system::error_code &err,OnConnectFun fun)
{
	bool success;
	if(err)
		success = false;
	else
		success = true;;
	fun(sess,success);
}
void CSocket::Connect(const char *ip,int port,OnConnectFun fun)
{
	if(ip == NULL)
		return;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	endpoint.address(boost::asio::ip::address::from_string(ip));
	UserSession newSession(new CSession(m_service));
	newSession->async_connect(endpoint,boost::bind(&CSocket::OnConnect,this,newSession,boost::asio::placeholders::error,fun));
}
void CSocket::StartAccept()
{
	UserSession newSession(new CSession(m_service));	
	m_acceptor.async_accept(*newSession,boost::bind(&CSocket::HandleAccept, this, newSession,boost::asio::placeholders::error));
}

void CSocket::HandleRecvBody(UserSession session, std::shared_ptr<CMsgHead> msgHead, std::shared_ptr<uint8_t> msgData,const boost::system::error_code &error)
{
	uint32_t msgSize = msgHead->GetSize();
	if(error || msgSize > MAX_MSG_LEN)
	{
		cout<<error.message()<<endl;
		//添加到package,处理user logout
		m_pPack->Logout(session);
		session->close();
		return;
	}

	std::string typeName = NetMsg::MessageType_Type_Name((NetMsg::MessageType_Type)msgHead->GetType());
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("NetMsg." + typeName);
	if (!descriptor) {
		cout << "get Descriptor error" << endl;
		return;
	}

	const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
	if (!prototype) {
		cout << "get message error" << endl;
		return;
	}

	message = prototype->New();
	message->ParseFromArray(msgData.get(), msgSize);

#ifdef _DEBUG
	cout << "recv msg:" << typeName << endl;
	std::string strMsg;
	google::protobuf::TextFormat::PrintToString(*message, &strMsg);
	cout << strMsg << endl;
#endif

	m_pPack->AddPack(session,msgHead->GetType(), SharedMsg(message));
	RecvHead(session);
}

void CSocket::HandleRecvHead(UserSession session, std::shared_ptr<CMsgHead> msgHead, const boost::system::error_code &error)
{
	uint32_t msgSize = msgHead->GetSize();
	if(error || msgSize > MAX_MSG_LEN)
	{
		//添加到package,处理user logout
		m_pPack->Logout(session);
		session->close();
		return;
	}
	if (msgSize == 0)
	{
#ifdef _DEBUG
		std::string typeName = NetMsg::MessageType_Type_Name((NetMsg::MessageType_Type)msgHead->GetType());
		cout << "recv msg:" << typeName << endl;
#endif
		m_pPack->AddPack(session, msgHead->GetType(), nullptr);
		RecvHead(session);
		return;
	}

	std::shared_ptr<uint8_t> buf(new uint8_t[msgSize]);
	boost::asio::async_read(*session,boost::asio::buffer(buf.get(),msgSize),
			boost::bind(&CSocket::HandleRecvBody,this,session,msgHead,buf,boost::asio::placeholders::error));
}

void CSocket::RecvHead(UserSession pSession)
{
	std::shared_ptr<CMsgHead> head(new CMsgHead);
	boost::asio::async_read(*pSession,boost::asio::buffer(head->GetData(),head->GetHeadSize()),
			boost::bind(&CSocket::HandleRecvHead,this,pSession,head,boost::asio::placeholders::error));
}

void CSocket::HandleAccept(UserSession pSession,const boost::system::error_code& error)
{
	if(error)
	{
		return;
	}
	else
	{
		RecvHead(pSession);
	}
	StartAccept();
}

void CSocket::HandleSend(UserSession pSession,SharedData msgData,const boost::system::error_code &error)
{
	if(error)
	{
		m_pPack->Logout(pSession);
		pSession->close();
		return;
	}
	SharedData msg;
	bool sendMsgBuf = false;
	{
		boost::mutex::scoped_lock lk(m_mutex);
		pSession->DelSendBuf(msgData);
		if(pSession->GetSendBuf(msg))
		{//find sendbuf to send
			sendMsgBuf = true;
		}
	}
	if(sendMsgBuf)
	{
		boost::asio::async_write(*pSession,boost::asio::buffer(msg->c_str(),msg->size()),
								 boost::bind(&CSocket::HandleSend,this,pSession,msg,boost::asio::placeholders::error));
	}
}

std::shared_ptr<CMsgHead> CSocket::GetMsgHead(SharedMsg &msg)
{
	std::shared_ptr<CMsgHead> head(new CMsgHead);

	std::string typeName = msg->GetTypeName();
	static int len = strlen("NetMsg.");
	typeName.insert(len, "MessageType.");
	const google::protobuf::EnumValueDescriptor* enum_value_descriptor = google::protobuf::DescriptorPool::generated_pool()->FindEnumValueByName(typeName);
	if (!enum_value_descriptor)
	{
		cout << "Get invalid message:" << typeName.c_str() << endl;
		return head;
	}
	int32_t msgType = enum_value_descriptor->number();
	head->SetType(msgType);
	head->SetSize(msg->ByteSize());
	return head;
}

void CSocket::SendMsg(UserSession session, uint32_t msgType)
{
	std::shared_ptr<CMsgHead> head(new CMsgHead);
	SharedData data(new string);
	{
		boost::mutex::scoped_lock lk(m_mutex);
		bool empty = session->SendBufEmpty();
		data->resize(head->GetHeadSize());
		memcpy((void*)data->c_str(), head->GetData(), head->GetHeadSize());
		session->AddSendBuf(data);
		if (!empty)
			return;
	}

	boost::asio::async_write(*session, boost::asio::buffer(data->c_str(), data->size()),
		boost::bind(&CSocket::HandleSend, this, session, data, boost::asio::placeholders::error));
}

void CSocket::SendMsg(UserSession pSession,SharedMsg pMsg)
{
	if (pSession.get() == nullptr)
		return;

	std::shared_ptr<CMsgHead> head = GetMsgHead(pMsg);

#ifdef _DEBUG
	std::string typeName = NetMsg::MessageType_Type_Name((NetMsg::MessageType_Type)head->GetType());
	cout << "send msg:" << typeName << endl;
	std::string strMsg;
	google::protobuf::TextFormat::PrintToString(*(pMsg.get()), &strMsg);
	cout << strMsg << endl;
#endif
	
	SharedData data(new string);
	{
		boost::mutex::scoped_lock lk(m_mutex);
		bool empty = pSession->SendBufEmpty();
		data->resize(head->GetHeadSize());
		memcpy((void*)data->c_str(), head->GetData(), head->GetHeadSize());
		pMsg->AppendToString(data.get());
		pSession->AddSendBuf(data);
		if(!empty)
			return;
	}
	
	boost::asio::async_write(*pSession,boost::asio::buffer(data->c_str(),data->size()),
			boost::bind(&CSocket::HandleSend,this,pSession,data,boost::asio::placeholders::error));
}

void CSocket::Run()
{
	m_exit = false;
	INI<> *pIniFile = CGlobalVal::GetData<INI<>>("read_ini");
	
	string printMsg;
	if (pIniFile->select("server"))
	{
		printMsg = pIniFile->get("open_msg");
	}
	
	if(atoi(printMsg.c_str()) == 1)
		m_printMsg = true;
	else
		m_printMsg = false;

	while(!m_exit)
	{
		try
		{
			if (m_service.poll_one() == 0)
				this_thread::yield();
			//this_thread::sleep_for(chrono::milliseconds(50));
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << "\n";
		}
		catch (boost::exception &e)
		{
			cout << "Exception" << endl;
		}
	}
}
