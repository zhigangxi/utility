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
    ETMainTask,//����
    ETBranchTask,//֧��
    ERepeatTask
};
enum ETaskItem
{
	ETIMonster = 1,//����
	ETIItem,//��Ʒ
	ETIExpe,//����
	ETIMoney,//��Ǯ
	ETITalkMan,//����(�Ի�)
};
struct STaskItem
{
    int     type;//1����,2��Ʒ,3����,4��Ǯ,5����
    int     val;//id
    int     num;//����
	int		target;//�ﵽ�����ݿ�����ɴ���
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
    int         m_level;//�ȼ�
    UINT32      m_occupation;//ְҵ
    int         m_dependTask;//
    list<STaskItem>   m_needItem;//��Ҫ��Ʒ
    list<STaskItem>   m_awardItem;//������Ʒ
};

//��ҵ�����
class CPlayerTask
{
public:
	int		m_roleId;//��ɫid
	int		m_taskId;//����id
	ETaskState	m_doneState;//���״̬ ��ӦETaskState
	int		m_repeatedTimes;//���ظ��Ĵ���
	list<STaskItem>		m_doneItem;//����ɵĲ��֣����Ѵ�����ֻ�֣���õ�������Ʒ��
	CNpcTask* m_pTask;//����ģ��ָ��
	CPlayerTask():m_roleId(0),m_taskId(0),m_doneState(UnDone),m_repeatedTimes(0),m_pTask(0){}
	bool ok();//����Ƿ����
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
	void AbandonTask(CNetMessage&,int);//��������
    void NpcItemList(CNetMessage&,int);
    void BuyNpcItem(CNetMessage&,int);
    void UseItem(CNetMessage&,int);
	void QueryNpcSkill(CNetMessage&,int);
    void GetPlayerSkill(CNetMessage&,int);

	bool GetTaskListByNpc(int id,TaskList& tasklist);//ͨ��npc��id�������npc����������
    EUserTaskType GetNpcTaskState( int npcId,int sock );//���npc�������״̬

    NpcList m_npcList;//NPCģ���б�
    TaskList m_taskList;//����ģ���б�
    CSocketServer *m_pSock;
};
#endif