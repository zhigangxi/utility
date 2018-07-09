#include "LearnProperty.h"
#include "Expressions.h"
#include "ReadWriteUser.h"
#include "utility.h"

bool CLearnProperty::Init()
{
	std::list<LearnPropertyInfo*> infos;
	if (!CReadWriteUser::ReadLearnPropeyty(infos))
		return false;

	m_propertys = infos;
	return true;
}

bool CLearnProperty::GetExpFri(int diff, int property,int luck,int & exp, int & friendship,int &crit, int &generalSoul) const
{
	LearnPropertyInfo *p;
	for (auto i = m_propertys.begin(); i != m_propertys.end(); i++)
	{
		p = *i;
		if (diff >= p->diffBegin && diff <= p->diffEnd)
		{
			LearnPropertyData data;
			data.Luck = luck;
			data.Property = property;
			exp = (int)DesignData::Expressions::sGetInstance().sHeroLearnPropertyLearnPropertyExp(p->exp, &data);
			friendship = p->friendship;
			generalSoul = p->generalSoul;

			int critPer = p->critBase;
			int critAdd = (int)DesignData::Expressions::sGetInstance().sHeroLearnPropertyLearnPropertyCritAdd(p->critAdd, &data);
			if (critAdd > 0)
				critPer += critAdd;
			if (CUtility::RandomInt(1, 100) <= critPer)
				crit = p->critTimes;
			else
				crit = 1;
			return true;
		}
	}
	return false;
}
