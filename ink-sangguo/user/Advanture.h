#pragma once
#include "macros.h"
#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <vector>

class CUser;

class CAdvantureConf :public boost::serialization::singleton<CAdvantureConf>
{
public:
	uint32_t GetAwardGold(CUser *user) const;
	uint32_t GetAwardExp(CUser *user) const;
	DEFINE_PROPERTY(uint32_t, m_awardGold, AwardGold);
	DEFINE_PROPERTY(uint32_t, m_awardExp, AwardExp);
	DEFINE_PROPERTY(uint32_t, m_recoverTime, RecoverTime);
};

class CAdvantureEvent
{
public:
	enum AdvEventType {
		INVALID = -1,
		AWARD = 0,
		CONTINUOUS_BATTLE,
		NUM
	};
	CAdvantureEvent() :m_roadBegin(0), m_roadEnd(1)
	{

	}
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_type, Type);
	DEFINE_PROPERTY(float, m_inSelfCity, InSelfCity);
	DEFINE_PROPERTY(float, m_roadBegin, RoadBegin);
	DEFINE_PROPERTY(float, m_roadEnd, RoadEnd);
	DEFINE_PROPERTY(uint32_t, m_enemyTeam, EnemyTeam);

	DEFINE_PROPERTY(float, m_percent, Percent);
	DEFINE_PROPERTY(uint32_t, m_awardId, AwardId);
	DEFINE_PROPERTY(uint32_t, m_delay, Delay);
};

struct AdEventData
{
	uint32_t id;
	uint32_t awardId;
	uint32_t delay;
	uint32_t crit;
	CAdvantureEvent *event;
};

class CAdvantureMgr :public boost::serialization::singleton<CAdvantureMgr>
{
public:
	bool Init();
	bool GetEvent(CUser *user, AdEventData &event) const;
	const CAdvantureEvent *GetAdvEvent(uint32_t id) const;
private:
	std::vector<CAdvantureEvent*> m_events;
}; 