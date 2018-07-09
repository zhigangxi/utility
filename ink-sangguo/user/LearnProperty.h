#pragma once

#include <boost/serialization/singleton.hpp>
#include <list>

struct LearnPropertyInfo
{
	int diffBegin;
	int diffEnd;
	int exp;
	int friendship;
	int critBase;
	int	critAdd;
	int critTimes;
	int generalSoul;
};

class CLearnProperty:public boost::serialization::singleton<CLearnProperty>
{
public:
	bool Init();
	bool GetExpFri(int diff,int property,int luck, int &exp, int &friendship,int &crit,int &generalSoul) const;
private:
	std::list<LearnPropertyInfo*> m_propertys;
};