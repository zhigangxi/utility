#include "RankReputation.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include <algorithm>
#include <boost/format.hpp>

bool CRankType::Init(uint32_t userID, const char* name, uint32_t force, uint32_t level, uint32_t title, uint32_t reputation, uint32_t military, uint32_t icon)
{

	char buff[32];
	snprintf(buff, sizeof(buff), "%s", name);
	m_playerName	= buff;
	m_userID		= userID;
	m_force			= force;
	m_level			= level;
	m_title			= title;
	m_reputation	= reputation;
	m_military      = military;
	m_icon			= icon;

	return true;
}

bool CRankReputationMgr::Init()
{
	std::list<CRankType*> ranks;
	m_rankreputation.clear();
	if (!CReadWriteUser::ReadRankReputation(ranks))
		return false;

	auto fun = [=](CRankType *rank) {
		m_rankreputation.push_back(rank);
	};
	std::for_each(ranks.begin(), ranks.end(), fun);
	return true;
}

void CRankReputationMgr::Update()
{
	if (this->Init())
	{
	}
}

CRankType * CRankReputationMgr::GetRankByReputation() const
{
	CRankType rank;
	auto more = [](const CRankType *r1, const CRankType *r2) {
		return r1->GetReputation() > r2->GetReputation();
	};
	auto iter = std::lower_bound(m_rankreputation.begin(), m_rankreputation.end(), &rank, more);
	if (iter == m_rankreputation.end())
		return nullptr;
	return *iter;
}

bool CRankLevelMgr::Init()
{
	std::list<CRankType*> ranks;
	m_rankLevel.clear();
	if (!CReadWriteUser::ReadRankLevel(ranks))
		return false;

	auto fun = [=](CRankType *rank) {
		m_rankLevel.push_back(rank);
	};
	std::for_each(ranks.begin(), ranks.end(), fun);
	return true;
}

void CRankLevelMgr::Update()
{
	if(this->Init())
	{
	}
}

