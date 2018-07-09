#include "UserBattle.h"
#include "utility.h"
#include "EnemyTeam.h"
#include <boost/random.hpp>

uint64_t CUserBattle::m_uId;

bool CUserBattle::Init(uint32_t type,uint32_t teamId)
{
	if(m_id == 0)
		m_id = GenerateUID();
	m_type = type;
	m_battleData.set_battleid(m_id);
	m_battleData.set_teamid(teamId);
	return true;
}

bool CUserBattle::Init(uint32_t type, uint64_t typeData, const char * data)
{
	if (data == nullptr)
		return false;

	if (!CUtility::StrToMsg((char*)data, &m_battleData))
		return false;

	m_updateToDb = true;
	m_type = type;
	m_typeData = typeData;
	m_id = m_battleData.battleid();
	return true;
}

void CUserBattle::AddEnemy(uint32_t id,uint32_t level, EnemyInfo & info)
{
	NetMsg::BattleEnemy *enemy = m_battleData.add_enemys();
	enemy->set_id(id);
	InitEnemy(level, &info, enemy);
}

void CUserBattle::InitEnemy(uint32_t userLevel, EnemyInfo * info, NetMsg::BattleEnemy * enemy)
{
	/*boost::uniform_int<> distribution(info->beginLevel, info->endLevel);
	boost::kreutzer1986 engine;
	boost::variate_generator<boost::kreutzer1986, boost::uniform_int<> > myrandom(engine, distribution);
	myrandom();*/
	int addLevel = CUtility::RandomInt(info->beginLevel, info->endLevel);
	int level = userLevel + addLevel;
	if (level <= 0)
		level = 1;
	enemy->set_level(level);
	enemy->set_formation(info->formation);
	enemy->set_promotion(info->promotion);
	enemy->set_star(info->star);
	for (auto i = info->skills.begin(); i != info->skills.end(); i++)
	{
		enemy->add_skill(*i);
	}
	for (auto i = info->skillLevels.begin(); i != info->skillLevels.end(); i++)
	{
		enemy->add_skilllevel(*i);
	}
}

void CUserBattle::GetDataStr(std::string & str)
{
	CUtility::MsgToStr(&m_battleData, str);
}

