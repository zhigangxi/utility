#pragma once
#include "macros.h"
#include "NetMsg.pb.h"
#include <stdint.h>

struct EnemyInfo;
class CUserBattle
{
public:
	CUserBattle() :m_id(0), m_type(0),m_typeData(0),m_updateToDb(false), m_delBattle(false),
		m_awardId(0)
	{

	}
	enum BattleType
	{
		QuestBattle = 1,
		CycBattle = 2,
		AdvEventBattle = 3,
		BiographyBattle = 4,
		WorldWarBattle = 5
	};
	bool Init(uint32_t type,uint32_t teamId);
	bool Init(uint32_t type,uint64_t typeData, const char *data);
	void AddEnemy(uint32_t enemyId,uint32_t userLevel, EnemyInfo &info);
	void SetTeamId(uint32_t teamId)
	{
		m_battleData.set_teamid(teamId);
	}

	void InitEnemy(uint32_t userLevel, EnemyInfo *info, NetMsg::BattleEnemy *enemy);

	void GetDataStr(std::string &str);

	DEFINE_PROPERTY(uint64_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_type, Type);
	DEFINE_PROPERTY(uint64_t, m_typeData, TypeData);
	DEFINE_PROPERTY_REF(NetMsg::BattleAck, m_battleData, BattleData);
	DEFINE_STATIC_UID(uint64_t, m_uId);

	DEFINE_PROPERTY(bool, m_updateToDb, UpdateToDb);
	DEFINE_PROPERTY(bool, m_delBattle, DelBattle);

	DEFINE_PROPERTY(uint32_t, m_awardId, AwardId);
};