#pragma once
#include <stdint.h>
#include <boost/serialization/singleton.hpp>
#include <unordered_map>
#include <vector>
#include "ServerPb.pb.h"

struct CampData
{
	uint32_t campType = 0;
	uint32_t Level = 0;
	uint32_t LevelUpTime = 0;
	uint32_t LevelUpCost = 0;
	uint32_t RequireLevel = 0;
	float OutputPerHour = 0;
	float OutputHourLimit = 0;
};

class CCampMgr :public boost::serialization::singleton<CCampMgr>
{
public:
	bool Init();
	bool InitUserCamp(ServerPB::UserCamp *camp, uint32_t type, uint32_t lv) const;
	const CampData *GetCamp(uint32_t type, uint32_t lv) const;
	uint32_t GetClearCDCost(uint32_t type) const;
private:
	std::unordered_map<uint32_t, std::vector<CampData>> m_otherCamps;//type,camps
	std::unordered_map<uint32_t, uint32_t> m_clearCDCosts;
};