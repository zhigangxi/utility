#include "Award.h"
#include "ReadWriteUser.h"
#include "Expressions.h"
#include "user.h"
#include "utility.h"

bool CAwardMgr::Init()
{
	std::list<CAward*> awards;
	CReadWriteUser::ReadAwards(awards);
	if (awards.size() <= 0)
		return false;

	std::list<AwardGroup*> awardGroups;
	CReadWriteUser::ReadAwardGroups(awardGroups);
	if (awardGroups.size() <= 0)
		return false;

	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		m_awards.insert(std::make_pair((*i)->GetId(), *i));
	}
	for (auto i = awardGroups.begin(); i != awardGroups.end(); i++)
	{
		auto iter = m_awardGroup.find((*i)->id);
		if (iter == m_awardGroup.end())
		{
			std::vector<AwardGroup*> group;
			group.push_back(*i);
			m_awardGroup.insert(std::make_pair((*i)->id, group));
		}
		else
		{
			iter->second.push_back(*i);
		}
	}
	return true;
}

const std::vector<AwardGroup*>* CAwardMgr::GetAwardGroup(uint32_t groupId) const
{
	auto iter = m_awardGroup.find(groupId);
	if (iter == m_awardGroup.end())
		return nullptr;

	return &(iter->second);
}

CAward * CAwardMgr::GetAward(uint32_t id) const
{
	auto iter = m_awards.find(id);
	if (iter == m_awards.end())
		return nullptr;
	return iter->second;
}

void CAward::InitFixAwards(std::vector<AwardItem> &awards,const AwardData &data)
{
	DesignData::Expressions &exp = DesignData::Expressions::sGetInstance();
	AwardItem item;
	item.type = AwardItemType::EXP;
	item.itemId = 0;
	item.num = exp.sAwardAwardExp(m_exp,&data);
	if (item.num != 0)
		AddAward(item, awards);

	item.type = AwardItemType::EXP_HERO;
	item.itemId = 0;
	item.num = exp.sAwardAwardExpHero(m_expHero, &data);
	if (item.num != 0)
		AddAward(item, awards);
	
	item.type = AwardItemType::COIN;
	item.itemId = 0;
	item.num = exp.sAwardAwardCoin(m_coin, &data);
	if (item.num != 0)
		AddAward(item, awards);

	item.type = AwardItemType::GOLD;
	item.itemId = 0;
	item.num = exp.sAwardAwardGold(m_gold, &data);
	if (item.num != 0)
		AddAward(item, awards);
	
	item.type = AwardItemType::CONTRIBUTION;
	item.itemId = 0;
	item.num = exp.sAwardAwardContribution(m_contribution, &data); 
	if (item.num != 0)
		AddAward(item, awards);
	
	item.type = AwardItemType::REPUTATION;
	item.itemId = 0;
	item.num = exp.sAwardAwardReputation(m_reputation, &data);
	if(item.num != 0)
		AddAward(item, awards);
}
bool CAward::AddAward(int type, uint32_t itemId, uint32_t num)
{
	if (type <= INVALID || type >= NUM)
		return false;

	AwardItem item;
	item.type = (AwardItemType)type;
	item.itemId = itemId;
	item.num = num;
	
	AddAward(item, m_awards);
	return true;
}

void CAward::GenerateAward(std::vector<AwardItem> &awards, const AwardData &data)
{
	for (auto i = m_awards.begin(); i != m_awards.end(); i++)
	{
		AddAward(*i,awards);
	}
	InitFixAwards(awards,data);

	const int MAX_RANDOM = 10000;
	int r = CUtility::RandomInt(1, MAX_RANDOM);
	if (r > m_randomPer*MAX_RANDOM || m_randomGroup == 0 || m_randTimes.size() == 0)
		return;

	std::vector<uint32_t> chances;
	for (auto i = m_randTimes.begin(); i != m_randTimes.end(); i++)
	{
		chances.push_back(MAX_RANDOM*i->timesPer);
	}
	uint32_t pos = CUtility::RandomChance(chances);
	uint32_t times = m_randTimes[pos].times;

	const std::vector<AwardGroup*> *group = CAwardMgr::get_const_instance().GetAwardGroup(m_randomGroup);
	if (group == nullptr)
		return;

	for (uint32_t i = 0; i < times; i++)
	{
		chances.clear();
		for (auto iter = group->begin(); iter != group->end(); iter++)
		{
			chances.push_back((*iter)->percent*MAX_RANDOM);
		}
		pos = CUtility::RandomChance(chances);
		const AwardGroup *ag = (*group)[pos];
		AwardItem award;
		award.type = ag->type;
		award.itemId = ag->itemId;
		award.num = CUtility::RandomInt(ag->minNum, ag->maxNum);
		AddAward(award, awards);
	}
}

void CAward::AddRandTimes(float timesPer, uint32_t times)
{
	RandomTimes rt;
	rt.timesPer = timesPer;
	rt.times = times;
	m_randTimes.push_back(rt);
}

void CAward::AddAward(AwardItem & award, std::vector<AwardItem> &awards)
{
	switch (award.type)
	{
	case HERO:
	case ITEM:
		if (award.itemId == 0 || award.num == 0)
			return;
		break;
	case STRENGTH_HERO:
	case LEADERSHIP_HERO:
	case WISDOM_HERO:
	case CHARM_HERO:
	case LUCK_HERO:
	case EXP_HERO:
	case EXP:
	case COIN:
	case GOLD:
	case CONTRIBUTION:
	case REPUTATION:
	case GENERALSOUL:
	case FRIENDSHIP:
		if (award.num == 0)
			return;
		break;
	}
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		if (CanOverlay(award, *i))
		{
			i->num += award.num;
			return;
		}
	}
	awards.push_back(award);
}

bool CAward::CanOverlay(AwardItem & a1, AwardItem & a2)
{
	if (a1.type != a2.type)
		return false;

	switch (a1.type)
	{
	case HERO:
	case ITEM:
	case EXP_HERO:
	case STRENGTH_HERO:
	case LEADERSHIP_HERO:
	case WISDOM_HERO:
	case CHARM_HERO:
	case LUCK_HERO:
		return a1.itemId == a2.itemId;
	case EXP:
	case COIN:
	case GOLD:
	case CONTRIBUTION:
	case REPUTATION:
	case GENERALSOUL:
	case FRIENDSHIP:
		return true;
	default:
		break;
	}
	return false;
}
