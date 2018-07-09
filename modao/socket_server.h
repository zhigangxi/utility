#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <sys/epoll.h>
#include "hash_table.h"
#include "pack_list.h"
#include "self_typedef.h"
#include "protocol.h"
class CNetMessage;
class CSocketServer;
class CLogs;
struct sockaddr_in;

#define MAX_RECV_PACK_LEN  4096

class CSocketServer
{
public:
    CSocketServer();
    virtual ~CSocketServer();
    bool Init(int maxConnect);
    bool Bind(const char *ip,const char *port);
    void DespatchEvent(int timeOut);

    //������Ϣ
    void SendMsg(int sock, CNetMessage &msg,bool clearOld = false);
    
    void ObserveConnectClose(boost::function<void(int)>);

    //����Զ�˷�����
    int Connect(const char *ip,const char *port);

    void SetSock(int sockId);

    //�ر�����
    void CloseConnect(int sock);

    CNetMessage *GetPackage(int &sock)
    {
        return m_packageList.GetMsg(sock);
    }
    
    bool AddEvent(int sock);
    
protected:
    //�̳��߿���ʵ�ִ˺������������false����ܻ᲻���������
    virtual bool OnAccept(int,sockaddr_in*);    
    
    virtual void OnRecv(int sock);
    virtual void OnSend(int sock);
private:    
    int m_listerSock;
    int m_epollFd;
    static const int m_epollNum = MAX_CON_USER;
    epoll_event m_epollEvent[m_epollNum];
#ifdef LOST_2_MSG
    uint16  m_lostRecvData[MAX_CON_USER];
#endif

//#ifdef QQ
    //void DealQQMsg(char *pMsg,int msgLen);
    //bool IsQQServer(uint32 ip);
    //void OnRecvQQ(int sock);
//#endif

    CHashTable<int,CNetMessage*>    m_sendMsgList;
    boost::mutex                  m_sendListMutex;
    CHashTable<int,CNetMessage*>    m_recvMsgList;
    //boost::mutex                  m_recvListMutex;
    CPackageList                    m_packageList;

    CLogs &m_log;
    boost::function<void(int)>      m_onClose;
};

#endif
