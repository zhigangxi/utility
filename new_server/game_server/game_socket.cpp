#include "stdafx.h"
#include "game_socket.h"
#include "IniFile.h"
#include <string>
using namespace std;

void CGameSocket::ConnectLoginServer()
{
    m_loginSerSock = -1;
    string ip = CIniFile::GetValue("ip","login_server",CONFIG_FILE_NAME);
    string port = CIniFile::GetValue("port","login_server",CONFIG_FILE_NAME);
    Connect(ip.c_str(),port.c_str());
}

bool CGameSocket::OnConnect(int sock,bool flag)
{
    m_loginSerSock = 0;
    if(!flag)
    {
        cout<<"connect login server error"<<endl;
        return false;
    }
    m_loginSerSock = sock;
    if(CSocketServer::OnConnect(sock,flag))
    {
        return true;
    }
    cout<<"connect login server error"<<endl;
    return false;
}