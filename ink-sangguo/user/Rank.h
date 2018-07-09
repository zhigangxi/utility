#pragma once
#include "macros.h"
#include <stdint.h>
#include <list>
#include <boost/serialization/singleton.hpp>
#include <vector>

class CRank
{
public:
	bool Init(uint32_t level, uint32_t exp, char *titles);
	uint32_t GetDefaultTitle();
	bool HaveTitle(uint32_t title);
	std::list<uint32_t> *GetTitles()
	{
		return &m_titles;
	}
	DEFINE_PROPERTY(uint32_t, m_level, Level);
	DEFINE_PROPERTY(uint32_t, m_exp, Exp);
private:
	std::list<uint32_t> m_titles;
};

class CRankMgr :public boost::serialization::singleton<CRankMgr>
{
public:
	bool Init();
	CRank *GetRankByLevel(uint32_t level) const;
	CRank *GetRankByExp(uint32_t exp) const;
	const std::vector<CRank*> *GetRanks() const
	{
		return &m_ranks;
	}
private:
	std::vector<CRank*> m_ranks;
};