#include "stdafx.h"
#include "server_data_base.h"
#include "IniFile.h"
#include "net_msg.h"
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include "DatabaseSql.h"
#include "game_server.h"
#include "login_pack.h"
#include "game_pack.h" 
#include "game_npc.h"
#include "scene_manager.h"
#include "global_manager.h"

using namespace std;
CGameServer::CGameServer()
{
    m_pGameSocket = new CGameSocket;
    CGlobalValManager::CreateInstance()->GetVal(&m_pDespCmd,EGV_CDESPATCH_COMMAND);
    m_pGameSocket->Init();
	new server_data();
	new scene_manager();
    m_threadNum = 0;
}
CGameServer::~CGameServer()
{
    delete m_pGameSocket;
}

bool CGameServer::Init()
{
    CLoginPackDeal *pDeal = new CLoginPackDeal(m_pGameSocket);
	pDeal->AddCommandDeal(m_pDespCmd);
	server_data::instance().Start();
	scene_manager::instance().AddCommandDeal();

    return (InitDbPool() && InitSocket());
}

bool CGameServer::InitDbPool()
{
    CDbPool *pPool = CDbPool::CreateInstance();
    
    string host = CIniFile::GetValue("dbhost","database",CONFIG_FILE_NAME);
    string port = CIniFile::GetValue("dbport","database",CONFIG_FILE_NAME);
    string user = CIniFile::GetValue("dbuser","database",CONFIG_FILE_NAME);
    string password = CIniFile::GetValue("dbpassword","database",CONFIG_FILE_NAME);
    string dbname = CIniFile::GetValue("dbname","database",CONFIG_FILE_NAME);

    pPool->SetDbConfigure(user.c_str(),password.c_str(),host.c_str(),dbname.c_str(),port.c_str());
    string num = CIniFile::GetValue("threadnum","game_server",CONFIG_FILE_NAME);
    m_threadNum = atoi(num.c_str());
    for(int i = 0; i < m_threadNum; i++)
    {
        if(!pPool->AddDbConnect())
            return false;
    }
    CNpcManager *pNpc = new CNpcManager;
    if (!pNpc->Init(m_pGameSocket))
    {
        cout<<"npc init error"<<endl;
        return false;
    }
    CGlobalValManager::CreateInstance()->AddVal(pNpc,EGV_NPC_MANAGER);
    return true;
}

bool CGameServer::InitSocket()
{
    string port = CIniFile::GetValue("port","game_server",CONFIG_FILE_NAME);
    if(m_pGameSocket->Init())
    {
        m_pGameSocket->ConnectLoginServer();
        return m_pGameSocket->Bind(NULL,port.c_str());
    }
    return false;
}

void CGameServer::DealPackThread()
{
    int sock;
    while (1)
    {
        CNetMessage *pMsg = m_pGameSocket->GetRecvMsg(sock);
        if(pMsg != NULL)
        {
            m_pDespCmd->Despatch(pMsg,sock);
            delete pMsg;
        }
    }
}

void TimerFun()
{
    while (1)
    {
        scene_manager::instance().OnLoop(GetTickCount());
        Sleep(100);
    }
};

void CGameServer::MainLoop()
{
    for(int i = 0; i < m_threadNum; i++)
    {
        new boost::thread(boost::bind(&CGameServer::DealPackThread,this));
    }
    boost::thread timerThread(TimerFun);

    while(1)
    {
        m_pGameSocket->DespatchEvent();
    }
}

int main(int argc,char *argv[])
{
    CGlobalValManager *pManager = CGlobalValManager::CreateInstance();
    pManager->AddVal(COnlineUserList::CreateInstance(),EGV_ONLINE_USER_LIST);
    pManager->AddVal(new CDespatchCommand,EGV_CDESPATCH_COMMAND);
    
    CGameServer server;
	COnlineUserList::instance().AddCommandDeal();
    if(server.Init())
    {
        server.MainLoop();
    }
    
    return 0;
}   