#ifndef _LOGIN_SERVER_H_
#define _LOGIN_SERVER_H_
#include <list>
#include "socket_server.h"
#include "despatch_command.h"
using namespace std;

class CLoginServer
{
public:
    bool Init();
    void MainLoop();
    CLoginServer();
    ~CLoginServer();
private:
    bool InitDbPool();
    bool InitSocket();
    void ThreadFun();
    CSocketServer *m_pLoginSocket;
    CDespatchCommand *m_pDespCmd;
};

#endif