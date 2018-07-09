#include "stdafx.h"
#include "login_pack.h"
#include "despatch_command.h"
#include <boost/bind.hpp>
#include "protocol.h"
#include "DatabaseSql.h"
#include "net_msg.h"
#include "game_socket.h"
#include <boost/lexical_cast.hpp>

struct SRoleInfo
{
    int roleId;
    string name;
    int race;
    int occupation;
    int level;
    int headWear;
    int glass;
    int mask;
    int neckChain;
    int wig;
    int hairColor;
    int armor;
    int shoulderpad;
    int armguard;
    int belt;
    int legguard;
    int shoes;
    int cape;
    int roleplace;
    int head;
    int entered;
    int weapon;
};

CLoginPackDeal::CLoginPackDeal(CGameSocket *pSocket):m_pSocket(pSocket)
{
    m_pOnlineUser = COnlineUserList::CreateInstance();
    m_pSocket->ObserveConnectClose(boost::bind(&CLoginPackDeal::OnSocketClose,this,_1));
}

void CLoginPackDeal::AddCommandDeal(CDespatchCommand *pDespatch)
{
    SCommand comDeal[] = {
        CG_PLAYER_LOGIN,boost::bind(&CLoginPackDeal::PlayerLogin,this,_1,_2),
        CG_SELECT_ROLE,boost::bind(&CLoginPackDeal::SelectRole,this,_1,_2),
        LG_PLAYER_LOGIN_AGAIN,boost::bind(&CLoginPackDeal::PlayerLoginAgain,this,_1,_2),
        LG_PLAYER_IS_LOGIN,boost::bind(&CLoginPackDeal::PlayerIsLogin,this,_1,_2),
		CG_CREATE_ROLE,boost::bind(&CLoginPackDeal::CreateRole,this,_1,_2),
		CG_DELETE_ROLE,boost::bind(&CLoginPackDeal::DeleteRole,this,_1,_2),
    };
    pDespatch->AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));
}

void CLoginPackDeal::GetRoles(int userId,list<SRoleInfo> *pList)
{
    CGetDbConnect getDb,getDb1;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    CDatabaseSql *pDb1 = getDb1.GetDbConnect();
    if(pDb == NULL || pDb1 == NULL)
        return;
    string sqlStr = "select roleid,rolename,race,occupation,level,entered from user_role_infos where userid=" + boost::lexical_cast<std::string,int>(userId);
    char **row;
    if(!pDb->Query(sqlStr.c_str()))
    {
        cout<<sqlStr.c_str()<<endl;
        return;
    }

    while((row = pDb->GetRow()) != NULL)
    {
        SRoleInfo info;
        info.roleId = atoi(row[0]);
        info.name = row[1];
        info.race = atoi(row[2]);
        info.occupation = atoi(row[3]);
        info.level = atoi(row[4]);
        info.entered = atoi(row[5]);
        sqlStr = "select headWear,glass,mask,neckChain,wig,hairColor,armor,shoulderPad,armGuard,belt,legGuard,shoes,cape,roleplace,head,weapon from "\
            "user_role_avatar where roleid=" + boost::lexical_cast<std::string,int>(info.roleId);
        if (!pDb1->Query(sqlStr.c_str()))
        {
            cout<<sqlStr<<endl;
			cout<<pDb1->GetErrMsg()<<endl;
            continue;
        }
        row = pDb1->GetRow();
        if(row == NULL)
            continue;
        info.headWear = atoi(row[0]);
        info.glass = atoi(row[1]);
        info.mask = atoi(row[2]);
        info.neckChain = atoi(row[3]);
        info.wig = atoi(row[4]);
        info.hairColor = atoi(row[5]);
        info.armor = atoi(row[6]);
        info.shoulderpad = atoi(row[7]);
        info.armguard = atoi(row[8]);
        info.belt = atoi(row[9]);
        info.legguard = atoi(row[10]);
        info.shoes = atoi(row[11]);
        info.cape = atoi(row[12]);
        info.roleplace = atoi(row[13]);
        info.head = atoi(row[14]);
        info.weapon = atoi(row[15]);
        pList->push_back(info);
    }
}

void CLoginPackDeal::PlayerIsLogin(CNetMessage &msg,int sock)
{
    int userId = 0;
    int flag;
    msg>>userId>>flag;
    if ((userId == 0) || (flag != SUCCESS_FLAG))
    {
        m_pOnlineUser->DelUser(userId);
        return;
    }
    msg.ReWrite();
    msg.SetType(GC_PLAYER_LOGIN);
    msg<<flag;
    if (flag == SUCCESS_FLAG)
    {
        list<SRoleInfo> roleList;
        GetRoles(userId,&roleList);
        list<SRoleInfo>::iterator i = roleList.begin();
        int num = (int)roleList.size();
        msg<<num;
        for(; i != roleList.end(); i++)
        {
            msg<<i->roleId<<i->name<<i->race<<i->occupation<<i->level<<i->headWear<<i->glass<<i->mask<<i->neckChain
                <<i->wig<<i->hairColor<<i->armor<<i->shoulderpad<<i->armguard<<i->belt<<i->legguard<<i->shoes
                <<i->cape<<i->roleplace<<i->head<<i->entered<<i->weapon;
        }
    }
    CPlayer *pUser = m_pOnlineUser->GetUser(userId);
    if(pUser != NULL)
    {
        m_pSocket->SendMsg(pUser->sock,msg);
    }
}

