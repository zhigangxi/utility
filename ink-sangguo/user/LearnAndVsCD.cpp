#include "LearnAndVsCD.h"
#include "Expressions.h"
#include "ReadWriteUser.h"
#include "utility.h"

bool CLearnAndVsCDMgr::Init()
{
	std::list<CLearnAndVsCD*> infos;
	if (!CReadWriteUser::ReadHeroFriendshipLevelUp(infos))
		return false;
	
	m_mapsize = infos.size();
	CLearnAndVsCD *cd;
	for(auto i=infos.begin();i!=infos.end();i++)
	{
		cd = *i;
		m_learnAndVsCD.insert(std::make_pair(cd->level, cd));
	}

	return true;
}

bool CLearnAndVsCDMgr::GetCDinfo(uint32_t friendshipLevel, CLearnAndVsCD & cdinfo) const
{
	auto iter = m_learnAndVsCD.find(friendshipLevel);
	if(iter == m_learnAndVsCD.end())
	{
		return false;
	}
	else
	{
		CLearnAndVsCD *cd = iter->second;
		cdinfo.exp = cd->exp;
		cdinfo.level = cd->level;
		cdinfo.teacher_learn_cd = cd->teacher_learn_cd;
		cdinfo.student_learn_cd = cd->student_learn_cd;
	}
	return true;
}

