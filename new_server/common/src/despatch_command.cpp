#include "stdafx.h"
#include "despatch_command.h"
#include "net_msg.h"
bool CDespatchCommand::AddCommand(UINT16 comType,CommandFun fun)
{
    /*if(m_funMap.find(comType) != m_funMap.end())
    {
        assert(1);
        return false;
    }*/
    //m_funMap[comType] = fun;
    m_funMap.AddVal(comType,fun);
    return true;
}

bool CDespatchCommand::Despatch(CNetMessage *pMsg,int sock)
{
    if(pMsg == NULL)
        return false;
    CommandFun fun;
    if(!m_funMap.Find(pMsg->GetType(),fun))
    {
        return false;
    }
    fun(*pMsg,sock);
    return true;
}

void CDespatchCommand::AddCommandDeal(SCommand *pBegin,int num)
{
    for (int i = 0; i < num; i++)
    {
        AddCommand(pBegin->comType,pBegin->comFun);
        pBegin++;
    }
}