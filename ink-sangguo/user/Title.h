#pragma once
#include "macros.h"
#include <stdint.h>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include <unordered_map>

class CTitle
{
public:
	bool Init(uint32_t id, const char *quests, const char *questsChance);
	uint32_t RandQuest();

	void SetAddProperty(char *pro);
	void SetCampaignTime(char *time);
	uint32_t GetAddStrength()
	{
		return m_addProperty[0];
	}
	uint32_t GetAddLeadership()
	{
		return m_addProperty[1];
	}
	uint32_t GetAddWisdom()
	{
		return m_addProperty[2];
	}
	uint32_t GetAddCharm()
	{
		return m_addProperty[3];
	}
	uint32_t GetAddLuck()
	{
		return m_addProperty[4];
	}
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_questNum, QuestNum);
	DEFINE_PROPERTY(int, m_limitNum, LimitNum);
	DEFINE_PROPERTY(uint32_t, m_awardId, AwardId);
	DEFINE_PROPERTY(uint32_t, m_campaignSpace, CampaignSpace);
	DEFINE_PROPERTY(uint32_t, m_campaignHour, CampaignHour);
	DEFINE_PROPERTY(uint32_t, m_campaignMinute, CampaignMinute);
	DEFINE_PROPERTY(std::string, m_name, Name);
	//num_limit, property_add, award_id, campaign_cycle, campaign_time
private:
	//Strength	Leadership	Wisdom	Charm	Luck	
	uint32_t m_addProperty[5];

	struct SelQuest
	{
		uint32_t questId;
		uint32_t questChance;
	};
	uint32_t m_tolChance;
	std::vector<SelQuest> m_questsChance;
};

class CTitleMgr :public boost::serialization::singleton<CTitleMgr>
{
public:
	bool Init();
	CTitle *GetTitle(uint32_t id) const;
	const std::unordered_map<uint32_t, CTitle*> *GetTitles() const
	{
		return &m_titles;
	}
private:
	std::unordered_map<uint32_t, CTitle*> m_titles;
};