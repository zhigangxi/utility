#ifndef _ONLINE_USER_INFO_H_
#define _ONLINE_USER_INFO_H_
#include "HashTable.h"
#include <functional>
#include <list>
#include <stdint.h>

class CUser;

class COnlineUser
{
public:
    COnlineUser():
		m_onlineUser(6000)
    {
    }
    CUser *GetUser(uint32_t id);
    void ForEachUser(std::function<void(CUser*)> f);
    void AddUser(uint32_t id,CUser *);
    CUser *DelUser(uint32_t roleId);
private:
    CHashTable<uint32_t,CUser*>  m_onlineUser;
};

#endif
