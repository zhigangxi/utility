#include "stdafx.h"
#include "game_npc.h"
#include "DatabaseSql.h"
#include "socket_server.h"
#include "net_msg.h"
#include "protocol.h"
#include "despatch_command.h"
#include "global_manager.h"
#include <boost/bind.hpp>
#include "online_user.h"
#include "game_player.h"
#include <boost/lexical_cast.hpp>
#include "server_data_base.h"

bool CNpcManager::ReadTaskItem(list<STaskItem> *pItemList,string str,bool bReadTarget)
{
	if( str.empty() )
		return true;
    size_t pos = -1;
    do
    {
        STaskItem item;
        pos += 1;
        item.type = atoi(str.c_str() + pos);
        pos = str.find('|',pos);
        pos += 1;
        item.val = atoi(str.c_str() + pos);
        pos = str.find('|',pos);
        if(pos == string::npos)
            return false;
        pos += 1;
        item.num = atoi(str.c_str() + pos);
		if( bReadTarget )
		{
			pos = str.find('|',pos);
			if( pos == string::npos )
				return false;
			pos += 1;
			item.target = atoi(str.c_str() + pos);
		}
        pItemList->push_back(item);
    }while((pos = str.find('|',pos)) != string::npos);
    return true;
}

bool CNpcManager::WriteTaskItem(list<STaskItem>* pItemList,string& str, bool bWriteTarget )
{
	str.clear();
	list<STaskItem>::iterator iter = pItemList->begin();
	for ( ; iter != pItemList->end(); iter++ )
	{
		str += boost::lexical_cast<string,int>(iter->type) + "|" + boost::lexical_cast<string,int>(iter->val)
			+ "|" + boost::lexical_cast<string,int>(iter->num) + "|";
		if ( bWriteTarget )
			str += boost::lexical_cast<string,int>(iter->target) + "|";
	}
	if( !str.empty() )
		str  = str.substr(0,str.size()-1);
	return true;
}

bool CNpcManager::Init(CSocketServer *pSock)
{
    SCommand comDeal[] = {
		CG_TASKCOMMIT,boost::bind(&CNpcManager::CommitTask,this,_1,_2),
        CG_GET_TASKLIST,boost::bind(&CNpcManager::GetTaskList,this,_1,_2),
        CG_GET_TASKTARGET,boost::bind(&CNpcManager::GetTaskCondition,this,_1,_2),
        CG_ACCEPT_TASK,boost::bind(&CNpcManager::AcceptTask,this,_1,_2),
		CG_ABABDON_TASK,boost::bind(&CNpcManager::AbandonTask,this,_1,_2),
        CG_NPC_ITEM_LIST,boost::bind(&CNpcManager::NpcItemList,this,_1,_2),
        CG_BUY_NPC_ITEM,boost::bind(&CNpcManager::BuyNpcItem,this,_1,_2),
        CG_SKILL_LIST,boost::bind(&CNpcManager::QueryNpcSkill,this,_1,_2),
        CG_QUERY_SKILL,boost::bind(&CNpcManager::GetPlayerSkill,this,_1,_2)
    };
    CDespatchCommand *pDespatch;
    CGlobalValManager::CreateInstance()->GetVal(&pDespatch,EGV_CDESPATCH_COMMAND);
    pDespatch->AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));


    m_pSock = pSock;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if (pDb == NULL)
    {
        return false;
    }
    string sql = "select id,npc_id,name,type,repeat_times,need_level,occupation,depend_task_id,need_item,award_item,dialog,target from npc_task";
    if (!pDb->Query(sql.c_str()))
    {
        return false;
    }
    char **row;
    while((row = pDb->GetRow()) != NULL)
    {
        CNpcTask *pTask = new CNpcTask;
        pTask->m_id = atoi(row[0]);
        pTask->m_npcId = atoi(row[1]);
        pTask->m_name = row[2];
        pTask->m_type = (ETaskType)atoi(row[3]);
        pTask->m_canRepeatTimes = atoi(row[4]);
        pTask->m_level = atoi(row[5]);
        pTask->m_occupation = (UINT32)atoi(row[6]);
        pTask->m_dependTask = atoi(row[7]);
        pTask->m_dialog = row[10];
        pTask->m_target = row[11];
        if(ReadTaskItem(&(pTask->m_needItem),row[8]) && ReadTaskItem(&(pTask->m_awardItem),row[9]))
        {
            m_taskList.push_back(pTask);
        }
		else
		{
			cout<<"read task:"<<row[2]<<" error"<<endl;
			delete pTask;
		}
    }

    sql = "select id,name,type,res_name,x_pos,z_pos,direction,town_id from game_npc";
    if(!pDb->Query(sql.c_str()))
    {
        cout<<"read npc error"<<endl;
        return false;
    }
    while((row = pDb->GetRow()) != NULL)
    {
        CGameNpc *pNpc = new CGameNpc;
        pNpc->m_id = atoi(row[0]);
        pNpc->m_name = row[1];
        pNpc->m_typeMask = atoi(row[2]);
        pNpc->m_resName = row[3];
        pNpc->m_xPos = (float)atof(row[4]);
        pNpc->m_zPos = (float)atof(row[5]);
        pNpc->m_direction = atoi(row[6]);
        pNpc->m_townId = atoi(row[7]);
        m_npcList.push_back(pNpc);
    }
    return true;
}

