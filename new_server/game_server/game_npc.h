#ifndef _GAME_NPC_H_
#define _GAME_NPC_H_
#include <list>
#include <map>
#include "protocol.h"
#include "singleton.h"
using namespace std;


class CGameNpc
{
public:
    int     m_id;
    string  m_name;
    UINT32  m_typeMask;
    string  m_resName;
    float   m_xPos;
    float   m_zPos;
    int     m_direction;
    int     m_townId;
};

enum ETaskType
{
    ETMainTask,//主线
    ETBranchTask,//支线
    ERepeatTask
};
enum ETaskItem
{
	ETIMonster = 1,//怪物
	ETIItem,//物品
	ETIExpe,//经验
	ETIMoney,//金钱
	ETITalkMan,//人物(对话)
};
struct STaskItem
{
    int     type;//1怪物,2物品,3经验,4金钱,5人物
    int     val;//id
    int     num;//数量
	int		target;//达到此数据可算完成此项
	STaskItem():target(0){}
	bool	match(int t,int v){ return (type == t && val == v); }
	bool	ok(){ return (target <= num) ;}
};
struct EggAward
{
	unsigned  int begin;
	unsigned  int end;
	unsigned  int itemId;
	unsigned  int num;
};
class CNpcTask
{
public:
    int         m_id;
    int         m_npcId;
    string      m_name;
    string      m_dialog;
    string      m_target;
    ETaskType   m_type;
    int         m_canRepeatTimes;
    int         m_level;//等级
    UINT32      m_occupation;//职业
    int         m_dependTask;//
    list<STaskItem>   m_needItem;//需要物品
    list<STaskItem>   m_awardItem;//奖励物品
};

//玩家的任务
class CPlayerTask
{
public:
	int		m_roleId;//角色id
	int		m_taskId;//任务id
	ETaskState	m_doneState;//完成状态 对应ETaskState
	int		m_repeatedTimes;//已重复的次数
	list<STaskItem>		m_doneItem;//已完成的部分（如已打死几只怪，或得到几个物品）
	CNpcTask* m_pTask;//任务模板指针
	CPlayerTask():m_roleId(0),m_taskId(0),m_doneState(UnDone),m_repeatedTimes(0),m_pTask(0){}
	bool ok();//检测是否完成
};

typedef list<CGameNpc*> NpcList;
typedef list<CGameNpc*>::iterator NpcListIter;
typedef list<CNpcTask*> TaskList;
typedef list<CNpcTask*>::iterator TaskListIter;
typedef list<CPlayerTask> PlayerTaskList;
typedef list<CPlayerTask>::iterator PlayerTaskListIter;

class CSocketServer;
class CNetMessage;
class SItem;
enum EUserTaskType;
class CNpcManager:public singleton<CNpcManager>

{
public:
    bool Init(CSocketServer*);
    void SendNpcList(int sceneId,int sock);
	bool ReadTaskItem(list<STaskItem>*,string,bool bReadTarget = false);
	bool WriteTaskItem(list<STaskItem>*,string&,bool bWriteTarget = false);

	
	bool WringEgg(SItem*,int);

	CNpcTask* GetTaskById(int id);
	bool IsTaskBelongToNpc( int taskId, int npcId );
	CGameNpc* GetNpcById(int npcId);

private:
	void CommitTask(CNetMessage&,int);
    void GetTaskList(CNetMessage&,int);
    void GetTaskCondition(CNetMessage&,int);
    void AcceptTask(CNetMessage&,int);
	void AbandonTask(CNetMessage&,int);//放弃任务
    void NpcItemList(CNetMessage&,int);
    void BuyNpcItem(CNetMessage&,int);
    void UseItem(CNetMessage&,int);
	void QueryNpcSkill(CNetMessage&,int);
    void GetPlayerSkill(CNetMessage&,int);

	bool GetTaskListByNpc(int id,TaskList& tasklist);//通过npc的id获得任务npc的所有任务
    EUserTaskType GetNpcTaskState( int npcId,int sock );//获得npc任务完成状态

    NpcList m_npcList;//NPC模板列表
    TaskList m_taskList;//任务模板列表
    CSocketServer *m_pSock;
};
#endif