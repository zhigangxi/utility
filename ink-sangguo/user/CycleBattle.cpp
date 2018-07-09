#include "CycleBattle.h"
#include "ReadWriteUser.h"
#include <list>
#include "utility.h"
#include "user.h"

using namespace std;

bool CCycleBattle::Init()
{
	list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "round","difficulty","enemy_teams","first_award","normal_award" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "cycle_battle"))
		return false;

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		CycBatCfg data;
		char *p[20];
		if (CUtility::SplitLine(p, 2, i->GetVal(0), ',') != 2)
			return false;
		data.roundBegin = atoi(p[0]);
		data.roundEnd = atoi(p[1]);
		data.diff = i->GetVal<uint32_t>(1);
		if (data.diff > m_maxDiff)
			m_maxDiff = data.diff;

		int num = CUtility::SplitLine(p, 20, i->GetVal(2), ',');
		if (num <= 0)
			return false;
		for (int j = 0; j < num; j++)
		{
			data.entmyTeams.push_back(atoi(p[j]));
		}
		data.firstTimeAward = i->GetVal<uint32_t>(3);
		data.normalAward = i->GetVal<uint32_t>(4);
		m_cycBattCfgs.push_back(data);

		auto iter = m_maxRounds.find(data.diff);
		if (iter == m_maxRounds.end())
			m_maxRounds.insert(std::make_pair(data.diff, data.roundEnd));
		else if (iter->second < data.roundEnd)
			iter->second = data.roundEnd;
	}

	return true;
}

CycleBattleData CCycleBattle::GetData(CUser * user) const
{
	CycleBattleData data;
	auto &battle = user->GetCycleBattle();
	for (auto i = m_cycBattCfgs.begin(); i != m_cycBattCfgs.end(); i++)
	{
		if (battle.curdifficulty() == i->diff && battle.curround() >= i->roundBegin && battle.curround() <= i->roundEnd)
		{
			if (battle.curround() == 1)
				data.awardId = i->firstTimeAward;
			else
				data.awardId = i->normalAward;
			data.enemyTeamId = CUtility::RandomSel(i->entmyTeams);
		}
	}
	return data;
}

uint32_t CCycleBattle::GetMaxDiff() const
{
	return m_maxDiff;
}

uint32_t CCycleBattle::GetMaxRound(uint32_t diff) const
{
	auto iter = m_maxRounds.find(diff);
	if (iter != m_maxRounds.end())
		return iter->second;
	return 0;
}

