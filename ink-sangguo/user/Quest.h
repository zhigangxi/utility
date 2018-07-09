#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <boost/any.hpp>
#include "NetMsg.pb.h"
#include <list>

struct ConsAgriculture
{
	uint32_t title;
	float selfCityPer;
	uint32_t target;
	uint32_t awardId;
	std::vector<uint32_t> places;
};

struct ConsBusiness
{
	uint32_t title;
	float selfCityPer;
	uint32_t target;
	uint32_t awardId;
	std::vector<uint32_t> places;
};

struct BattleCateran
{
	//ID	Chance	InSelfCityChance	PositionOnRoad	Duration	EnemyTeam	AwardID								
	uint32_t id;
	uint32_t chance;
	float selfCityPer;
	float onRoadBegin;
	float onRoadEnd;
	uint32_t duration;
	uint32_t enemyTeam;
	uint32_t awardId;
};

struct ConsMilitary
{
	uint32_t title;
	float selfCityPer;
	uint32_t target;
	uint32_t awardId;
	std::vector<uint32_t> places;
};

struct QuestVisit
{
	uint32_t title;
	uint32_t inSelfforce;
	uint32_t inEnemyforce;
	uint32_t inOtherforce;
	std::vector<uint32_t> raritys;
	std::vector<uint32_t> raritysChance;
	uint32_t awardId;
	uint32_t place;
};

struct VisitExtra
{
	uint32_t id;
	uint32_t rarity;
	uint32_t chance;
};

struct QuestConscrip
{
	uint32_t title;
	uint32_t cost;
	uint32_t awardId;
	uint32_t place;
	uint32_t target;
};

struct QuestDiscovery
{
	uint32_t Title;
	float ItemChance;
	uint32_t ErrorChance;
	uint32_t NotFindChance;
	uint32_t FindChance;
	uint32_t ErrorAwardID;
	uint32_t NotFindAwardID;
	uint32_t FindAwardID;
	float InCityChance;
	std::vector<uint32_t> Place;
	std::vector<uint32_t> ItemPool;
	std::vector<uint32_t> ItemPoolChance;
	std::vector<uint32_t> HeroPool;
	std::vector<uint32_t> HeroPoolChance;
};

struct DiscoveryItemPool
{
	uint32_t id;
	uint32_t item;
	uint32_t notFindAward;
	uint32_t findAward;
};

struct DiscoveryHeroPool
{
	uint32_t id;
	uint32_t hero;
	uint32_t notFindAward;
	uint32_t findAward;
};

struct DiscoveryHeroName
{
	uint32_t id;
	uint32_t sex;
	std::string name;
};

class CHeroInst;

class CQuest
{
public:
	template<typename Data>
	Data *GetQuestData(uint32_t id)
	{
		auto iter = m_quests.find(id);
		if (iter == m_quests.end())
			return nullptr;
	 	return  boost::any_cast<Data*>(iter->second);
	}
	uint32_t GetEffectNum(CHeroInst *hero);
	void GetHeroAward(CHeroInst *hero,uint32_t effect, NetMsg::DoQuestAward *award);

	template<typename Data>
	void AddQuestData(uint32_t id, Data *data)
	{
		m_quests.insert(std::make_pair(id, boost::any(data)));
	}

	template<typename Data>
	void GetQuestDatas(std::list<Data*> &datas)
	{
		for (auto i = m_quests.begin(); i != m_quests.end(); i++)
		{
			datas.push_back(boost::any_cast<Data*>(i->second));
		}
	}
	bool SetAward(char *type,char *val);

	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_type, Type);
	DEFINE_PROPERTY(uint32_t, m_subType, SubType);
	DEFINE_PROPERTY(uint32_t, m_effect, Effect);
private:
	std::unordered_map<uint32_t, boost::any> m_quests;

	enum DoQuestAwardType {
		INVALID = -1,
		STRENGTH = 0,
		LEADERSHIP,
		WISDOM,
		CHARM,
		LUCK,
		NUM
	};

	struct Award
	{
		int type;
		float val;
	};
	std::vector<Award> m_awrads;
};

class CQuestMgr :public boost::serialization::singleton<CQuestMgr>
{
public:
	bool InitQuest();
	CQuest *GetQuest(uint32_t id) const;
	const std::vector<VisitExtra> &GetVisitExtData() const
	{
		return m_visitExtData;
	}
	uint32_t GetDialogNum(uint32_t questId) const;
	uint32_t GetVisitDiaNum(uint32_t heroClassId) const;
	uint32_t GetDestroyDiaNum(uint32_t id) const;
	std::vector<DiscoveryItemPool*> *GetDisItems()
	{
		return &m_discoveryItems;
	}
	std::vector<DiscoveryHeroPool*> *GetDisHeros()
	{
		return &m_discoveryHeros;
	}
	std::vector<DiscoveryHeroName*> *GetDisHeroName()
	{
		return &m_discoveryHeroNames;
	}
	struct CommonInfo
	{
		int heroCd;
		int cdCostType;
		int cdCost;
		int refreshCostType;
		int refreshCost;
	};
	const CommonInfo &GetCommonInfo() const
	{
		return m_commonInfo;
	}
private:
	bool InitQuestData(NetMsg::QuestType type,CQuest *);
	bool InitConsAgricultureData(CQuest *);
	bool InitConsBusinessData(CQuest *);
	bool InitConsMilitaryData(CQuest *);
	bool InitBattleCateran(CQuest *);
	bool InitQuestVisit(CQuest *);
	bool InitQuestConscrip(CQuest *);
	bool Discovery(CQuest *);
	
	std::unordered_map<uint32_t, CQuest*> m_quests;

	std::vector<VisitExtra> m_visitExtData;

	std::unordered_map<uint32_t, uint32_t> m_questDialogNums;//questId,num
	std::unordered_map<uint32_t, uint32_t> m_visitDialogNums;//heroClass,num

	std::unordered_map<uint32_t, uint32_t> m_destroyDialogNums;

	std::vector<DiscoveryItemPool*> m_discoveryItems;
	std::vector<DiscoveryHeroPool*> m_discoveryHeros;
	std::vector<DiscoveryHeroName*> m_discoveryHeroNames;

	CommonInfo m_commonInfo;
};