void CNpcManager::SendNpcList(int sceneId,int sock)
{
    CNetMessage msg;
    NpcListIter iter = m_npcList.begin();
    int num = 0;
    int pos = msg.GetDataLen();
    msg<<num;
    msg.SetType(GC_SENCE_NPC);
    CGameNpc *pNpc = NULL;
    for (; iter != m_npcList.end(); iter++)
    {
        pNpc = *iter;
        if (pNpc->m_townId == sceneId)
        {
            num++;
            //npc类型，NPC模型文件名,NPC名字, NPCid, NPC方向,  NPC坐标x,z(flaot), 任务状态EUserTaskType(int 0表示没有)
            msg<<pNpc->m_typeMask<<pNpc->m_resName<<pNpc->m_name<<pNpc->m_id<<pNpc->m_direction<<pNpc->m_xPos<<pNpc->m_zPos<<GetNpcTaskState(pNpc->m_id,sock);
        }
    }
    if(num > 0)
    {
        msg.WriteData(pos,&num,sizeof(num));
        m_pSock->SendMsg(sock,msg);
    }
}

void CNpcManager::GetTaskList(CNetMessage &msg,int sock)
{
    //服务器返回任务相关 内容：  任务数量taskcount(int),任务ID taskid(int),任务标题 tasktitle(string) ，任务类型 tasktype(int)
    //GC_GET_TASKLIST,
    int npcId = 0;
    msg>>npcId;
    TaskListIter iter = m_taskList.begin();
    int num = 0;
    msg.ReWrite();
    int pos = msg.GetDataLen();
    msg<<num;
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist )
		player = userlist->GetUserBySock(sock);
	if( player == NULL )
		return;
    for (; iter != m_taskList.end(); iter++)
    {
        CNpcTask *pTask = *iter;
		player->m_taskList;//用于调试
        if(pTask->m_npcId == npcId && player->CanGetTask(pTask->m_id))
        {
            num++;
            msg.SetType(GC_GET_TASKLIST);
            msg<<pTask->m_id<<pTask->m_name<<pTask->m_type;
        }
    }
    if(num > 0)
    {
        msg.WriteData(pos,&num,sizeof(num));
        m_pSock->SendMsg(sock,msg);
    }
}

void CNpcManager::GetTaskCondition(CNetMessage &msg,int sock)
{
    //服务器返回任务目标及奖励  内容： 对话内容(string),任务目标  tasktarget (string),奖励数量 trophycount(int),奖励类型（int,1怪物,2物品,3经验,4金钱）,奖励值，奖励数量
    //奖励值：type为1、2时表示id，type为3、4时表示值
    //奖励数量：type为1、2时表示，type为3、4是无意义
    int taskId = 0;
    msg>>taskId;
    if(taskId == 0)
        return;

    TaskListIter iter = m_taskList.begin();
    int num = 0;
    msg<<num;
    for (; iter != m_taskList.end(); iter++)
    {
        CNpcTask *pTask = *iter;
        if(pTask->m_id == taskId)
        {
            msg.ReWrite();
            msg.SetType(GC_GET_TASKTARGET);
            msg<<pTask->m_dialog<<pTask->m_target<<(int)pTask->m_awardItem.size();
            list<STaskItem>::iterator i = pTask->m_awardItem.begin();
            for (; i != pTask->m_awardItem.end(); i++)
            {
                msg<<(*i).type<<(*i).val<<(*i).num;
            }
            m_pSock->SendMsg(sock,msg);
        }
    }
}

