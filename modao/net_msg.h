#ifndef _NET_MESSAGE_H_
#define _NET_MESSAGE_H_
#include <string>
#include <boost/pool/pool_alloc.hpp>
#include <iostream>
#include <string.h>
#include "self_typedef.h"
#include "utility.h"

using namespace std;

//typedef basic_string<char,char_traits<char>,boost::pool_allocator<char> > PoolString;
typedef string PoolString;

class CNetMessage
{
public:
    CNetMessage();
    uint16      GetDataLen();
    void        SetDataLen();
    PoolString  *GetMsgData();
    void        SetType(uint16);
    uint16      GetType();
    void        ReWrite();
    void        ReRead();
    void        CopyData(CNetMessage&);
    void        WriteData(uint16 begin,void *pData,uint16 datalen);

    void *operator new(size_t);
    void operator delete(void*);

    template<typename Type>
    CNetMessage &operator<<(Type val)
    {
        WriteData(&val,sizeof(val));
        return *this;
    }

    template<typename Type>
    CNetMessage &operator>>(Type &val)
    {
        ReadData(&val,sizeof(val));
        return *this;
    }   

    bool RecvComplete();
    int  RecvMsg(int sockId);
    int  SendMsg(int sockId);
    bool SendComplete();
    void ReSend()
    {
        m_sendBegin = 0;
    }
    uint16 GetSendBegin()
    {
        return m_sendBegin;
    }
    void WriteData(void *pData,uint16 dataLen);
    void Append(CNetMessage &val);
private:
    void ReadData(void *pData,uint16 dataLen);
    void ReadString(string&);

    uint16      m_readPos;
    PoolString  m_msgData;
    uint16      m_recvLen;
    uint16      m_sendBegin;
    const static uint32 MAX_SEND_PACK_LEN = 8192;
    const static int m_msgHeadLen   = 4;
    const static int m_msgTypeBegin = 2;
};

template<> inline CNetMessage &CNetMessage::operator << <string>(string val)
{
    uint16 len = (uint16)val.size();
    string str;
    str.resize(len*2);
    len = GbkToUnicode((char*)str.c_str(),len*2,(char*)val.c_str(),len);

    WriteData(&len,sizeof(len));
    if(len > 0)
    {
        WriteData((void*)str.c_str(),len);
    }
    return *this;
}

template<> inline CNetMessage &CNetMessage::operator << <const char*> (const char *val)
{
    uint16 len = (uint16)strlen(val);
    string str;
    str.resize(len*2);
    len = GbkToUnicode((char*)str.c_str(),len*2,(char*)val,len);

    WriteData(&len,sizeof(len));
    if(len > 0)
    {
        WriteData((void*)str.c_str(),len);
    }
    return *this;
}

template<> inline CNetMessage &CNetMessage::operator<< <char*> (char *val)
{
    uint16 len = (uint16)strlen(val);
    string str;
    str.resize(len*2);
    len = GbkToUnicode((char*)str.c_str(),len*2,(char*)val,len);

    WriteData(&len,sizeof(len));
    if(len > 0)
    {
        WriteData((void*)str.c_str(),len);
    }
    return *this;
}

template<> inline CNetMessage &CNetMessage::operator>> <string> (string &val)
{
    string str;
    ReadString(str);

    uint16 len = (uint16)str.size();
    val.resize(len);
    len = UnicodeToGbk((char*)val.c_str(),len,(char*)str.c_str(),str.size());
    val.resize(len);
    return *this;
}

template<> inline CNetMessage &CNetMessage::operator<< <CNetMessage> (CNetMessage val)
{
    m_msgData.append(val.m_msgData.substr(m_msgHeadLen));
    SetDataLen();
    return *this;
}

template<> inline CNetMessage &CNetMessage::operator>> <CNetMessage> (CNetMessage &val)
{
    val.m_msgData.append(m_msgData.substr(m_msgHeadLen));
    val.SetDataLen();
    return *this;
}
#endif
