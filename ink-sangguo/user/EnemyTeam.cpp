#include "EnemyTeam.h"
#include "ReadWriteUser.h"

bool CEnemyMgr::Init()
{
	std::list<CEnemy*> enemys;
	if (!CReadWriteUser::ReadEnemy(enemys))
		return false;

	for (auto i = enemys.begin(); i != enemys.end(); i++)
	{
		m_enemys.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}

CEnemy * CEnemyMgr::GetEnemy(uint32_t id) const
{
	auto iter = m_enemys.find(id);
	if (iter == m_enemys.end())
		return nullptr;
	return iter->second;
}

bool CEnemyTeamMgr::Init()
{
	std::list<CEnemyTeam*> enemyTeams;
	if (!CReadWriteUser::ReadEnemyTeam(enemyTeams))
		return false;

	for (auto i = enemyTeams.begin(); i != enemyTeams.end(); i++)
	{
		m_enemyTeams.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}

CEnemyTeam * CEnemyTeamMgr::GetTeam(uint32_t id) const
{
	auto iter = m_enemyTeams.find(id);
	if (iter == m_enemyTeams.end())
		return nullptr;
	return iter->second;
}
