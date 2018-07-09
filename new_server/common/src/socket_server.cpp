#include "stdafx.h"
#include "event.h"
#include "socket_server.h"
#include "net_msg.h"
#include <stack>
#include <utility>
#include <list>
#include <hash_map>
#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition.hpp>
#include <boost/signal.hpp>
#include <MSTCPiP.h>
typedef pair<int,CNetMessage*> MsgNode;
typedef boost::signal<void(int)> CloseSignal;

class CServerData
{
private:
    stack<SEventData*>              m_eventStack;
    static const int                m_oneInsertNum = 1024;
public:
    list<MsgNode>                   m_sendMsgList;
    list<MsgNode>                   m_recvMsgList;
    hash_map<int,CNetMessage*>      m_sendingList;
    boost::mutex                    m_sendListMutex;
    boost::mutex                    m_recvListMutex;

    HANDLE                          m_semaphone;

    CServerData():m_pEventBase(NULL)
    {
        m_semaphone = CreateSemaphore(NULL,0,0xffff,NULL);
#ifdef _PRINT_LOG
        m_recvPag = 0;
        m_sendPag = 0;
        m_eventSize = 0;
#endif
    }
    ~CServerData()
    {
        CloseHandle(m_semaphone);
    }

    void SemPost()
    {
        ReleaseSemaphore(m_semaphone,1,NULL);
    }
    
    void SemWait()
    {
        WaitForSingleObject(m_semaphone,INFINITE);
    }

    /*SEventData      *PopEventData();
    void            PushEventData(SEventData *pData);*/

    CloseSignal         m_closeSignal;
    event_base          *m_pEventBase;
    event               m_timeOutEvent;
#ifdef _PRINT_LOG
    int m_recvPag;
    int m_sendPag;
    int m_eventSize;
    int GetEventSize()
    {
        return m_eventSize;
    }
#endif
};

struct SEventData
{
    event           m_event;
    CNetMessage     *m_pMessage;
    CSocketServer   *m_pSockServer;
    SEventData():m_pSockServer(NULL)
    {
    }
    void *operator new(size_t);
    void operator delete(void *p);
};
typedef boost::singleton_pool<SEventData,sizeof(SEventData)> SEventDataPool;
void *SEventData::operator new(size_t)
{
    return SEventDataPool::malloc();
}
void SEventData::operator delete(void *p)
{
    SEventDataPool::free(p);
}

//SEventData *CServerData::PopEventData()
//{
//#ifdef _PRINT_LOG
//    m_eventSize++;
//#endif
//    if(m_eventStack.empty())
//    {
//        SEventData *pEventData = new SEventData[m_oneInsertNum];
//        for(int i = 1; i < m_oneInsertNum; i++)
//        {
//            m_eventStack.push(pEventData+i);
//        }
//        return pEventData;
//    }
//    SEventData *p = m_eventStack.top();
//    m_eventStack.pop();
//    return p;
//}
//
//void CServerData::PushEventData(SEventData *pData)
//{
//#ifdef _PRINT_LOG
//    m_eventSize--;
//#endif
//    m_eventStack.push(pData);
//}

CSocketServer::CSocketServer()
{
    m_pData = new CServerData;
}

CSocketServer::~CSocketServer()
{
    if(m_pData->m_pEventBase != NULL)
        event_base_free(m_pData->m_pEventBase);
    delete m_pData;
}

bool CSocketServer::Init()
{
    WSADATA wsaData;

    int err = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( err != 0 ) {
        cout<<"WSAStartup error"<<endl;
        return false;
    }
    if(m_pData->m_pEventBase != NULL)
        return true;
    m_pData->m_pEventBase = event_base_new();
    
    if(m_pData->m_pEventBase == NULL)
        return false;
    
    timeval tm = {0,200};
    event_set(&(m_pData->m_timeOutEvent), -1, 0, TimeOut, this);
    event_base_set((m_pData->m_pEventBase),&(m_pData->m_timeOutEvent));
    event_add(&(m_pData->m_timeOutEvent),&tm);

    return true;
}

void CSocketServer::TimeOut(int,short,void *p)
{
    CSocketServer *pThis = (CSocketServer*)p;
    timeval tm = {0,200};
    event_set(&(pThis->m_pData->m_timeOutEvent), -1, 0, TimeOut, p);
    event_base_set(pThis->m_pData->m_pEventBase,&(pThis->m_pData->m_timeOutEvent));
    event_add(&(pThis->m_pData->m_timeOutEvent),&tm);
    pThis->TimerFun();
}

