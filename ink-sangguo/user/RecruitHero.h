#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>

struct RecruitInfo
{
	uint32_t id;
	uint32_t heroId;
	uint32_t contribution;
	uint32_t city;
	uint32_t priceType;
	uint32_t price;
};

class CRecruitHero:public boost::serialization::singleton<CRecruitHero>
{
public:
	bool Init();
	const RecruitInfo *GetRecruitInfo(uint32_t id) const;
private:
	std::unordered_map<uint32_t, RecruitInfo> m_recruitInfos;
};