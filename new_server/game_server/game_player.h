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
	/*任务相关  begin*/
	void UpdateTask(int type,int value,int num);//更新任务
	CPlayerTask* GetUnDoneTaskById(int taskId);//通过任务id得到任务
	CPlayerTask* GetUnCommitTaskById(int taskId);//通过任务id得到任务
	CPlayerTask* GetTaskByIdFromList( int taskId, PlayerTaskList& tasklist );
	string GetTaskItemName(int type,int id = 0);//获得任务项的怪物或人物或物品的名字
	int GetUnDoneAndUnCommitTaskNum();//得到所接任务数量
	void RemoveTask(int taskId);//从列表删除任务
	int  GetItemNumByTypeId( int itemTypeId );//通过类型获得物品数量
	/*任务相关  end*/
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
	/*任务相关  begin*/
	bool AddTask(CNpcTask* pTask);//添加任务
	void UT_KillMonster( int id );//杀死怪物，更新任务
	void UT_AddItem( int id, int num = 1);//增加物品（购买或拾取），更新任务
	void UT_AddExp( int exp = 0 );//增加经验，更新任务
	void UT_AddMoney( int money = 0 ) ;//增加金钱，更新任务
	bool LoadTaskFromDB();//从数据库加载任务
	bool SaveTaskToDB();//保存任务至数据库
//	SItem CreateItem(int type_id,int num=1);//通过类型id创建物品
	int GetItemById(int id,SItem& item);//通过物品实例id查找物品
	bool RemoveItem(int type_id,int num = 1);//通过物品类型id删除物品
	bool GetGotTask(int taskId,CPlayerTask& task);//获取已领取但未完成的任务
	bool IsTaskDoneUnCommit(int taskId);//判断某任务是否做完但还未提交
	bool CanGetTask( int taskId );//判断玩家是否可接此任务
	bool IsTaskDone( int taskId );//判断某任务是否完成（已提交）
	void UnCommitToDone(int taskId);//从未提交列表移到完成列表
	void SendTaskList();//发送任务列表 
	void SendTaskListWithNpc(int npcId);//发送npc相关的任务列表
	bool AbandonTask(int taskId);//放弃任务
	bool IsAcceptTask(int taskId);//是否是接受了的任务
	CPlayerTask* GetDoneTaskById(int taskId);//通过任务id得到任务
	CPlayerTask* GetTaskById(int taskId);//通过id得到任务
	/*任务相关  end*/
	/*属性访问 begin*/
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
	/*属性访问 end*/
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
	int power_; //白虎之力
	int quick_; //朱雀之灵
	int intelligence_; //青龙之神
	int energy_; //玄武之精
	int maxhp_;
	int maxmp_;
	int phyattack_;
	int maattack_;
	int phydefend_;
	int madefend_;
	int attackspeed_;
	int hitrate_;//命中率
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
	PlayerTaskList m_taskList;//任务列表
	int m_nModelSize;
};


typedef hash_map<int,CPlayer*>::iterator HashIter;
typedef boost::shared_ptr<CPlayer> player_ptr;