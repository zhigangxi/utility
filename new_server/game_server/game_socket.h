#ifndef _GAME_SOCKET_H_
#define _GAME_SOCKET_H_
#include "socket_server.h"
#include "singleton.h"

class CGameSocket:public CSocketServer,public singleton<CGameSocket>
{
public:
    void ConnectLoginServer();
    int  GetLoginSock()
    {
        return m_loginSerSock;
    }
private:
    void TimerFun()
    {
        if (m_loginSerSock == 0)
        {
            ConnectLoginServer();
        }
    }
    bool OnConnect(int sock,bool flag);
    int m_loginSerSock;
};
#endif
