#include "LearnSkill.h"
#include "Expressions.h"
#include "ReadWriteUser.h"
#include "utility.h"

bool CLearnSKill::Init()
{
	if (!CReadWriteUser::ReadLearnSkill(m_friendship, m_critBase, m_critTimes, m_generalSoul))
		return false;
	return true;
}

int CLearnSKill::GetSkillExp(int Wisdom,int luck,int &crit) const
{
	LearnSkillData data;
	data.Wisdom = Wisdom;
	data.Luck = luck;

	int critPer = m_critBase;
	int critAdd = (int)DesignData::Expressions::sGetInstance().sHeroLearnSkillLearnSkillCritAdd(0, &data);
	if (critAdd > 0)
		critPer += critAdd;

	if (CUtility::RandomInt(1, 100) <= critPer)
		crit = m_critTimes;

	return (int)DesignData::Expressions::sGetInstance().sHeroLearnSkillLearnSkillExp(0, &data);
}
