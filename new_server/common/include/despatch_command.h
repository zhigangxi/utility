#ifndef _DESPATCH_COMMAND_H_
#define _DESPATCH_COMMAND_H_
#include <boost/function.hpp>
#include <hash_map>
#include <list>
#include "int_hash.h"
#include "singleton.h"
class CNetMessage;
using namespace std;

typedef boost::function<void(CNetMessage&,int)> CommandFun;

struct  SCommand
{
    UINT16      comType;
    CommandFun  comFun;
};

class CDespatchCommand:public singleton<CDespatchCommand>
{
public:
    CDespatchCommand():m_funMap(512)
    {
    }
    void AddCommandDeal(SCommand *pBegin,int num);
    bool Despatch(CNetMessage*,int sock);
private:
    bool AddCommand(UINT16 comType,CommandFun);
    CIntHashTable<CommandFun> m_funMap;
};

#endif;