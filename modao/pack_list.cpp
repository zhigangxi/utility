#include "pack_list.h"
#include "net_msg.h"
#include <algorithm>
#include <boost/bind.hpp>

CNetMessage *CPackageList::GetMsg(int &sock)
{
    CNetMessage *pMsg = NULL;
    try
    {
        //if(m_semphone.timed_wait(const boost::posix_time::ptime & abs_time))
        if(m_semphone.try_wait())
        {
            boost::mutex::scoped_lock lk(m_listMutex);
            if(m_msgList.size() > 0)
            {
                pMsg = (*m_msgList.begin()).second;
                sock = (*m_msgList.begin()).first;
                m_msgList.pop_front();
            }
        }
        else
        {
            usleep(50);
        }
    }
    catch(...)
    {
    }
    return pMsg;
}

bool MsgEqual(const MsgNode &node,int sock)
{
    return node.first == sock;
}

bool CPackageList::AddMsg(CNetMessage *pMsg,int sock)
{
    boost::mutex::scoped_lock lk(m_listMutex);
    int num = std::count_if(m_msgList.begin(),m_msgList.end(),boost::bind(MsgEqual,_1,sock));
    if(num > 200)
    {
        delete pMsg;
        return false;
    }
    m_msgList.push_back(make_pair(sock,pMsg));
    m_semphone.post();
    return true;
}

void CPackageList::DelMsg(int sock)
{
    boost::mutex::scoped_lock lk(m_listMutex);
    list<MsgNode>::iterator i = m_msgList.begin();
    list<MsgNode>::iterator next = i;
    for(; i != m_msgList.end();)
    {
        next = i;
    	next++;
        if(i->first == sock)
        {
            delete i->second;
            m_msgList.erase(i);
        }
        i = next;
        //pMsg = (*m_msgList.begin()).second;
        //sock = (*m_msgList.begin()).first;
        //m_msgList.pop_front();
    }
}

