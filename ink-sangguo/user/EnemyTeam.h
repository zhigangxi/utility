#pragma once
#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <unordered_map>
#include "macros.h"
#include <vector>

class CEnemy
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY(uint32_t, m_sex, Sex);
	DEFINE_PROPERTY(uint32_t, m_soldierType, SoldierType);
	DEFINE_PROPERTY(uint32_t, m_strength, Strength);
	DEFINE_PROPERTY(uint32_t, m_leadership, Leadership);
	DEFINE_PROPERTY(uint32_t, m_wisdom, Wisdom);
	DEFINE_PROPERTY(uint32_t, m_charm, Charm);
	DEFINE_PROPERTY(uint32_t, m_luck, Luck);
	DEFINE_PROPERTY(uint32_t, m_vsHealth, VsHealth);
};

class CEnemyMgr:public boost::serialization::singleton<CEnemyMgr>
{
public:
	bool Init();
	CEnemy *GetEnemy(uint32_t id) const;
	const std::unordered_map<uint32_t, CEnemy*> *GetEnemys() const
	{
		return &m_enemys;
	}
private:
	std::unordered_map<uint32_t, CEnemy*> m_enemys;
};

struct EnemyInfo
{
	uint32_t star;
	uint32_t promotion;
	uint32_t formation;
	uint32_t conditionPos;
	int beginLevel;
	int endLevel;
	std::vector<uint32_t> skills;
	std::vector<uint32_t> skillLevels;
};

class CEnemyTeam
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_awardId, AwardId);
	DEFINE_PROPERTY(uint32_t, m_cost, Cost);
	std::vector<EnemyInfo> *GetEnemys()
	{
		return &m_enemys;
	}
private:
	std::vector<EnemyInfo> m_enemys;
};

class CEnemyTeamMgr :public boost::serialization::singleton<CEnemyTeamMgr>
{
public:
	bool Init();
	CEnemyTeam *GetTeam(uint32_t id) const;
private:
	std::unordered_map<uint32_t, CEnemyTeam*> m_enemyTeams;
};