bool CSocketServer::Bind(const char *ip,const char *port)
{
    int listenFd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        cout<<"create listen socket error"<<endl;
        return false;
    }
    int reuseaddr_on = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr_on,sizeof(reuseaddr_on)) == -1)
    {
        cout<<"setsocketopt error"<<endl;
        return false;
    }
        
    sockaddr_in listenAddr;
    int len = sizeof(listenAddr);
    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    if(ip == NULL)
        listenAddr.sin_addr.s_addr = INADDR_ANY;
    else
        listenAddr.sin_addr.s_addr = inet_addr(ip);

    listenAddr.sin_port = htons(atoi(port));
    if (bind(listenFd, (struct sockaddr *)&listenAddr,sizeof(listenAddr)) < 0)
    {
        cout<<"bind error ip:"<<ip<<" port:"<<port<<endl;
        return false;
    }
    
    if (listen(listenFd, 1024) < 0)
    {
        cout<<"listen error"<<endl;
        return false;
    }

    evutil_make_socket_nonblocking(listenFd);
    m_listerSock = listenFd;
    if(!AddEvent(listenFd,EV_READ|EV_PERSIST, CSocketServer::Accept))//|EV_PERSIST
    {
        cout<<"add accept function error!"<<endl;
        return false;
    }
    return true;
}

bool CSocketServer::SetKeepAlive(int sockId)
{
    DWORD dwBytes = 0;
    tcp_keepalive sKA_Settings = {0}, sReturned = {0} ;
    sKA_Settings.onoff = 1 ;
    sKA_Settings.keepalivetime = 5500 ; // Keep Alive in 5.5 sec.
    sKA_Settings.keepaliveinterval = 3000 ; // Resend if No-Reply
    return (WSAIoctl(sockId, SIO_KEEPALIVE_VALS, &sKA_Settings,
        sizeof(sKA_Settings), &sReturned, sizeof(sReturned), &dwBytes,
        NULL, NULL) == 0);
}

void CSocketServer::Accept(int sockId, short, void *p)
{
    if(p == NULL)
        return;
    SEventData *pData = (SEventData*)p;
    sockaddr_in clientAddr;

    int len = sizeof(sockaddr_in);
    int clientFd = (int)accept(SOCKET(sockId), (sockaddr *)&clientAddr, &len);
    if(clientFd < 0)
        return;
    
    evutil_make_socket_nonblocking(clientFd);
    if(!pData->m_pSockServer->SetKeepAlive(clientFd))
    {
        cout<<"set keep alive error"<<endl;
    }

    if(!pData->m_pSockServer->OnAccept(clientFd,&clientAddr))
    {
        //不允许此ip地址连接
        closesocket(clientFd);
        return;
    }

    pData->m_pSockServer->AddEvent(clientFd,EV_READ,CSocketServer::Recv);
    //pThis->AddSendFun(clientFd,Send);
}

void CSocketServer::Recv(int sockId, short, void *p)
{
    if(p == NULL)
        return;
    SEventData *pData = (SEventData*)p;
    pData->m_pSockServer->OnRecv(pData);
}

void CSocketServer::Send(int sockId, short, void *p)
{
    if(p == NULL)
        return;
    SEventData *pData = (SEventData*)p;
    pData->m_pSockServer->OnSend(pData);
}

void CSocketServer::OnRecv(SEventData *pData)
{
    CNetMessage *pMsg = pData->m_pMessage;
    if(pMsg == NULL)
    {
        pMsg = new CNetMessage;
        pData->m_pMessage = pMsg;       
    }    
    int recvLen = pMsg->RecvMsg(pData->m_event.ev_fd);
    if(recvLen <= 0)
    {
        DelEvent(pData);
        CloseConnect(pData->m_event.ev_fd);
        return;
    }
    if(pMsg->RecvComplete())
    {
        MsgNode msg(pData->m_event.ev_fd,pData->m_pMessage);
        boost::mutex::scoped_lock lk(m_pData->m_recvListMutex);
        m_pData->m_recvMsgList.push_back(msg);
        m_pData->SemPost();
        pData->m_pMessage = NULL;
    }
    AddEvent(pData);
}

CNetMessage *CSocketServer::GetRecvMsg(int &sock)
{
    CNetMessage *pMsg = NULL;
    m_pData->SemWait();
    boost::mutex::scoped_lock lk(m_pData->m_recvListMutex);
    if(m_pData->m_recvMsgList.size() > 0)
    {
        MsgNode node = *(m_pData->m_recvMsgList.begin());
        pMsg = node.second;
        sock = node.first;
        m_pData->m_recvMsgList.pop_front();
    }
    return pMsg;
}

