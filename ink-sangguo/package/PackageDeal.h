#ifndef _PACKAGE_H_
#define _PACKAGE_H_
#include <list>
#include <boost/thread.hpp>
#include "socket.h"
#include <stdint.h>
#include <boost/timer.hpp>

using namespace std;

class CCommand;
class COnlineUser;
class CCmdPackageInterface;

class CPackageDeal
{
public:
	void AddPack(UserSession &pSession,uint32_t msgType,SharedMsg pMsg);
	void Logout(UserSession &pSession);
	void AddCmdPackage(CCommand *pCommand, CCmdPackageInterface*);

	void Run(CCommand *pCommand);
	void Stop()
	{
		m_run = false;
	}
	boost::thread *GetThread()
	{
		return m_pThread;
	}
private:
	struct SPack
	{
		UserSession pSession;
		uint32_t msgType;
		SharedMsg pMsg;
	};
	void DealPackThread();
	bool GetPack(SPack&);
	bool GetLogout(UserSession&);
	
	void DealOnePack(boost::posix_time::ptime now);

	list<SPack> m_packList;
	list<UserSession> m_logoutSession;
	boost::mutex m_mutex;
	boost::thread *m_pThread;
	CCommand *m_pCommand;
	COnlineUser *m_pOnlineUser;
	bool m_run;
};

#endif
