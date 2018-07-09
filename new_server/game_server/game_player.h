#pragma once
#include "net_msg.h"
#include "vector3d.hpp"
#include <hash_map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game_npc.h"
#include "Item.h"



class monster;
typedef boost::shared_ptr<monster> monster_ptr;
class CPlayer
{

public:
	CPlayer():userId(0),roleId(0),sock(0),curSceneId(0),m_nModelSize(100)
	{
	}
    static void PlayerQuitGame(CPlayer &player);
private:
	void PlayerEnterGame(CNetMessage &msg);
	void PlayerMove(CNetMessage &msg);
	void UpdatePlayerPoint(CNetMessage& msg);
	void PacketPlayerInfo(CNetMessage& msg);
	void UpdatePlayerInfo(CNetMessage& msg);
	void SavingPlayerInfo();
	void UpdateBackpackData(CNetMessage& msg,CNetMessage& msg1);
	void SavingBackpackData();
	void FectchBackpackData(CNetMessage& msg);
	void PlayerChat(CNetMessage& msg);
	void ChatBroadcast(CPlayer& player);
	void PlayerAddAttriPoint(CNetMessage& msg);
	void AnalyzeData(string &str,SItem &item);
	void CombinationData(SItem &item);
	void PlayerDeleteItem(CNetMessage& msg);
	void PlayerUseItem(CNetMessage& msg);
	void PlayerExchangeItemPlace(CNetMessage& msg);
	void CountPlayerAttr();
	void SetExpendShortCut(CNetMessage& msg);
	void TrimPack(CNetMessage& msg);
	void SendAndUpdataSowlStoneInfo();
	void SaveSowlStoneBagToDB();
	void GetSolwStoneInfoFromDB();
	/*�������  begin*/
	void UpdateTask(int type,int value,int num);//��������
	CPlayerTask* GetUnDoneTaskById(int taskId);//ͨ������id�õ�����
	CPlayerTask* GetUnCommitTaskById(int taskId);//ͨ������id�õ�����
	CPlayerTask* GetTaskByIdFromList( int taskId, PlayerTaskList& tasklist );
	string GetTaskItemName(int type,int id = 0);//���������Ĺ�����������Ʒ������
	int GetUnDoneAndUnCommitTaskNum();//�õ�������������
	void RemoveTask(int taskId);//���б�ɾ������
	int  GetItemNumByTypeId( int itemTypeId );//ͨ�����ͻ����Ʒ����
	/*�������  end*/
public:
	//void AddCommandDeal();
	void PlayerRoleInfo(CNetMessage& msg);
	void PlayerUpgrade();
	void PacketPlayerNotifyInfo(CNetMessage& msg);
	int PlayerAddItem(SItem& item,int needspace = 1);
	int PlayerLoseItem(SItem& item);
	int  PlayerCheckItemSpace(int bagid);
	void SendUpdateBackPackData(int bagid);
	void SendAllBackPackData();
	void PlayerAddSowlStone(SItem* item);
	void ProcessMsg(CNetMessage& msg);
	/*�������  begin*/
	bool AddTask(CNpcTask* pTask);//�������
	void UT_KillMonster( int id );//ɱ�������������
	void UT_AddItem( int id, int num = 1);//������Ʒ�������ʰȡ������������
	void UT_AddExp( int exp = 0 );//���Ӿ��飬��������
	void UT_AddMoney( int money = 0 ) ;//���ӽ�Ǯ����������
	bool LoadTaskFromDB();//�����ݿ��������
	bool SaveTaskToDB();//�������������ݿ�
//	SItem CreateItem(int type_id,int num=1);//ͨ������id������Ʒ
	int GetItemById(int id,SItem& item);//ͨ����Ʒʵ��id������Ʒ
	bool RemoveItem(int type_id,int num = 1);//ͨ����Ʒ����idɾ����Ʒ
	bool GetGotTask(int taskId,CPlayerTask& task);//��ȡ����ȡ��δ��ɵ�����
	bool IsTaskDoneUnCommit(int taskId);//�ж�ĳ�����Ƿ����굫��δ�ύ
	bool CanGetTask( int taskId );//�ж�����Ƿ�ɽӴ�����
	bool IsTaskDone( int taskId );//�ж�ĳ�����Ƿ���ɣ����ύ��
	void UnCommitToDone(int taskId);//��δ�ύ�б��Ƶ�����б�
	void SendTaskList();//���������б� 
	void SendTaskListWithNpc(int npcId);//����npc��ص������б�
	bool AbandonTask(int taskId);//��������
	bool IsAcceptTask(int taskId);//�Ƿ��ǽ����˵�����
	CPlayerTask* GetDoneTaskById(int taskId);//ͨ������id�õ�����
	CPlayerTask* GetTaskById(int taskId);//ͨ��id�õ�����
	/*�������  end*/
	/*���Է��� begin*/
	vector3d GetPosition(){ return pos_; }
	int GetSock(){return sock;}
	int GetMaxHP(){return maxhp_;}
	int GetMaxMp(){return maxmp_;}
	int GetHP(){return hp_;}
	void SetHP(int hp){ hp_ = hp; }
	int GetMP(){return mp_;}
	void SetMP(int mp){ mp_ = mp; }
	int GetRoleId(){return roleId;}
	int GetModelSize(){return m_nModelSize;}
	float GetHitRate(){return (float)hitrate_/100;}
	/*���Է��� end*/
	bool IsDead(){return hp_ <= 0;}
	int CalcAttDamage(CPlayer* pTarget);
	int CalcAttDamage(monster_ptr pMonster);

public:
	int userId;
	int roleId;
	int sock;
	int model_id_;
	int action_;
	vector3d pos_;
	int dir_;
	std::string name_;
	std::string nick_;
	__int64    experence;
	__int64    next_exp;
	int race_;
	int occupation_;
	int level_;
	int power_; //�׻�֮��
	int quick_; //��ȸ֮��
	int intelligence_; //����֮��
	int energy_; //����֮��
	int maxhp_;
	int maxmp_;
	int phyattack_;
	int maattack_;
	int phydefend_;
	int madefend_;
	int attackspeed_;
	int hitrate_;//������
	int movespeed_;
	int evadePro_;
	int releasePro_;
	int MPRevert_;
	int HPRevert_;
	int CarryWeight_;
	int RigidTime_;
	int resistance_;
	int FrozenRessis_;
	int BurnRessis_;
	int PalsyRessis_;
	int ToxinRessis_;
	int VertigoRessis_;
	int skillRessis_;
	int MARessis_;
	int weakRessis_;
	int reputation_;
	int AttriPoint_;
	int SkillPoint_;
	int AttackStorm_;
	int lucky_;
	int money_;
	int gold_;
	int hairColor;
	int	glass;
	int neckchain;
	int cape;
	int belt;
	int mask;
	int wig;
	int headwear;
	int shoulderpad;
	int armor;
	int armguard;
	int legguard;
	int shoes;
	int roleplace;
	int head;
	int weapon;
	int hp_;
	int mp_;
	bool if_change;

	int equippack_id;
	int expendpack_id;
	int materialpack_id;
	int taskpack_id;
	int decorpack_id;
	int backpack_weight;

	SItem equip_data[52];
	SItem expend_data[52];
	SItem material_data[52];
	SItem task_data[52];
	SItem decor_data[52];
	int expend_shortcut[5];

	SowlStone* sowlstone_data[21];
    string m_skill;


	int curSceneId;
	string chatContent;
	int channel;
	PlayerTaskList m_taskList;//�����б�
	int m_nModelSize;
};


typedef hash_map<int,CPlayer*>::iterator HashIter;
typedef boost::shared_ptr<CPlayer> player_ptr;