#ifndef _MAIN_H_
#define _MAIN_H_
#include "socket.h"
#include "PackageDeal.h"

class CServer
{
public:
	CServer():m_socket(&m_pack)
	{
		m_thisServer = this;
	}
	bool Init();
	void Run();
	bool InitSingleton();
	static void SigHandler(int);
#ifdef WIN32
	static BOOL WINAPI HandlerRoutine(int);
#endif
private:
	CSocket m_socket;
	CPackageDeal m_pack;
	static CServer *m_thisServer;
	bool m_run;
};

#endif
