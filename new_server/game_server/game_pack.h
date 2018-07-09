//#ifndef _GAME_PACKAGE_DEAL_H_
//#define _GAME_PACKAGE_DEAL_H_
//#include <list>
//#include "online_user.h"
//using namespace std;
//class CGameSocket;
//class CDespatchCommand;
//class CNetMessage;
//struct SRoleInfo;
//
//class CGamePackDeal
//{
//public:
//	CGamePackDeal(CGameSocket *pSocket):m_pSocket(pSocket)
//	{
//		m_pOnlineUser = COnlineUserList::CreateInstance();
//	}
//	void AddCommandDeal(CDespatchCommand*);
//private:
//	void PlayerEnterGame(CNetMessage &msg,int sock);
//	void PlayerMove(CNetMessage &msg,int sock);
//	void PlayerJumpMap(CNetMessage &msg,int sock);
//	CGameSocket *m_pSocket;
//	COnlineUserList *m_pOnlineUser;
//};
//
//#endif