#include "stdafx.h"
#include "net_msg.h"
#include <stack>
#include <boost/pool/singleton_pool.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
using namespace boost;

//#define PoolMalloc(memType,memData) \
//{\
//    boost::pool<> *pPool = &(GetThreadData()->m_p##memType##Pool); \
//    memData = (memType*)pPool->malloc();\
//}
//
//#define PoolFree(memType,memData)\
//{\
//    boost::pool<> *pPool = &(GetThreadData()->m_p##memType##Pool); \
//    pPool->free(memData);\
//}
//
//struct ThreadData
//{
//    ThreadData();
//    pool<> m_pSMessageDataPool;
//    pool<> m_pCNetMessagePool;
//};
//
//static thread_specific_ptr<ThreadData> sThreadDataPtr;
//static ThreadData *GetThreadData()
//{
//    ThreadData *p = sThreadDataPtr.get();
//    if(p == NULL)
//    {
//        p = new ThreadData;
//        sThreadDataPtr.reset(p);
//    }
//    return p;
//}

//ThreadData::ThreadData():
//    m_pSMessageDataPool(sizeof(SMessageData))
//    ,m_pCNetMessagePool(sizeof(CNetMessage))
//{
//}

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
}

void CNetMessage::ReRead()
{
    m_readPos = m_msgHeadLen;
}

void CNetMessage::ReWrite()
{
    m_msgData.clear();
    m_msgData.resize(m_msgHeadLen);
}

UINT16 CNetMessage::GetDataLen()
{
    return *(UINT16*)(m_msgData.c_str()) + m_msgHeadLen;
}

void CNetMessage::SetDataLen()
{
    *(UINT16*)(m_msgData.c_str()) = (UINT16)m_msgData.length() - m_msgHeadLen;
}

void CNetMessage::SetType(UINT16 type)
{
    *(UINT16*)(m_msgData.c_str() + m_msgTypeBegin) = type;
}

UINT16 CNetMessage::GetType()
{
    return *(UINT16*)(m_msgData.c_str() + m_msgTypeBegin);
}

PoolString *CNetMessage::GetMsgData()
{
    return &m_msgData;
}

void CNetMessage::CopyData(PoolString &str)
{
    m_msgData = str;
}

void CNetMessage::WriteData(UINT16 begin,void *pData,UINT16 datalen)
{
    if(begin + datalen > (int)m_msgData.length())
    {
        cout<<"write over overflow"<<endl;
        return;
    }
    char *p = (char*)m_msgData.c_str() + begin;
    memcpy(p, pData, datalen);
}

void CNetMessage::ReadData(void *pData,UINT16 dataLen)
{
    if(m_readPos + dataLen > (int)m_msgData.length())
    {
        cout<<"read over overflow"<<endl;
        return;
    }
    char *p = (char*)m_msgData.c_str() + m_readPos;
    memcpy(pData,p,dataLen);
    m_readPos += dataLen;
}

void CNetMessage::WriteData(void *pData,UINT16 dataLen)
{
    m_msgData.insert(m_msgData.length(),(char*)pData,dataLen);
    SetDataLen();
}

void CNetMessage::ReadString(string &val)
{
    UINT16 len;
    ReadData(&len,sizeof(len));
    if(m_readPos + len > (int)m_msgData.length())
    {
        cout<<"read string over overflow"<<endl;
        return;
    }
    char *p = (char*)m_msgData.c_str() + m_readPos;
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
            m_msgData.resize(GetDataLen());
        }
        return len;
    }
    msg = (char*)m_msgData.c_str() + m_recvLen;
    len = recv(sockId,msg,GetDataLen() - m_recvLen,0);
    if(len > 0)
        m_recvLen += len;
    return len;
}

int CNetMessage::SendMsg(int sockId)
{
    char *msg = (char*)m_msgData.c_str() + m_sendBegin;
    int len = send(sockId,msg,(int)(m_msgData.length() - m_sendBegin),0);
    if(len > 0)
        m_sendBegin += len;
    return len;
}

bool CNetMessage::SendComplete()
{
    return (m_sendBegin == GetDataLen());
}