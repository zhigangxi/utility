#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <list>
#include <boost/serialization/singleton.hpp>
#include <vector>

class CRankType
{
public:
	bool Init(uint32_t userID, const char* name, uint32_t force,uint32_t level,uint32_t title,uint32_t reputation, uint32_t military, uint32_t icon);
	
	DEFINE_PROPERTY_REF(std::string, m_playerName, PlayerName);
	DEFINE_PROPERTY(uint32_t, m_userID, UserID);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY(uint32_t, m_level, Level);
	DEFINE_PROPERTY(uint32_t, m_title, Title);
	DEFINE_PROPERTY(uint32_t, m_reputation, Reputation);
	DEFINE_PROPERTY(uint32_t, m_military, Military);
	DEFINE_PROPERTY(uint32_t, m_icon, Icon);
private:

};

class CRankReputationMgr :public boost::serialization::singleton<CRankReputationMgr>
{
public:
	bool Init();
	CRankType *GetRankByReputation() const;
	const std::vector<CRankType*> *GetRankReputation() const
	{
		return &m_rankreputation;
	}
	void Update();
private:
	std::vector<CRankType*> m_rankreputation;
};


class CRankLevelMgr :public boost::serialization::singleton<CRankLevelMgr>
{
public:
	bool Init();
	const std::vector<CRankType*> *GetRankLevel() const
	{
		return &m_rankLevel;
	}
	void Update();
private:
	std::vector<CRankType*> m_rankLevel;
};



