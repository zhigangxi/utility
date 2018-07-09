#include "net_msg.h"
#include <stack>
#include <boost/pool/singleton_pool.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <sys/types.h>
#include <sys/socket.h>
using namespace boost;

typedef boost::singleton_pool<CNetMessage,sizeof(CNetMessage)> MsgPool;

void *CNetMessage::operator new(size_t)
{
    return MsgPool::malloc();
}

void CNetMessage::operator delete(void *p)
{
    MsgPool::free(p);
}

CNetMessage::CNetMessage():m_readPos(m_msgHeadLen),m_recvLen(0),m_sendBegin(0)
{
    m_msgData.resize(m_msgHeadLen);
    SetDataLen();
}

void CNetMessage::ReRead()
{
    m_readPos = m_msgHeadLen;
}

void CNetMessage::ReWrite()
{
    m_sendBegin = 0;
    m_msgData.clear();
    m_msgData.resize(m_msgHeadLen);
}

uint16 CNetMessage::GetDataLen()
{
    return *(uint16*)(m_msgData.c_str()) + m_msgHeadLen;
}

void CNetMessage::SetDataLen()
{
    *(uint16*)(m_msgData.c_str()) = (uint16)m_msgData.length() - m_msgHeadLen;
}

void CNetMessage::SetType(uint16 type)
{
    *(uint16*)(m_msgData.c_str() + m_msgTypeBegin) = type;
}

uint16 CNetMessage::GetType()
{
    return *(uint16*)(m_msgData.c_str() + m_msgTypeBegin);
}

PoolString *CNetMessage::GetMsgData()
{
    return &m_msgData;
}

void CNetMessage::Append(CNetMessage &val)
{
    if(m_msgData.length() > MAX_SEND_PACK_LEN)
        return;
    //cout<<"append"<<endl;
    //m_msgData.append(val.m_msgData,val.m_sendBegin,val.m_msgData.length()-val.m_sendBegin);
    char *data = (char*)(val.m_msgData.c_str()+val.m_sendBegin);
    uint16 dataLen = val.m_msgData.length() - val.m_sendBegin;
    m_msgData.append(data,dataLen);
}

void CNetMessage::CopyData(CNetMessage &val)
{
    m_msgData.clear();
    //m_msgData.append(val.m_msgData,val.m_sendBegin,val.m_msgData.length()-val.m_sendBegin);
    char *data = (char*)(val.m_msgData.c_str()+val.m_sendBegin);
    int dataLen = val.m_msgData.length() - val.m_sendBegin;
    m_msgData.insert(0,data,dataLen);
    m_sendBegin = val.m_sendBegin;
    //cout<<"copy:"<<dataLen<<":"<<val.m_sendBegin<<endl;
    /*m_msgData = val.m_msgData;
    m_sendBegin = val.m_sendBegin;
    m_readPos = val.m_readPos;
    m_recvLen = val.m_recvLen;*/
}

void CNetMessage::WriteData(uint16 begin,void *pData,uint16 datalen)
{
    if(begin + datalen > (int)m_msgData.length())
    {
        //cout<<"write over overflow"<<endl;
        return;
    }
    char *p = (char*)m_msgData.c_str() + begin;
    memcpy(p, pData, datalen);
}

void CNetMessage::ReadData(void *pData,uint16 dataLen)
{
    if(m_readPos + dataLen > (int)m_msgData.length())
    {
        //cout<<"read over overflow"<<endl;
        return;
    }
    char *p = (char*)m_msgData.c_str() + m_readPos;
    memcpy(pData,p,dataLen);
    m_readPos += dataLen;
}

void CNetMessage::WriteData(void *pData,uint16 dataLen)
{
    m_msgData.insert(m_msgData.length(),(char*)pData,dataLen);
    SetDataLen();
}

void CNetMessage::ReadString(string &val)
{
    uint16 len = 0;
    ReadData(&len,sizeof(len));
    if(len <= 0)
        return;
    if(m_readPos + len > (int)m_msgData.length())
    {
        //cout<<"read string over overflow"<<endl;
        return;
    }
    char *p = (char*)(m_msgData.c_str() + m_readPos);
    val.clear();
    val.insert(0,p,len);
    m_readPos += len;
}

bool CNetMessage::RecvComplete()
{
    return (GetDataLen() == m_recvLen);
}

int CNetMessage::RecvMsg(int sockId)
{
    char *msg = (char*)m_msgData.c_str() + m_recvLen;
    int len = 0;
    if(m_recvLen < m_msgHeadLen)
    {
        len = recv(sockId,msg,m_msgHeadLen - m_recvLen,0);
        if(len > 0)
            m_recvLen += len;
        if (m_recvLen == m_msgHeadLen)
        {
            //assert(GetDataLen() < 4096);
            m_msgData.resize(GetDataLen());
        }
        return len;
    }
    msg = (char*)m_msgData.c_str() + m_recvLen;
    len = recv(sockId,msg,GetDataLen() - m_recvLen,0);
    if(len > 0)
        m_recvLen += len;
    /*PoolString *str = GetMsgData();
    for(int i = 0; i < (int)str->length(); i++)
    {
        cout<<(int)str->at(i)<<" ";
    }
    cout<<endl;*/
    return len;
}

int CNetMessage::SendMsg(int sockId)
{
    char *msg = (char*)m_msgData.c_str() + m_sendBegin;
    int len = send(sockId,msg,(int)(m_msgData.length() - m_sendBegin),0);
    if(len > 0)
    {
        m_sendBegin += len;
        //assert(m_sendBegin < 2048);
    }
    return len;
}

bool CNetMessage::SendComplete()
{
    bool flag = (m_sendBegin >= m_msgData.length());
    if(flag)
    {
        m_sendBegin = 0;
    }
    return flag;
}
