#include "Advanture.h"
#include "Expressions.h"
#include "user.h"
#include "utility.h"
#include "ReadWriteUser.h"

using namespace std;

uint32_t CAdvantureConf::GetAwardGold(CUser * user) const
{
	AdvantureData data;
	data.Level = user->GetLevel();
	return DesignData::Expressions::sGetInstance().sAdvantureConfigBaseAwardGold(m_awardGold, &data);
}

uint32_t CAdvantureConf::GetAwardExp(CUser * user) const
{
	AdvantureData data;
	data.Level = user->GetLevel();
	return DesignData::Expressions::sGetInstance().sAdvantureConfigBaseAwardExp(m_awardGold, &data);
}

bool CAdvantureMgr::Init()
{
	CAdvantureConf &conf = CAdvantureConf::get_mutable_instance();
	if (!CReadWriteUser::ReadAdvConf(&conf))
		return false;
	
	/*std::list<CAdvantureEvent*> awardGroups;
	CReadWriteUser::ReadAdvEvent(awardGroups);
	if (awardGroups.size() <= 0)
		return false;

	for (auto i = awardGroups.begin(); i != awardGroups.end(); i++)
	{
		m_events.push_back(*i);
	}*/

	list<CReadData> datas;
	std::vector<const char *> fieldNames;
	fieldNames = { "id","type","percent","award_id","delay","in_self_city","pos_on_road","enemy_team"};
	if (!CReadWriteUser::ReadData(datas, fieldNames, "advanture_event"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		CAdvantureEvent *e = new CAdvantureEvent;
		e->SetId(i->GetVal<uint32_t>(0));
		e->SetType(i->GetVal<uint32_t>(1));
		e->SetPercent(i->GetVal<float>(2));
		e->SetAwardId(i->GetVal<uint32_t>(3));
		e->SetDelay(i->GetVal<uint32_t>(4));
		e->SetInSelfCity(i->GetVal<float>(5));
		char *p[2];
		if (CUtility::SplitLine(p, 2, i->GetVal(6), ',') == 2)
		{
			e->SetRoadBegin(atof(p[0]));
			e->SetRoadEnd(atof(p[1]));
		}
		e->SetEnemyTeam(i->GetVal<uint32_t>(7));
		m_events.push_back(e);
	}
	return true;
}

bool CAdvantureMgr::GetEvent(CUser *user, AdEventData &event) const
{
	const int MAX_CHANCE = 1000;
	std::vector<uint32_t> chances;
	
	float crits[] = { 1,0.7,
		2,0.2,
		4,0.1
	};

	chances.clear();
	for (size_t i = 0; i < sizeof(crits) / sizeof(crits[0]) / 2; i++)
	{
		chances.push_back(crits[2 * i + 1] * 1000);
	}
	uint32_t pos = CUtility::RandomChance(chances);
	event.crit = crits[2 * pos];

	NetMsg::AdvantureInfoAck &advInfo = user->GetAdvInfoAck();
	uint32_t doTimes = advInfo.dotimes();
	uint32_t eventId = 0;
	if (doTimes == 1)
		return false;
	else if (doTimes == 2)
		eventId = 101;
	else if (doTimes == 3)
		eventId = 102;
	pos = 0;
	if (eventId != 0)
	{
		for (uint32_t i = 0; i < m_events.size(); i++)
		{
			if (m_events[i]->GetId() == eventId)
			{
				pos = i;
				break;
			}
		}
	}
	else
	{
		chances.clear();
		int intChance = 0;
		for (auto i = m_events.begin(); i != m_events.end(); i++)
		{
			chances.push_back((uint32_t)((*i)->GetPercent() * MAX_CHANCE));
			intChance += (int)((*i)->GetPercent() * MAX_CHANCE);
		}
		if (intChance < MAX_CHANCE)
		{
			if (CUtility::RandomInt(1, MAX_CHANCE) > intChance)
				return false;
		}
		
		pos = CUtility::RandomChance(chances);
	}
		
	event.id = m_events[pos]->GetId();
	event.awardId = m_events[pos]->GetAwardId();
	AdvantureEventData data;
	event.delay = DesignData::Expressions::sGetInstance().sAdvantureEventDelay(m_events[pos]->GetDelay(), &data);
	event.event = m_events[pos];

	return true;
}

const CAdvantureEvent * CAdvantureMgr::GetAdvEvent(uint32_t id) const
{
	for (auto i = m_events.begin(); i != m_events.end(); i++)
	{
		if ((*i)->GetId() == id)
			return *i;
	}
	return nullptr;
}
