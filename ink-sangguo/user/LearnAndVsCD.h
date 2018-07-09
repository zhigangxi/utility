#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <list>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <unordered_map>

class CLearnAndVsCD
{
	 public:
	CLearnAndVsCD()
	:level(0),
	 exp(0),
	 teacher_learn_cd(0),
	 student_learn_cd(0)
	{

	}
	uint32_t level;
	uint32_t exp;
	uint32_t teacher_learn_cd;
	uint32_t student_learn_cd;
};


class CLearnAndVsCDMgr :public boost::serialization::singleton<CLearnAndVsCDMgr>
{
public:
	bool Init();
	bool GetCDinfo(uint32_t friendshipLevel, CLearnAndVsCD & cdinfo) const;
	uint32_t GetDataSize()const
	{	
		return m_mapsize;
	}
private:
	std::unordered_map<uint32_t,CLearnAndVsCD*> m_learnAndVsCD;
	uint32_t m_mapsize;
};