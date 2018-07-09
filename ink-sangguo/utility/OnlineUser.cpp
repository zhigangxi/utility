#include "OnlineUser.h"

CUser *COnlineUser::GetUser(uint32_t id)
{
	CUser *pUser = NULL;
	m_onlineUser.Find(id,pUser);
	return pUser;
}

static bool EachUser(uint32_t,CUser *pUser,std::function<void(CUser*)> *f)
{
    if(pUser != NULL)
    {
        (*f)(pUser);
    }    
    return true;
}

void COnlineUser::ForEachUser(std::function<void(CUser*)> f)
{
	m_onlineUser.ForEach(std::bind(EachUser, std::placeholders::_1, std::placeholders::_2, &f));
}

void COnlineUser::AddUser(uint32_t id,CUser *pUser)
{
	if(pUser == NULL)
		return;
	m_onlineUser.Insert(id,pUser);
}

CUser *COnlineUser::DelUser(uint32_t roleId)
{
	CUser *pUser = NULL;
	m_onlineUser.Erase(roleId,pUser);
	return pUser;
}