void CSocketServer::OnSend(SEventData *pData)
{
    boost::mutex::scoped_lock lk(m_pData->m_sendListMutex);
    CNetMessage *pMsg = pData->m_pMessage;
    if(pMsg == NULL)
        return;
    int len = pMsg->SendMsg(pData->m_event.ev_fd);
    if((len == 0) || pMsg->SendComplete())
    {
        m_pData->m_sendingList.erase(pData->m_event.ev_fd);
        DelEvent(pData);
    }
    else
    {
        if(!AddEvent(pData))
        {
            m_pData->m_sendingList.erase(pData->m_event.ev_fd);
        }
    }
}

void CSocketServer::SendMsg(int sockId,CNetMessage &msg)
{
    boost::mutex::scoped_lock lk(m_pData->m_sendListMutex);
    
    hash_map<int,CNetMessage*>::iterator iter = m_pData->m_sendingList.find(sockId);
    if(iter != m_pData->m_sendingList.end())
    {
        PoolString *pStr = iter->second->GetMsgData();
        PoolString *pMsg = msg.GetMsgData();
        pStr->append(*pMsg);
    }
    else
    {
		int len = msg.SendMsg(sockId);
		if((len == 0) || msg.SendComplete())
		{
			return;
		}
		CNetMessage *pMsg = new CNetMessage;
        pMsg->CopyData(*msg.GetMsgData());
        if(!AddEvent(sockId,EV_WRITE,CSocketServer::Send,pMsg))
        {
            delete pMsg;
        }
        else
        {
            m_pData->m_sendingList[sockId] = pMsg;
        }
    }
}


void CSocketServer::DelEvent(SEventData *pData)
{
    if(pData->m_pMessage != NULL)
    {
        delete pData->m_pMessage;
        pData->m_pMessage = NULL;
    }
    delete pData;
}

void CSocketServer::CloseConnect(int sockId)
{
    m_pData->m_closeSignal(sockId);
    closesocket(sockId);    
}

bool CSocketServer::AddEvent(int sockId,int short flag,void(*fun)(int,short,void*),CNetMessage *pMsg)
{
    SEventData *pData = new SEventData;
    pData->m_pMessage = pMsg;
    pData->m_pSockServer = this;
    event_set(&(pData->m_event), sockId, flag, fun, pData);
    event_base_set(m_pData->m_pEventBase,&(pData->m_event));

    if(event_add(&(pData->m_event),NULL) != 0)
    {
        DelEvent(pData);
        CloseConnect(sockId);
        return false;
    }
    return true;
}

bool CSocketServer::AddEvent(SEventData *pData)
{
    if(event_add(&(pData->m_event),NULL) != 0)
    {
        DelEvent(pData);
        CloseConnect(pData->m_event.ev_fd);
        return false;
    }
    return true;
}

bool CSocketServer::DespatchEvent()
{
    return (event_base_loop(m_pData->m_pEventBase,EVLOOP_ONCE) == 0);
}

bool CSocketServer::OnAccept(int,sockaddr_in*)
{
    return true;
}

void CSocketServer::ObserveConnectClose(boost::function<void(int)> fun)
{
    m_pData->m_closeSignal.connect(fun);
}

void CSocketServer::Connect(int sockId,short mask,void *p)
{
    if(p == NULL)
        return;
    SEventData *pData = (SEventData*)p;
    pData->m_pSockServer->DelEvent(pData);
    pData->m_pSockServer->OnConnect(sockId, mask != 0);
}

bool CSocketServer::Connect(const char *ip,const char *port)
{
    int sockId = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sockId < 0)
    {
        cout<<"create connect socket error"<<endl;
        return false;
    }
    
    sockaddr_in addr;
    int len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(ip == NULL)
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = inet_addr(ip);

    addr.sin_port = htons(atoi(port));
    
    evutil_make_socket_nonblocking(sockId);

    if(connect(sockId,(sockaddr*)&addr,len) == 0)
    {
        OnConnect(sockId,true);
        return true;
    }
    if(WSAEWOULDBLOCK == WSAGetLastError())
    {
        if(!AddEvent(sockId,EV_CONNECT|EV_WRITE, CSocketServer::Connect))
        {
            cout<<"add connect function error!"<<endl;
            return false;
        }
    }
    else
    {
        closesocket(sockId);
        return false;
    }
    DespatchEvent();
    return true;
}

bool CSocketServer::OnConnect(int sock,bool flag)
{
    if(!flag)
        return false;
    return AddEvent(sock,EV_READ,Recv);
}

int CSocketServer::GetEventDataSock(SEventData *pData)
{
    return pData->m_event.ev_fd;
}

#ifdef _PRINT_LOG
int CSocketServer::GetConnectUser()
{
    return m_pData->GetEventSize();
}
int CSocketServer::GetSendPag()
{
    return m_pData->m_sendPag;
}
int CSocketServer::GetRecvPag()
{
    return m_pData->m_recvPag;
}
#endif