#ifndef _NET_MESSAGE_H_
#define _NET_MESSAGE_H_
#include <string>
#include <boost/pool/pool_alloc.hpp>
using namespace std;

typedef basic_string<char,char_traits<char>,boost::pool_allocator<char>> PoolString;

class CNetMessage
{
public:
    CNetMessage();
    UINT16      GetDataLen();
    void        SetDataLen();
    PoolString  *GetMsgData();
    void        SetType(UINT16);
    UINT16      GetType();
    void        ReWrite();
    void        ReRead();
    void        CopyData(PoolString&);
    void        WriteData(UINT16 begin,void *pData,UINT16 datalen);

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

    template<> CNetMessage &operator<< <string> (string val)
    {
        UINT16 len = (UINT16)val.size();
        WriteData(&len,sizeof(len));
        WriteData((void*)val.c_str(),(UINT16)val.size());
        return *this;
    }

    template<> CNetMessage &operator<< <const char*> (const char *val)
    {
        UINT16 len = (UINT16)strlen(val);
        WriteData(&len,sizeof(len));
        WriteData((void*)val,len);
        return *this;
    }

    template<> CNetMessage &operator<< <char*> (char *val)
    {
        UINT16 len = (UINT16)strlen(val);
        WriteData(&len,sizeof(len));
        WriteData((void*)val,len);
        return *this;
    }

    template<> CNetMessage &operator>> <string> (string &val)
    {
        ReadString(val);
        return *this;
    }

    template<> CNetMessage &operator<< <CNetMessage> (CNetMessage val)
    {
        m_msgData.append(val.m_msgData.substr(m_msgHeadLen));
        SetDataLen();
        return *this;
    }

    template<> CNetMessage &operator>> <CNetMessage> (CNetMessage &val)
    {
        val.m_msgData.append(m_msgData.substr(m_msgHeadLen));
        val.SetDataLen();
        return *this;
    }

    bool RecvComplete();
    int  RecvMsg(int sockId);
    int  SendMsg(int sockId);
    bool SendComplete();

    UINT16 GetSendBegin()
    {
        return m_sendBegin;
    }
private:
    void WriteData(void *pData,UINT16 dataLen);
    void ReadData(void *pData,UINT16 dataLen);
    void ReadString(string&);

    UINT16      m_readPos;
    PoolString  m_msgData;
    UINT16      m_recvLen;
    UINT16      m_sendBegin;

    const static int m_msgHeadLen   = 4;
    const static int m_msgTypeBegin = 2;
};

#endif