void CNpcManager::AcceptTask(CNetMessage &msg,int sock)
{
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist )
		player = userlist->GetUserBySock(sock);
	if( player == NULL )
		return ;
    int taskId;
    msg>>taskId;
    msg.ReWrite();
    msg.SetType(GC_ACCEPT_TASK);
	bool bacceptTask = false;
    TaskListIter iter = m_taskList.begin();
    for (; iter != m_taskList.end(); iter++)
    {
        CNpcTask *pTask = *iter;
        if(pTask->m_id == taskId && player->AddTask(pTask) )
        {
            msg<<(int)SUCCESS_FLAG<<taskId<<pTask->m_npcId<<pTask->m_name ;
            m_pSock->SendMsg(sock,msg);
			bacceptTask = true;
        }
    }
	if( !bacceptTask )
	{
		msg<<(int)ERROR_FLAG;
		m_pSock->SendMsg(sock,msg);
	}
	player->SendTaskList();
}

bool CPlayerTask::ok()
{
	list<STaskItem>::iterator iter = m_doneItem.begin();
	for (; iter != m_doneItem.end(); iter++ )
	{
		if( !(iter->ok()) )
			return false;
	}
	return true;
}

void CNpcManager::NpcItemList(CNetMessage &msg,int sock)
{
    //服务器像客户端返回物品列表，内容：物品数量（int）,物品id（int），物品名（string），(描述信息由客户端获取)
	int npcId ,itemId,itemNum; 
	npcId = itemId = itemNum = 0;
	msg >> npcId;
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
	{
		return ;
	}
	string sql = "select item_id from npc_item where npc_id = " + boost::lexical_cast<std::string,int>(npcId) ;
	if (!pDb->Query(sql.c_str()))
	{
		return;
	}
	char** row;
    itemNum = pDb->GetRowNum();
    msg.ReWrite();
    msg.SetType(GC_NPC_ITEM_LIST);
    msg<<itemNum;
	while((row = pDb->GetRow()) != NULL)
	{
		itemId  = atoi(row[0]);
		std::map<int,Logic::ItemEData_t>::iterator it = server_data::instance().ItemEData_map.find(itemId);
		if( server_data::instance().ItemEData_map.end() != it )
		{
			msg<<itemId<<string(it->second.m_szName);
		}
	}
    if (itemNum > 0)
    {
        m_pSock->SendMsg(sock,msg);
    }
}

void CNpcManager::BuyNpcItem(CNetMessage &msg,int sock)
{
    //购买是否成功返回，内容：物品id（int），数量（int），成功标志（int），如果是失败后面有失败原因（int，1不满足购买条件，2背包满）
    int itemId = 0;
    int num;
    msg>>itemId>>num;
    msg.ReWrite();
    msg.SetType(GC_BUY_NPC_ITEM);
    msg<<itemId<<num<<SUCCESS_FLAG;
    m_pSock->SendMsg(sock,msg);
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist && (player = userlist->GetUserBySock(sock)) )
    {
		player->PlayerAddItem(SItem::CreateItem(itemId));
		player->UT_AddItem(itemId);
		player->SendAllBackPackData();
    }
}

void CNpcManager::UseItem(CNetMessage &msg,int sock)
{
    int itemId = 0;
    msg>>itemId;
    //使用物品返回，内容：物品id（int），成功标志（int，SUCCESS_FLAG表示成功），
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist && (player = userlist->GetUserBySock(sock)) )
	{
		SItem item;
		if( player->GetItemById(itemId,item) && item.type_id == 40000 )
			WringEgg(&item,sock);
	}
    msg.ReWrite();
    msg.SetType(GC_USE_ITEM);
    if(itemId > 0)
        msg<<itemId<<SUCCESS_FLAG;
    else
        msg<<itemId<<ERROR_FLAG;
    m_pSock->SendMsg(sock,msg);
}

