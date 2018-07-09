#include "stdafx.h"
#include "comnect_socket.h"
#include "net_msg.h"
#include <iostream>
using namespace std;

void CConnect::OnRecv(SEventData *pData)
{
    int len = 0;
    char buf[1024] = {0};
    //string str;
    len = recv(GetEventDataSock(pData),buf,1024,0);
    if(len == 0)
    {
        DelEvent(pData);
        CloseConnect(GetEventDataSock(pData));
    }
    else
    {
        AddEvent(pData);
    }
}

bool CConnect::OnConnect(int sock,bool flag)
{
    if(CSocketServer::OnConnect(sock,flag))
    {
        m_connectList.push_back(sock);
        return true;
    }
    return false;
}   

list<int> *CConnect::GetConnectList()
{
    return &m_connectList;
}