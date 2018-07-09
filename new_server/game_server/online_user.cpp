#include "stdafx.h"
#include "online_user.h"
#include "despatch_command.h"
#include "protocol.h"
#include <boost/bind.hpp>



COnlineUserList *COnlineUserList::m_pOnlineUser;
boost::mutex COnlineUserList::m_mutex;

COnlineUserList *COnlineUserList::CreateInstance()
{
    boost::mutex::scoped_lock lk(m_mutex);
    if(m_pOnlineUser == NULL)
        m_pOnlineUser = new COnlineUserList;
    return m_pOnlineUser;
}

bool COnlineUserList::IsOnline(int userId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter i = m_userIdHash.find(userId);
    if(i == m_userIdHash.end())
        return false;
    return true;
}

CPlayer *COnlineUserList::GetUser(int userId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter i = m_userIdHash.find(userId);
    if(i != m_userIdHash.end())
        return i->second;
    else
        return NULL;
}

CPlayer *COnlineUserList::GetUserBySock(int sock)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter i = m_sockHash.find(sock);
    if(i != m_sockHash.end())
        return i->second;
    else
        return NULL;
}

void COnlineUserList::AddUser(CPlayer &user)
{
    boost::mutex::scoped_lock lk(m_mutex);
    CPlayer *pUser;
    if(m_userInfoList.size() > 0)
    {
        pUser = *m_userInfoList.begin();
        m_userInfoList.pop_front();
    }
    else
    {
        pUser = new CPlayer;
    }
    *pUser = user;
    m_sockHash[user.sock] = pUser;
    m_userIdHash[user.userId] = pUser;
//	pUser->AddCommandDeal();
}

void COnlineUserList::DelUser(int userId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter itUserId = m_userIdHash.find(userId);
    if(itUserId == m_userIdHash.end())
        return;
    CPlayer *pUser = itUserId->second;
    m_userIdHash.erase(itUserId);

    HashIter itSock = m_sockHash.find(pUser->sock);
    if(itSock != m_sockHash.end())
        m_sockHash.erase(itSock);
    m_userInfoList.push_back(pUser);
}

void COnlineUserList::DelUserBySock(int sock)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter itUserId = m_sockHash.find(sock);
    if(itUserId == m_sockHash.end())
        return;
    CPlayer *pUser = itUserId->second;
    m_sockHash.erase(itUserId);

    HashIter itSock = m_userIdHash.find(pUser->userId);
    if(itSock != m_userIdHash.end())
        m_userIdHash.erase(itSock);
    m_userInfoList.push_back(pUser);
}

SigConnect COnlineUserList::ConnectUserLogout(boost::function<void(CPlayer&)> fun)
{
    boost::mutex::scoped_lock lk(m_mutex);
    return m_signalLogout.connect(fun);
}

void COnlineUserList::DesconnectUserLogout(SigConnect con)
{
    boost::mutex::scoped_lock lk(m_mutex);
    con.disconnect();
}
void COnlineUserList::UserLogout(CPlayer &user)
{
    m_signalLogout(user);
}

void COnlineUserList::ForEach(boost::function<void(CPlayer&)> fun)
{
    boost::mutex::scoped_lock lk(m_mutex);
    HashIter iter = m_userIdHash.begin();
    for (; iter != m_userIdHash.end(); iter++)
    {
        fun(*(iter->second));
    }
}

void COnlineUserList::AddCommandDeal()
{
    COnlineUserList::instance().ConnectUserLogout(boost::bind(&CPlayer::PlayerQuitGame,_1));
	SCommand comDeal[] = {
		CG_ENTER_GAME,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_CHAT_INFO,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_ROLE_INFO,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_ADD_ATTRIPOINT,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_USE_DELETE_ITEM,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_USE_ITEM,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_EXCHANGE_ITEM_PLACE,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_SET_EXPEND_SHORTCUT,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2),
		CG_PACK_TRIM,boost::bind(&COnlineUserList::ProcessMsg,this,_1,_2)
		
	};
	CDespatchCommand::instance().AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));
}

void COnlineUserList::ProcessMsg(CNetMessage& msg,int sock)
{
	int type = msg.GetType();
	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	if (player!=NULL)
	{
		//if (player->roleId>0)
		//{
			player->ProcessMsg(msg);
		//}
	}
}