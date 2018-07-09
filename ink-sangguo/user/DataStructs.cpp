#include "DataStructs.h"
#include "EnemyTeam.h"
#include "utility.h"

bool EnemyFilterData::id(int id)  const
{
	return m_enemy->GetId() == id;
}

bool EnemyFilterData::soldier(int sol) const
{
	return m_enemy->GetSoldierType() == sol;
}

bool EnemyFilterData::force(int force) const
{
	return m_enemy->GetForce() == force;
}

int AwardData::Random(int min, int max) const
{
	return CUtility::RandomInt(min,max);
}

int AdvantureEventData::Random(int min, int max) const
{
	return CUtility::RandomInt(min, max);
}
