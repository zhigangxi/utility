#include "Rank.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include <algorithm>

bool CRank::Init(uint32_t level, uint32_t exp, char * titles)
{
	const int MAX_SPLIT_NUM = 20;
	char *titleId[MAX_SPLIT_NUM];
	int num = CUtility::SplitLine(titleId, MAX_SPLIT_NUM, (char*)titles, ',');

	if (num <= 0)
		return false;

	m_titles.clear();
	m_level = level;
	m_exp = exp;
	for (int i = 0; i < num; i++)
	{
		m_titles.push_back((uint32_t)atoi(titleId[i]));
	}

	return true;
}

uint32_t CRank::GetDefaultTitle()
{
	if (m_titles.size() <= 0)
		return 0;
	return *(m_titles.begin());
}

bool CRank::HaveTitle(uint32_t title)
{
	for (auto i = m_titles.begin(); i != m_titles.end(); i++)
	{
		if (*i == title)
			return true;
	}
	return false;
}

bool CRankMgr::Init()
{
	std::list<CRank*> ranks;
	if (!CReadWriteUser::ReadRanks(ranks))
		return false;

	auto fun = [=](CRank *rank) {
		m_ranks.push_back(rank);
	};
	std::for_each(ranks.begin(), ranks.end(), fun);

	auto comp = [](const CRank *r1, const CRank *r2) {
		return r1->GetLevel() < r2->GetLevel();
	};
	std::sort(m_ranks.begin(), m_ranks.end(), comp);
	return true;
}

CRank * CRankMgr::GetRankByLevel(uint32_t level) const
{
	CRank rank;
	rank.SetLevel(level);
	auto less = [](const CRank *r1, const CRank *r2) {
		return r1->GetLevel() < r2->GetLevel();
	};
	auto iter = std::lower_bound(m_ranks.begin(), m_ranks.end(),&rank , less);
	if (iter == m_ranks.end())
		return nullptr;
	return *iter;
}

CRank * CRankMgr::GetRankByExp(uint32_t exp) const
{
	CRank rank;
	rank.SetExp(exp);
	auto less = [](const CRank *r1, const CRank *r2) {
		return r1->GetExp() < r2->GetExp();
	};
	auto iter = std::lower_bound(m_ranks.begin(), m_ranks.end(), &rank, less);
	if (iter == m_ranks.end())
		return nullptr;
	return *iter;
}