void CLoginPackDeal::PlayerLogin(CNetMessage &msg,int sock)
{
    int userId = 0;
    msg>>userId;
    msg.ReWrite();
    msg.SetType(GL_PLAYER_IS_LOGIN);
    msg<<userId;
    m_pSocket->SendMsg(m_pSocket->GetLoginSock(),msg);
    CPlayer user;
    user.sock = sock;
    user.userId = userId;
    m_pOnlineUser->AddUser(user);
}

void CLoginPackDeal::SelectRole(CNetMessage &msg,int sock)
{
    int roleId = 0;
	int userId = 0;
	int respond;
	msg>>userId>>roleId;
	msg.ReWrite();
	msg.SetType(GC_SELECT_ROLE);
	//*******************************************
	//判断人物角色是否存在，存在为0，不存在为1
	//*******************************************
	respond = 0;
	//CGetDbConnect getDb;
	msg<<respond;
	//CPlayer user;
	m_pSocket->SendMsg(sock,msg);
}

void CLoginPackDeal::CreateRole(CNetMessage &msg, int sock)
{
	CPlayer *pUser = m_pOnlineUser->GetUserBySock(sock);
	if( pUser == NULL ) 
		return;
	char sql_buf[200];
	memset(sql_buf,0,sizeof sql_buf);
	std::string rolename;
	int race,occup;
	msg>>rolename>>race>>occup;
	const int avatar_slice = 16;//add by me primer 13 
	int data[avatar_slice];
	for (int i = 0;i <avatar_slice;i++ )
	{
		msg>>data[i];
	}
	_snprintf(sql_buf,sizeof sql_buf,"call add_role(%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,@roleid)",
		pUser->userId,rolename.c_str(),race,occup,
		data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],
		data[15]);

	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if(pDb == NULL)
		return;
	std::string sqlStr = sql_buf;
	char **row;
	if(!pDb->Query(sqlStr.c_str()))
	{
		cout<<pDb->GetErrMsg()<<endl;
		//return;
	}
	row = pDb->GetRow();
	int roleid;
	if(row == NULL)
		roleid = 0;
	else
		roleid = atoi(row[0]);
	
	msg.ReWrite();
	msg.SetType(GC_CREATE_ROLE);
	
	if (roleid)
	{
		msg<<(int)0<<roleid;
	}else{
		msg<<(int)1;
	}
	m_pSocket->SendMsg(sock,msg);
}

void CLoginPackDeal::DeleteRole(CNetMessage &msg, int sock)
{
	CPlayer *pUser = m_pOnlineUser->GetUserBySock(sock);
	char sql_buf[200];
	memset(sql_buf,0,sizeof sql_buf);
	std::string rolename,security;
	int roleid;
	msg>>roleid>>security;
	_snprintf(sql_buf,sizeof sql_buf,"call delete_role(%d,%d,'%s',@state)",pUser->userId,roleid,security.c_str());

	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if(pDb == NULL)
		return;
	std::string sqlStr = sql_buf;
	char **row;
	if(!pDb->Query(sqlStr.c_str()))
	{
		cout<<pDb->GetErrMsg()<<endl;
		//return;
	}
	row = pDb->GetRow();
	int state;
	if(row == NULL)
		state = 1;
	else
		state = atoi(row[0]);

	msg.ReWrite();
	msg.SetType(GC_DELETE_ROLE);
	msg<<state;
	
	m_pSocket->SendMsg(sock,msg);
}

void CLoginPackDeal::PlayerLoginAgain(CNetMessage &msg,int sock)
{
    int userId = 0;
    msg>>userId;
    if(userId == 0)
        return;
    msg.ReWrite();
    msg.SetType(GC_PLAYER_LOGIN_AGAIN);
    CPlayer *pUser = m_pOnlineUser->GetUser(userId);
    if(pUser != NULL)
    {
        m_pOnlineUser->DelUser(userId);
        m_pSocket->SendMsg(pUser->sock,msg);
        PlayerLogout(*pUser);
    }
}

void CLoginPackDeal::PlayerLogout(CPlayer &user)
{
    CNetMessage msg;
    msg.SetType(GL_PLAYER_LOGOUT);
    msg<<user.userId;
    m_pSocket->SendMsg(m_pSocket->GetLoginSock(),msg);
    m_pOnlineUser->UserLogout(user);
}

void CLoginPackDeal::OnSocketClose(int sock)
{
    if (sock == m_pSocket->GetLoginSock())
    {
        m_pSocket->ConnectLoginServer();
        return;
    }
    CPlayer *pUser = m_pOnlineUser->GetUserBySock(sock);
    if(pUser != NULL)
    {
        m_pOnlineUser->DelUserBySock(sock);
        PlayerLogout(*pUser);
    }
}