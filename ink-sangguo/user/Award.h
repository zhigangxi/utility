#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include "DataStructs.h"

class CAward
{
public:
	enum AwardItemType {
		INVALID = -1,
		HERO = 0,
		ITEM,
		EXP,
		EXP_HERO,
		STRENGTH_HERO,
		LEADERSHIP_HERO,
		WISDOM_HERO,
		CHARM_HERO,
		LUCK_HERO,
		COIN,
		GOLD,
		CONTRIBUTION,
		REPUTATION,
		FRIENDSHIP,
		GENERALSOUL,
		NUM
	};

	struct AwardItem
	{
		AwardItemType type;
		uint32_t itemId;
		uint32_t num;
	};
	bool AddAward(int type, uint32_t itemId, uint32_t num);
	void GenerateAward(std::vector<AwardItem> &awards, const AwardData &data);

	void AddRandTimes(float timesPer, uint32_t times);
	void InitFixAwards(std::vector<AwardItem> &awards, const AwardData &data);

	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_exp, Exp);
	DEFINE_PROPERTY(uint32_t, m_expHero, ExpHero);
	DEFINE_PROPERTY(uint32_t, m_coin, Coin);
	DEFINE_PROPERTY(uint32_t, m_gold, Gold);
	DEFINE_PROPERTY(uint32_t, m_contribution, Contribution);
	DEFINE_PROPERTY(uint32_t, m_reputation, Reputation);
	DEFINE_PROPERTY(float, m_randomPer, RandomPer);
	DEFINE_PROPERTY(uint32_t, m_randomGroup, RandomGroup);
private:
	void AddAward(AwardItem &award, std::vector<AwardItem> &awards);
	bool CanOverlay(AwardItem &a1, AwardItem &a2);

	std::vector<AwardItem> m_awards;
	struct RandomTimes
	{
		float timesPer;
		uint32_t times;
	};
	std::vector<RandomTimes> m_randTimes;
};

struct AwardGroup
{
	uint32_t id;
	float percent;
	CAward::AwardItemType type;
	uint32_t itemId;
	uint32_t minNum;
	uint32_t maxNum;
};

class CAwardMgr :public boost::serialization::singleton<CAwardMgr>
{
public:
	bool Init();
	const std::vector<AwardGroup*> *GetAwardGroup(uint32_t groupId) const;
	CAward *GetAward(uint32_t id) const;
private:
	std::unordered_map<uint32_t,CAward*> m_awards;
	std::unordered_map<uint32_t, std::vector<AwardGroup*>> m_awardGroup;
};