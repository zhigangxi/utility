#pragma once
#include "macros.h"
#include <boost/serialization/singleton.hpp>

class CLearnSKill :public boost::serialization::singleton<CLearnSKill>
{
public:
	DEFINE_PROPERTY(int, m_friendship, Friendship);
	DEFINE_PROPERTY(int, m_critBase, CritBase);
	DEFINE_PROPERTY(int, m_critTimes, CritTimes);
	DEFINE_PROPERTY(int, m_generalSoul, GeneralSoul);
	bool Init();
	int GetSkillExp(int Wisdom,int luck,int &crit) const;
};