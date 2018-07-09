#ifndef _COMMANG_H_
#define _COMMANG_H_
#include <functional>
#include "socket.h"
#include "HashTable.h"

typedef std::function<void(UserSession,SharedMsg)> CommandFun;

class CCommand
{
public:
	CCommand()
	{
	}
	void AddCommand(uint32_t type  ,CommandFun fun);
	void RunCmd(UserSession,uint32_t msgType,SharedMsg);
private:
	struct CmdData
	{
		CommandFun fun;
		uint32_t runTimes = 0;
	};
	CHashTable<uint32_t, CmdData> m_funcTable;
};
#endif
