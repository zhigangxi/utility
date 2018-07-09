#ifndef _COMMAND_DEAL_H_
#define _COMMAND_DEAL_H_
#include "despatch_command.h"
#include "net_msg.h"
#include "socket_server.h"
#include <hash_map>
#include <list>
using namespace std;

struct SOnlineUser 
{
    int userId;
    int gameServerSock;
};
struct SGameServerNode
{
    string ip;
    string port;
    string group;
    string name;
};

class COnlineUserList
{
public:
    SOnlineUser *GetUser(int userId);
    void AddUser(SOnlineUser&);
    void DelUser(int userId);
private:
    hash_map<int,SOnlineUser> m_onlineUserList;
};

class CLoginDeal
{
public:
    CLoginDeal(CSocketServer *pSocket):m_pSocket(pSocket)
    {
    }
    void AddCommandDeal(CDespatchCommand*);
    bool ReadGameServerList();
private:
    void PlayerLogin(CNetMessage&,int);
    void PlayerIsLogin(CNetMessage&,int);
    void PlayerLogout(CNetMessage&,int);
    void SendOtherPlayerLogin(SOnlineUser&);
    COnlineUserList m_onlineUserList;
    CSocketServer *m_pSocket;
    list<SGameServerNode>  m_serverList;
};

#endif