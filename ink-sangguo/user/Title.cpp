#include "Title.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>

bool CTitle::Init(uint32_t id, const char * quests, const char * questsChance)
{
	m_id = id;
	m_questsChance.clear();
	m_tolChance = 0;

	const int MAX_SPLIT_NUM = 40;
	char *questId[MAX_SPLIT_NUM];
	int num = CUtility::SplitLine(questId, MAX_SPLIT_NUM, (char*)quests, ',');

	if (num <= 0)
		return false;

	char *questsIdCh[MAX_SPLIT_NUM];
	if (CUtility::SplitLine(questsIdCh, MAX_SPLIT_NUM, (char*)questsChance, ',') != num)
		return false;

	SelQuest selQue;
	for (int i = 0; i < num; i++)
	{
		selQue.questId = (uint32_t)atoi(questId[i]);
		m_tolChance += (uint32_t)atoi(questsIdCh[i]);
		selQue.questChance = m_tolChance;
		m_questsChance.push_back(selQue);
	}
	return true;
}

uint32_t CTitle::RandQuest()
{
	if (m_questsChance.size() <= 0)
		return 0;

	uint32_t r = CUtility::RandomInt(1, m_tolChance);
	uint32_t pos = m_questsChance.size() - 1;
	for (int i = 0; i < m_questsChance.size(); i++)
	{
		if (r <= m_questsChance[i].questChance)
		{
			pos = i;
			break;
		}
	}
	return m_questsChance[pos].questId;
}

void CTitle::SetAddProperty(char * pro)
{
	char *p[5];
	if (CUtility::SplitLine(p, 5, pro, ',') != 5)
		return;
	for (int i = 0; i < 5; i++)
	{
		m_addProperty[i] = atoi(p[i]);
	}
}

void CTitle::SetCampaignTime(char * time)
{
	char *p[2];
	if (CUtility::SplitLine(p, 2, time, ':') < 2)
		return;
	m_campaignHour = boost::lexical_cast<uint32_t>(p[0]);
	m_campaignMinute = boost::lexical_cast<uint32_t>(p[1]);
}

bool CTitleMgr::Init()
{
	std::list<CTitle*> titles;
	if (!CReadWriteUser::ReadTitles(titles))
		return false;

	auto fun = [=](CTitle *title) {
		m_titles.insert(std::make_pair(title->GetId(),title));
	};
	std::for_each(titles.begin(), titles.end(), fun);
	return true;
}

CTitle * CTitleMgr::GetTitle(uint32_t id) const
{
	auto iter = m_titles.find(id);
	if(iter == m_titles.end())
		return nullptr;
	return iter->second;
}
