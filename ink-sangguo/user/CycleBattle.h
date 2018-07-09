#pragma once
#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <vector>
#include <unordered_map>

class CUser;

struct CycleBattleData
{
	uint32_t enemyTeamId = 0;
	uint32_t awardId = 0;
};

class CCycleBattle: public boost::serialization::singleton<CCycleBattle>
{
public:
	bool Init();
	CycleBattleData GetData(CUser *user) const;
	uint32_t GetMaxDiff() const;
	uint32_t GetMaxRound(uint32_t diff) const;
private:
	struct CycBatCfg
	{
		uint32_t roundBegin;
		uint32_t roundEnd;
		uint32_t diff;
		std::vector<uint32_t> entmyTeams;
		uint32_t firstTimeAward;
		uint32_t normalAward;
	};
	std::vector<CycBatCfg> m_cycBattCfgs;
	uint32_t m_maxDiff = 0;
	std::unordered_map<uint32_t, uint32_t> m_maxRounds;//diff round
};