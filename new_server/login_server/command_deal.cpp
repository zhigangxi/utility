#include "stdafx.h"
#include "protocol.h"
#include "command_deal.h"
#include "DatabaseSql.h"
#include "IniFile.h"
#include <boost/bind.hpp>
#include <string>
using namespace std;

SOnlineUser *COnlineUserList::GetUser(int userId)
{
    hash_map<int,SOnlineUser>::iterator i = m_onlineUserList.find(userId);
    if (i == m_onlineUserList.end())
    {
        return NULL;
    }
    return &(i->second);
}

void COnlineUserList::AddUser(SOnlineUser &user)
{
    m_onlineUserList[user.userId] = user;
}

void COnlineUserList::DelUser(int userId)
{
    hash_map<int,SOnlineUser>::iterator i = m_onlineUserList.find(userId);
    if(i != m_onlineUserList.end())
        m_onlineUserList.erase(i);
}

bool CLoginDeal::ReadGameServerList()
{
    string str = CIniFile::GetValue("server_num","game_server",CONFIG_FILE_NAME);
    for (int i = 0; i < atoi(str.c_str()); i++)
    {
        char buf[32];
        SGameServerNode node;
        sprintf(buf,"group%d",i+1);
        node.group = CIniFile::GetValue(buf,"game_server",CONFIG_FILE_NAME);
        sprintf(buf,"name%d",i+1);
        node.name = CIniFile::GetValue(buf,"game_server",CONFIG_FILE_NAME);
        sprintf(buf,"ip%d",i+1);
        node.ip = CIniFile::GetValue(buf,"game_server",CONFIG_FILE_NAME);
        sprintf(buf,"port%d",i+1);
        node.port = CIniFile::GetValue(buf,"game_server",CONFIG_FILE_NAME);
        m_serverList.push_back(node);
    }
    return true;
}

void CLoginDeal::PlayerLogin(CNetMessage &msg,int sock)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if (pDb == NULL)
    {
        return;
    }
    string name;
    string password;
    msg>>name>>password;
    string sqlStr = "select userid,username,password from user_base_info where username=\'" + name + "\'";
    if(!pDb->Query(sqlStr.c_str()))
	{
		cout<<sqlStr<<endl;
        return;
	}
    char **row = pDb->GetRow();
    msg.ReWrite();
    msg.SetType(LC_GAME_SERVER_LIST);
        
    if((row != NULL) && (name == row[1]) && (password == row[2]))
    {
        SOnlineUser user;
        user.userId = atoi(row[0]);
        user.gameServerSock = 0;
        SOnlineUser *pUser = m_onlineUserList.GetUser(user.userId);
        if (pUser != NULL)
        {
            SendOtherPlayerLogin(*pUser);
        }
        m_onlineUserList.AddUser(user);
        int serverNum = (int)m_serverList.size();
        msg<<SUCCESS_FLAG<<user.userId<<serverNum;
        list<SGameServerNode>::iterator i = m_serverList.begin();
        for (; i != m_serverList.end(); i++)
        {
            msg<<i->group<<i->name<<i->ip<<i->port;
        }
    }
    else
    {
        msg<<ERROR_FLAG;
    }
    m_pSocket->SendMsg(sock,msg);
}

void CLoginDeal::PlayerIsLogin(CNetMessage &msg,int sock)
{
    int userId;
    msg>>userId;
    int flag;
    SOnlineUser *pUser = m_onlineUserList.GetUser(userId);
    if (pUser != NULL)
    {
        pUser->gameServerSock = sock;
        flag = 0;
    }
    else
    {
        flag = 1;
    }
    msg.ReWrite();
    msg.SetType(LG_PLAYER_IS_LOGIN);
    msg<<userId;
    msg<<flag;
    m_pSocket->SendMsg(sock,msg);
}

void CLoginDeal::PlayerLogout(CNetMessage &msg,int sock)
{
    int userId;
    msg>>userId;
    SOnlineUser *pUser = m_onlineUserList.GetUser(userId);
    if((pUser != NULL) && (pUser->gameServerSock != 0))
        m_onlineUserList.DelUser(userId);
}

void CLoginDeal::AddCommandDeal(CDespatchCommand *pDespatch)
{
    SCommand comDeal[] = {
        CL_GAME_SERVER_LIST,boost::bind(&CLoginDeal::PlayerLogin,this,_1,_2),
        GL_PLAYER_IS_LOGIN,boost::bind(&CLoginDeal::PlayerIsLogin,this,_1,_2),
        GL_PLAYER_LOGOUT,boost::bind(&CLoginDeal::PlayerLogout,this,_1,_2)
    };
    pDespatch->AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));
}

void CLoginDeal::SendOtherPlayerLogin(SOnlineUser &user)
{
    if(user.gameServerSock != 0)
    {
        CNetMessage msg;
        msg.SetType(LG_PLAYER_LOGIN_AGAIN);
        msg<<user.userId;
        m_pSocket->SendMsg(user.gameServerSock,msg);
    }
    m_onlineUserList.DelUser(user.userId);
}