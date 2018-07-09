#ifndef _LOGIN_PACKAGE_DEAL_H_
#define _LOGIN_PACKAGE_DEAL_H_
#include <list>
#include "online_user.h"
using namespace std;
class CGameSocket;
class CDespatchCommand;
class CNetMessage;
struct SRoleInfo;

class CLoginPackDeal
{
public:
    CLoginPackDeal(CGameSocket *pSocket);
    void AddCommandDeal(CDespatchCommand*);
private:
    void PlayerLogin(CNetMessage &msg,int sock);
    void SelectRole(CNetMessage &msg,int sock);
    void PlayerIsLogin(CNetMessage &msg,int sock);
    void PlayerLoginAgain(CNetMessage &msg,int sock);
	void CreateRole(CNetMessage &msg,int sock);
	void DeleteRole(CNetMessage &msg,int sock);

    void OnSocketClose(int sock);
    void PlayerLogout(CPlayer&);

    void GetRoles(int userId,list<SRoleInfo> *pList);

    CGameSocket *m_pSocket;
    COnlineUserList *m_pOnlineUser;
};

#endif