void CNpcManager::CommitTask(CNetMessage &msg,int sock)
{
	int taskId = 0;
	msg>>taskId;
	msg.ReWrite();
	msg.SetType(GC_TASKCOMMIT);
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist && (player = userlist->GetUserBySock(sock)) )
	{
		CNpcTask* pTask = GetTaskById(taskId);
		if( pTask == NULL || ( player->GetTaskById(taskId) == NULL ) )
		{
			msg << (int)ERROR_FLAG ;
			m_pSock->SendMsg(sock,msg);
			return;
		}
		list<SItem> itemlist;
		list<STaskItem>::iterator i;
		for ( i = pTask->m_awardItem.begin(); i != pTask->m_awardItem.end(); i++ )
		{
			switch ( i->type )
			{
			case ETIItem:
				{
					SItem item = SItem::CreateItem(i->val);
					itemlist.push_back(item);
					if( !player->PlayerAddItem(item) )
					{
						msg << (int)ERROR_FLAG ;
						m_pSock->SendMsg(sock,msg);
						for ( list<SItem>::iterator i = itemlist.begin(); i != itemlist.end(); i++ )
							player->PlayerLoseItem(*i);
						return;
					}
				}
				break;
			case ETIExpe:
					player->experence += i->num;
				break;
			case ETIMoney:
					player->money_ += i->num;
				break;
			default:
				break;
			}
		}
		
		for ( i = pTask->m_needItem.begin(); i != pTask->m_needItem.end(); i++ )
		{
			switch ( i->type )
			{
			case ETIItem:
				{
					if( !player->RemoveItem(i->val,i->num) )
					{
						msg << (int)ERROR_FLAG ;
						m_pSock->SendMsg(sock,msg);
						for ( list<SItem>::iterator i = itemlist.begin(); i != itemlist.end(); i++ )
							player->PlayerLoseItem(*i);
						return;
					}
				}
				break;
			case ETIExpe:
				{
					player->experence -= i->num;
					if( player->experence < 0 )
						player->experence = 0;
				}
				break;
			case ETIMoney:
				{
					player->money_ -= i->num;
					if( player->money_ < 0 )
						player->money_ = 0;
				}
				break;
			default:
				break;
			}
		}
		CNetMessage msg_playerinfo;
		player->SendAllBackPackData();
		player->PlayerRoleInfo(msg_playerinfo);//通知客户端
		player->UnCommitToDone(taskId);
		
		string name;
		msg << (int)SUCCESS_FLAG << (int)pTask->m_awardItem.size() ;
		for ( i = pTask->m_awardItem.begin(); i != pTask->m_awardItem.end(); i++ )
		{
			switch ( i->type )
			{
			case ETIItem:
				{
					std::map<int,Logic::ItemEData_t>::iterator item_iter = server_data::instance().ItemEData_map.find(i->val);
					if( item_iter != server_data::instance().ItemEData_map.end() )
						name = item_iter->second.m_szName ;
				}
				break;
			case ETIMoney:
				name = "金钱";
				break;
			case ETIExpe:
				name = "经验";
				break;
			}
			if( !name.empty() )	
				msg << i->num << name;
		}
		m_pSock->SendMsg(sock,msg);
		player->SendTaskList();
	}
}

bool CNpcManager::WringEgg(SItem* egg,int sock)
{
	EggAward award[10] = 
	{
        {0,10000,21701,1},{10001,15000,21702,1},{15001,30000,21711,1},{30001,34000,21712,1},{34001,50000,21721,1},
		{50001,60000,21722,1},{60001,63000,21731,1},{63001,80000,21732,1},{80001,90000,21741,1},{90001,99999,21742,1}
	};
	unsigned int itemId,num,r;
	itemId = num = r = 0;
	errno_t err;
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist && (player = userlist->GetUserBySock(sock)) )
	{
		//srand((unsigned int)time(NULL));
		err = rand_s(&r);
		if( err != 0 )
			r = 0;
		r = r%100000;
		for ( int i = 0; i < sizeof(award)/sizeof(EggAward); i++ )
		{
			if( award[i].begin <= r && award[i].end >= r )
			{
				itemId = award[i].itemId;
				num = award[i].num;
				break;
			}
		}
		if( player->PlayerAddItem(SItem::CreateItem(itemId)) )
		{
			player->PlayerLoseItem(*egg);
			player->SendAllBackPackData();
			return true;
		}
		else
			return false;
	}
	return false;

}	


EUserTaskType CNpcManager::GetNpcTaskState( int npcId,int sock )
{
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( userlist && (player = userlist->GetUserBySock(sock)) )
	{
		CPlayerTask temp;
		TaskList tasklist;
		if ( GetTaskListByNpc(npcId,tasklist) )
		{
			TaskListIter task_it = tasklist.begin();
			for (; task_it != tasklist.end(); task_it++ )
			{
				if ( player->IsTaskDoneUnCommit((*task_it)->m_id) )
					return ECompletedTask;
			}
			for ( task_it = tasklist.begin(); task_it != tasklist.end(); task_it++  )
			{
				if ( player->GetGotTask((*task_it)->m_id,temp) )
					return EGotTask;
			}
			for ( task_it = tasklist.begin(); task_it != tasklist.end(); task_it++  )
			{
				if ( player->CanGetTask((*task_it)->m_id) )
					return ECanGetTask;
			}
		}
	}
	return ECanNotGetTask;
}

