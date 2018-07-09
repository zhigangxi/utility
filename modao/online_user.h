#ifndef _ONLINE_USER_INFO_H_
#define _ONLINE_USER_INFO_H_
#include "hash_table.h"
#include "self_typedef.h"
#include <boost/pool/detail/singleton.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <list>
using namespace std;

class CUser;

class COnlineUser
{
public:
    COnlineUser():
    m_roleIdList(6000),
    m_sockIdList(6000)
    {
    }
    ShareUserPtr GetUserByRoleId(uint32 id);
    ShareUserPtr GetUserBySock(int id);
    void ForEachUser(boost::function<void(CUser*)> f);
    void GetUserList(list<uint32> &userList);
    
    void ForEachUserNoLock(boost::function<void(CUser*)> f);
    
    CUser *ReLogin(int sock,uint32 roleId);
    
    CUser        *AddUser(int sock,uint32 userId);
    void         SetRoleId(int sock,uint32 roleId);
    void         DelUser(CUser*);
private:
    //list<ShareUserPtr> m_garbageUser;
    CHashTable<uint32,ShareUserPtr>      m_roleIdList;
    CHashTable<int,ShareUserPtr>         m_sockIdList;
    boost::recursive_mutex m_mutex;
};

#endif
