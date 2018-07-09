#ifndef _CONNECT_SOCKET_H_
#define _CONNECT_SOCKET_H_
#include "socket_server.h"
#include "net_msg.h"
#include <list>
using namespace std;
class CConnect:public CSocketServer
{
public:
    void OnRecv(SEventData *pData);
    bool OnConnect(int sock,bool flag);
    list<int> *GetConnectList();
private:
    list<int> m_connectList;
};

#endif