bool CNpcManager::GetTaskListByNpc(int id,TaskList& tasklist)
{
	CGameNpc* npc = NULL;
	NpcListIter npc_it = m_npcList.begin();
	for ( ; npc_it != m_npcList.end(); npc_it++ )
	{
		if( id == (*npc_it)->m_id )
		{npc = *npc_it;break;}
	}
	if( npc == NULL || !(npc->m_typeMask & ENpcTask) )
		return false;
	TaskListIter task_it = m_taskList.begin();
	for ( ; task_it != m_taskList.end(); task_it++ )
	{
		if( (*task_it)->m_npcId == id )
			tasklist.push_back(*task_it);
	}
	if( tasklist.empty() )
		return false;
	return true;
}

CNpcTask* CNpcManager::GetTaskById(int id)
{
	TaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end(); it++ )
	{
		if( id == (*it)->m_id )
			return *it;
	}
	return NULL;

}

bool CNpcManager::IsTaskBelongToNpc( int taskId, int npcId )
{
	TaskList tasklist;
	if( GetTaskListByNpc(npcId,tasklist) )
	{
		for ( TaskListIter it = tasklist.begin(); it != tasklist.end(); it++ )
		{
			if( taskId == (*it)->m_id )
				return true;
		}
		return false;
	}
	return false;
}

void CNpcManager::AbandonTask(CNetMessage& msg,int sock)
{
	COnlineUserList* userlist = NULL;
	CPlayer* player = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&userlist,EGV_ONLINE_USER_LIST);
	if ( !userlist || !(player = userlist->GetUserBySock(sock)) )
		return;
	int taskId = 0;
	msg >> taskId;
	msg.ReWrite();
	msg.SetType(GC_ABABDON_TASK);
	CNpcTask* pTask = GetTaskById(taskId);
	if ( pTask && player->AbandonTask(taskId) )
	{
		msg << (int)SUCCESS_FLAG << pTask->m_npcId <<pTask->m_name << GetNpcTaskState(pTask->m_npcId,sock) ;
		m_pSock->SendMsg(sock,msg);
	}
	else
	{
		msg << (int)ERROR_FLAG << 0;
		m_pSock->SendMsg(sock,msg);
		return;
	}
	player->SendTaskList();
}

CGameNpc* CNpcManager::GetNpcById(int npcId)
{
	NpcListIter it = m_npcList.begin();
	for ( ; it != m_npcList.end(); it++ )
	{
		if( npcId == (*it)->m_id )
			return *it;
	}
	return NULL;
}

void CNpcManager::QueryNpcSkill(CNetMessage &msg,int sock)
{
    //1010,1013
    //服务器象客户端返回仅能，内容：num（int技能总数），skill_1（int），……
    msg.ReWrite();
    msg.SetType(GC_SKILL_LIST);
    int num = rand() % 8;
    msg<<num*4;
    for (int i = 0; i < num; i++)
    {
        msg<<1010<<1<<5<<3<<500<<0<<1
            <<1013<<1<<8<<5<<600<<1010<<0
            <<1014<<1<<10<<6<<0<<0<<0
            <<1015<<1<<11<<10<<1000<<0<<0;
    }

    m_pSock->SendMsg(sock,msg);
}

void CNpcManager::GetPlayerSkill(CNetMessage &msg,int sock)
{
    //服务器象客户端返回技能列表，内容：num（技能总数），skill_1(int)，……
    //GC_SKILL_LIST,
    CPlayer * pPlayer = COnlineUserList::instance().GetUserBySock(sock);
    if (pPlayer == NULL)
    {
        return;
    }
    size_t pos = 0;
    msg.ReWrite();
    msg.SetType(GC_QUERY_SKILL);
    size_t wPos = msg.GetDataLen();
    int num = 0;
    msg<<num;
    do 
    {
        int skill = atoi(pPlayer->m_skill.c_str()+pos);
        msg<<skill;
        num++;
    } while((pos = pPlayer->m_skill.find('|',pos+1)) != string::npos);
    msg.WriteData((UINT16)wPos,&num,(UINT16)sizeof(num));
    m_pSock->SendMsg(sock,msg);
}