#ifndef _ONLINE_USER_H_
#define _ONLINE_USER_H_
#include "singleton.h"
#include <boost/thread/mutex.hpp>
#include <hash_map>
#include <boost/signal.hpp>
#include <boost/signals/connection.hpp>
#include <list>
#include "game_player.h"
using namespace std;

class CLoginPackDeal;

typedef boost::signals::connection SigConnect;



class COnlineUserList:public singleton<COnlineUserList>
{
public:
    static COnlineUserList *CreateInstance();
    bool IsOnline(int userId);

    CPlayer *GetUser(int userId);
    CPlayer *GetUserBySock(int sock);

    void UpdateUser(int userId,CPlayer &user);
    void AddUser(CPlayer &user);
    void DelUser(int userId);
    void DelUserBySock(int sock);
	void AddCommandDeal();
	void ProcessMsg(CNetMessage& msg,int sock);

    void ForEach(boost::function<void(CPlayer&)>);

    SigConnect ConnectUserLogout(boost::function<void(CPlayer&)>);
    void DesconnectUserLogout(SigConnect);

private:
    COnlineUserList(){}
    ~COnlineUserList(){}
    static boost::mutex    m_mutex;
    static COnlineUserList *m_pOnlineUser;
    hash_map<int,CPlayer*> m_sockHash;
    hash_map<int,CPlayer*> m_userIdHash;
    boost::signal<void(CPlayer&)>   m_signalLogout;
    list<CPlayer*>        m_userInfoList;

    friend CLoginPackDeal;
    void UserLogout(CPlayer&);
};

#endif