#include "stdafx.h"
#include "DatabaseSql.h"
#include "IniFile.h"
#include "login_server.h"
#include "net_msg.h"
#include <boost/thread.hpp>
#include "command_deal.h"
using namespace std;

CLoginServer::CLoginServer()
{
    m_pLoginSocket = new CSocketServer;
    m_pLoginSocket->Init();
    m_pDespCmd = new CDespatchCommand;
}
CLoginServer::~CLoginServer()
{
    delete m_pLoginSocket;
    delete m_pDespCmd;
}

bool CLoginServer::Init()
{
    CLoginDeal *pDeal = new CLoginDeal(m_pLoginSocket);
    pDeal->ReadGameServerList();
    pDeal->AddCommandDeal(m_pDespCmd);
    return (InitDbPool() && InitSocket());
}


bool CLoginServer::InitDbPool()
{
    CDbPool *pPool = CDbPool::CreateInstance();

    string host = CIniFile::GetValue("dbhost","database",CONFIG_FILE_NAME);
    string port = CIniFile::GetValue("dbport","database",CONFIG_FILE_NAME);
    string user = CIniFile::GetValue("dbuser","database",CONFIG_FILE_NAME);
    string password = CIniFile::GetValue("dbpassword","database",CONFIG_FILE_NAME);
    string dbname = CIniFile::GetValue("dbname","database",CONFIG_FILE_NAME);

    pPool->SetDbConfigure(user.c_str(),password.c_str(),host.c_str(),dbname.c_str(),port.c_str());
    if(!pPool->AddDbConnect())
        return false;
    
    return true;
}

bool CLoginServer::InitSocket()
{
    string port = CIniFile::GetValue("port","login_server",CONFIG_FILE_NAME);
    if(m_pLoginSocket->Init())
    {
        return m_pLoginSocket->Bind(NULL,port.c_str());
    }
    return false;
}

void CLoginServer::ThreadFun()
{
    int sock;
    while (1)
    {
        CNetMessage *pMsg = m_pLoginSocket->GetRecvMsg(sock);
        if(pMsg != NULL)
        {
            m_pDespCmd->Despatch(pMsg,sock);
            delete pMsg;
        }
    }
}

void CLoginServer::MainLoop()
{
    boost::thread th(boost::bind(&CLoginServer::ThreadFun,this));

    while(1)
    {
        m_pLoginSocket->DespatchEvent();
    }
}

int main(int argc,char *argv[])
{
    CLoginServer server;
    if(server.Init())
    {
        server.MainLoop();
    }
    return 0;
}