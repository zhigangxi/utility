#ifndef _PACK_LIST_H_
#define _PACK_LIST_H_
#include <utility>
#include <list>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;

class CNetMessage;
typedef pair<int,CNetMessage*> MsgNode;

class CPackageList
{
public:
    CPackageList():m_semphone(0)
    {
    }

    CNetMessage *GetMsg(int &sock);
    bool AddMsg(CNetMessage *pMsg,int sock);
    void DelMsg(int sock);
private:
    list<MsgNode> m_msgList;
    boost::interprocess::interprocess_semaphore m_semphone;
    boost::mutex    m_listMutex;
};

#endif
