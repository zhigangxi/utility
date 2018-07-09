#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <list>
#include <vector>

class CWorldCity;

class CForce
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_leader, Leader);
	DEFINE_PROPERTY(uint32_t, m_capital, Capital);
	DEFINE_PROPERTY(bool, m_canChoose, CanChoose);

	void AddCity(CWorldCity *city);

	void AddCooseHeros(uint32_t id)
	{
		m_chooseHeros.push_back(id);
	}
	std::vector<uint32_t> &GetChooseHeros()
	{
		return m_chooseHeros;
	}
	std::list<CWorldCity*> *GetCitys()
	{
		return &m_citys;
	}
private:
	std::list<CWorldCity*> m_citys;
	std::vector<uint32_t> m_chooseHeros;
};

class CForceMgr:public boost::serialization::singleton<CForceMgr>
{
public:
	bool Init();
	CForce *GetForce(uint32_t id) const;
	const std::unordered_map<uint32_t, CForce*> *GetForces() const
	{
		return &m_forces;
	}
private:
	std::unordered_map<uint32_t,CForce*> m_forces;
};