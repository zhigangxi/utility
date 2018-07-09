#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include <boost/function.hpp>

#define _PRINT_LOG

class CServerData;
struct SEventData;
class CNetMessage;
class CSocketServer;

class CSocketServer
{
public:
    CSocketServer();
    virtual ~CSocketServer();
    bool Init();
    bool Bind(const char *ip,const char *port);
    bool DespatchEvent();

    //发送消息
    void SendMsg(int sock, CNetMessage &msg);
    
    //返回的CNetMessage需要调用者调用delete释放
    CNetMessage *GetRecvMsg(int &sock);
    
    void ObserveConnectClose(boost::function<void(int)>);

    //连接远端服务器
    bool Connect(const char *ip,const char *port);

    int GetEventDataSock(SEventData*);

    //bool IsSending(int sockId);

    bool SetKeepAlive(int sockId);

    //关闭连接
    void CloseConnect(int sock);
#ifdef _PRINT_LOG
    int GetConnectUser();
    int GetRecvPag();
    int GetSendPag();
#endif

protected:
    //继承者可以实现此函数，如果返回false，框架会不允许此连接
    virtual bool OnAccept(int,sockaddr_in*);    
    //连接通知
    virtual bool OnConnect(int sock,bool);

    virtual void OnRecv(SEventData*);
    virtual void OnSend(SEventData*);

    bool AddEvent(int,short,void(*)(int,short,void*),CNetMessage *pMsg = NULL);
    bool AddEvent(SEventData*);
    void DelEvent(SEventData*);
    
    static void TimeOut(int,short,void*);

    virtual void TimerFun(){}

private:    
   /* void SetSending(int sockId);
    void CompleteSend(int sockId);*/

    //void SendListMessage();
    
    static void Accept(int, short, void*);
    static void Recv(int, short, void*);
    static void Send(int, short, void*);
    static void Connect(int,short,void*);

    CServerData *m_pData;

    int m_listerSock;
};

#endif