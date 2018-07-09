#ifndef _DESPATCH_COMMAND_H_
#define _DESPATCH_COMMAND_H_
#include <boost/function.hpp>
#include <list>
#include "self_typedef.h"
#include "hash_table.h"
class CNetMessage;
using namespace std;

typedef boost::function<void(CNetMessage*,int)> CommandFun;

struct  SCommand
{
    uint16      comType;
    CommandFun  comFun;
};

class CDespatchCommand
{
public:
    CDespatchCommand():m_funMap(512)
    {
    }
    void AddCommandDeal(SCommand *pBegin,int num);
    bool Despatch(CNetMessage*,int sock);
private:
    bool AddCommand(uint16 comType,CommandFun);
    CHashTable<int,CommandFun> m_funMap;
};
#endif
