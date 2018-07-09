#ifndef _SESSION_H_
#define _SESSION_H_
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <boost/any.hpp>
#include <google/protobuf/message.h>

typedef std::shared_ptr<google::protobuf::Message> SharedMsg;
typedef std::shared_ptr<std::string> SharedData;

class CSession:public boost::asio::ip::tcp::socket
{
public:
	CSession(boost::asio::io_service &service):boost::asio::ip::tcp::socket(service)
	{
	}
	~CSession()
	{
	}
	template<typename Type> Type *GetData()
	{
		if(m_data.empty())
			return NULL;
		return boost::any_cast<Type*>(m_data);
	}
	template<typename Type> void SetData(Type *data)
	{
		m_data = data;
	}
	void AddSendBuf(SharedData msg)
	{
		m_sendBuf.push_back(msg);
	}
	void DelSendBuf(SharedData msg)
	{
		for(std::list<SharedData>::iterator i = m_sendBuf.begin(); i != m_sendBuf.end(); i++)
		{
			if(i->get() == msg.get())
			{
				m_sendBuf.erase(i);
				break;
			}
		}
	}
	bool GetSendBuf(SharedData &msg)
	{
		if(m_sendBuf.empty())
			return false;
		msg = *(m_sendBuf.begin());
		return true;
	}
	bool SendBufEmpty()
	{
		return m_sendBuf.empty();
	}
private:
	boost::any m_data;
	std::list<SharedData> m_sendBuf;
};

typedef std::shared_ptr<CSession> UserSession;

#endif
