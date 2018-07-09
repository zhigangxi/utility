#pragma once
#include <stdint.h>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <list>
#include <unordered_map>
#include "NetMsg.pb.h"
#include "macros.h"
#include <boost/date_time.hpp>

class CHeroInst;
class CUser;

enum VSInputType {
	VSInputType_INVALID = -1,
	VSInputType_NONE = 0,
	VSInputType_ATTACK,
	VSInputType_DEFENCE,
	VSInputType_ULTRA,
};


enum VSType {
	VSType_INVALID = -1,
	VS_TYPE_NONE = 0,
	VS_TYPE_PRACTICE,
	VS_TYPE_BIOGRAPHY,
};

struct VsAward
{
	VSType type;
	uint32_t WinAwardId = 0;
	uint32_t DrawAwardId = 0;
	uint32_t LoseAwardId = 0;
};

struct VsRule
{
	VSInputType Player1Input;
	VSInputType Player2Input;
	int Player1LostHealth = 0;
	int Player1DeltaQi = 0;
	int Player2LostHealth = 0;
	int Player2DeltaQi = 0;
};

class CVsBattle
{
public:
	//初始怒气升星过几次就是几
	bool Init(uint64_t battleId, VSType type, uint32_t userId, CHeroInst *userHero, uint32_t heroId);
	void GetEnemy(NetMsg::VSBattleEnemy &enemy);
	uint32_t SetOptType(uint32_t turn, uint32_t optType);
	void SetReadTime();

	bool Update(boost::posix_time::ptime now);//this battle finished return true 
private:
	bool DoBattleTurn(CUser *user);
	void BattleEnd(CUser *user);
	void CalculationBattle(uint32_t opt1,uint32_t opt2, int strength1,int strength2,int &hp1,int &hp2,int &nuqi1,int &nuqi2);

	uint32_t AutoSelOpt(bool user);
	void AddTurn();

	uint32_t m_userId;
	uint32_t m_userHeroId;
	uint32_t m_heroId;
	uint32_t m_heroStar;
	int m_userHeroStrength;
	int m_heroStrength;

	DEFINE_PROPERTY(uint64_t,m_battleId,BattleId);

	VSType m_type;
	uint32_t m_curTurn;
	int m_userHeroHp;
	int m_heroHp;
	int m_userHeroNuqi;
	int m_heroNuqi;
	boost::posix_time::ptime m_createTime;
	boost::posix_time::ptime m_readyTime;
	boost::posix_time::ptime m_curOptTime;

	std::vector<uint32_t> m_userInputs;
};

class CVsBattleMgr :public boost::serialization::singleton<CVsBattleMgr>
{
public:
	bool Init();
	const VsAward *GetAward(VSType type) const;
	const VsRule *GetRule(uint32_t player1Input, uint32_t player2Input) const;
	uint32_t GetTurnTime(uint32_t turn) const
	{
		if (turn >= m_turnTime.size())
			return 0;
		return m_turnTime[turn];
	}
	uint32_t GetReadyTime() const
	{
		return m_readyTime;
	}
	void Update(boost::posix_time::ptime now);
	CVsBattle *CreateBattle(VSType type, uint32_t userId, CHeroInst *userHero, uint32_t heroId);
	CVsBattle *GetBattle(uint64_t battleId);
private:
	uint32_t GetMask(uint32_t player1Input, uint32_t player2Input) const
	{
		uint32_t mask = (player1Input << 16) | player2Input;
		return mask;
	}
	std::unordered_map<uint64_t,CVsBattle*> m_battles;
	std::unordered_map<uint32_t, VsAward> m_awards;
	std::unordered_map<uint32_t, VsRule> m_vsRules;

	uint32_t m_readyTime = 10000;
	std::vector<uint32_t> m_turnTime;

	uint64_t m_curBattleId = 0;
};