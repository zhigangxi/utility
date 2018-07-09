#pragma once

#include <boost/shared_ptr.hpp>
#include <ctime>
#include <map>
#include "vector3d.hpp"
#include "net_msg.h"
#include "logicdef.h"
#define PI 3.1415f
#define SEC  //second
#define MSEC //millisecond
class CScene;
class CPlayer;
enum EMonsterSeq;
typedef boost::shared_ptr<CScene> scene_ptr;
enum MONSTER_STATE//怪物状态
{
	WANDER,//闲置移动
	BATTLEMOVE,//战斗移动
	BATTLEDOGE,//攻击闪躲
	BATTLERUNAWAY,//战斗逃跑
	BATTLE,//战斗
};
class monster
{
public:
	monster(void);
	~monster(void);
	void fill_sendclient_pack(CNetMessage &msg);
	void fill_serverdata_pack(CNetMessage& msg);
	void fill_data(CNetMessage &msg);
	void load_drop(CNetMessage &msg);
	void PlayerCollectItem(CNetMessage &msg,int sock);
	void ProcessMsg(CNetMessage &msg,int sock);
	int  monster_id()			 {return insid;}
	bool IsDead()				 {return dead_;}
	void SetDead(bool bDead)	 {dead_ = bDead;}
	CScene* GetScene()			 {return m_pScene;}
	void SetScene(CScene* pScene){ m_pScene = pScene; }
	void BeAttack(CPlayer* player);
	void AI_OnLoop(DWORD dwTime);
private:
	void AI_GoOneStep(float speed,DWORD dwTime);//走一步
	void AI_WalkOneStep(DWORD dwTime); //闲置移动
	void AI_PursuitOneStep(DWORD dwTime);//战斗移动
	void SendMovement(EMonsterSeq movement);//发送动作消息
	void AI_Attack();//攻击中目标的血量减少
	float CalcDistance(vector3d src,vector3d dest);//计算两点距离
	bool IsPlayerInSpyRange(CPlayer* player);//判断玩家是否在侦测范围内
	bool IsTargetInAttRange();//判断目标是否在可攻击的范围内
	bool IsActivePursuit();//是否主动追击类型
	float GetMoveSpeed(){return m_pMonsterT->m_fMoveSpeed*200;}//获得移动速度
	float GetPursuitSpeed(){ return 1.5*GetMoveSpeed(); }//获得追击速度
	float GetAttSpeed(){ return 20/*m_pMonsterT->m_nAttackSpeed*/; }//获得攻击速度
	bool CalcAttPoint(vector3d& point);//计算攻击点
	void ChangeFaceToPos( const vector3d& pos );//改变方向，使怪物面向pos点
	bool CheckCrash();//碰撞检测，检测是否打中目标
	bool CheckAccuracy();//命中检测，检测命中率
	bool CheckTimeExpired(DWORD dwTime);//检测攻击频率
private:
	bool dead_;
	//add by fly
	CScene*					m_pScene;//场景
	DWORD					m_lasttime;//上次时间
	DWORD					m_lastHitTime;//上次攻击一下的时间
	MONSTER_STATE			m_state;//怪物状态
	CPlayer*				m_pTarget;//目标
	Logic::MonsterAtt_t*	m_pMonsterT;//模板
	bool					m_bBeAttack;//是否被攻击
	CPlayer*				m_pAttacker;//攻击者
public:
	int insid;//实例id
	vector3d originPos;//出生坐标
	int originFace;//出生朝向
	vector3d pos;//坐标
	int face_;//朝向
	int	m_nMonsterID;	//怪物编号
	//int	m_eMonsterRace;	//怪物种族
	//int	m_eMonsterPhysique;	//怪物体质
	std::string m_szMonsterIcon;	//怪物头像
	std::string	m_szMonsterTitle;	//怪物称号
	std::string	m_szMonsterName;	//怪物名字
	std::string	m_szMonsterModel;	//怪物模型
	float	m_fMonsterZoom;	//怪物缩放
	//std::string	m_szMonsterArt;	//怪物贴图
	//int	m_nMonsterWeapon;	//怪物武器
	//std::string	m_szMonsterEffect;	//武器特效
	int	m_eBossTag;	//BOSS标记
	//std::string	m_szTextureName1;	//特效贴图1
	//std::string	m_szTextureName2;	//特效贴图2
	//int	m_nMonsterTextureID;	//特效贴花
	//int	m_eGoalAIID;	//目标选择AI
	int	m_nMonsterLev;	//怪物等级
	int	m_nPhAttack;	//物理攻击
	int	m_nMaAttack;	//法术攻击
	int	m_nPhDefend;	//物理防御
	int	m_nMaDefend;	//法术防御
	int	m_nMonsterHP;	//生命值
	//int	m_nHPResume;	//生命回复
	int	m_nMonsterMP;	//法力值
	//int	m_nMPResume;	//法力回复
	//int	m_nAttackSpeed;	//攻击速度
	//int	m_eAttackMode;	//攻击状态
	//int	m_nSpyRange;	//侦测范围
	//int	m_nMoveRange;	//移动范围
	//int	m_ePursuitType;	//追击范围
	//int	m_nAttackDistance;	//攻击范围
	//int	m_nFendResist;	//击退抵抗
	//int	m_nImpendenceResist;	//悬空抵抗
	//int	m_nSkill1;	//技能1
	//int	m_nSkill2;	//技能2
	//int	m_nSkill3;	//技能3
	//int	m_nSkill4;	//技能4
	//int	m_nSkill5;	//技能5
	//int	m_nSkill6;	//技能6
	//int	m_eMoveType;	//移动类型
	//int	m_nHigh;	//距离地面高度
	//int	m_nJumpDistance;	//跳跃距离
	//float	m_fMoveSpeed;	//移动速度
	//int	m_eIdleMoveOrient;	//闲置移动路径
	//int	m_nIdleMoveDistan;	//闲置移动一次距离
	//int	m_eborderObject;	//边界物件
	//int	m_eHardObject;	//不可破坏物件
	//int	m_eAverageObject;	//一般破坏物件
	//int	m_ePlayerObject;	//玩家破坏物件
	__int64	m_nEXP;	//经验值
	int	m_nSPPro;	//SP点几率
	int	m_nMoneyPro;	//银币几率
	int	m_nMoney;	//银币
	int	m_nEquipPro1;	//掉落几率1
	int	m_nEquipItem1;	//掉落编号1
	int	m_nEquipPro2;	//掉落几率2
	int	m_nEquipItem2;	//掉落编号2
	int	m_nEquipPro3;	//掉落几率3
	int	m_nEquipItem3;	//掉落编号3
	int	m_nEquipPro4;	//掉落几率4
	int	m_nEquipItem4;	//掉落编号4
	int	m_nEquipPro5;	//掉落几率5
	int	m_nEquipItem5;	//掉落编号5
	int	m_nSpecialArtPro;	//特殊几率
	int	m_nSpecialArt;	//特殊物品
	int	m_nScriptID;	//脚本事件
	int m_nModelSize;//模型大小
	DWORD m_dwHitIntervalTime;//攻击间隔时间
};

typedef boost::shared_ptr<monster> monster_ptr;
typedef map<int,monster_ptr> monster_map;

