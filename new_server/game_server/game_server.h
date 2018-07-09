#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_
#include "game_socket.h"
#include "despatch_command.h"
#include "../common/include/singleton.h"

class CGameServer:public singleton<CGameServer>
{
public:
    bool Init();
    void MainLoop();
    CGameServer();
    ~CGameServer();
private:
    bool InitDbPool();
    bool InitSocket();
    void DealPackThread();
    CGameSocket *m_pGameSocket;
    CDespatchCommand *m_pDespCmd;
    int m_threadNum;
};

#endif