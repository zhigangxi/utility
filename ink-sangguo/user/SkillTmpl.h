#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <vector>

class CSkillTmpl
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_star, Star);
	DEFINE_PROPERTY(bool, m_isExclusive, IsExclusive);
	DEFINE_PROPERTY(bool, m_isBattleSkill, IsBattleSkill);
	DEFINE_PROPERTY(uint32_t, m_soldierType, SoldierType);
	DEFINE_PROPERTY(std::string, m_animName, AnimName);
	DEFINE_PROPERTY(uint32_t, m_levelUpId, LevelUpId);
	DEFINE_PROPERTY(std::string, m_skillParam1, SkillParam1);
	DEFINE_PROPERTY(std::string, m_skillParam2, SkillParam2);
	DEFINE_PROPERTY(std::string, m_skillParam3, SkillParam3);
	DEFINE_PROPERTY(std::string, m_skillParam4, SkillParam4);
};

class CSkillMgr :public boost::serialization::singleton<CSkillMgr>
{
public:
	CSkillTmpl *GetSkill(uint32_t id) const;
	bool InitSkill();
private:
	std::unordered_map<uint32_t, CSkillTmpl*> m_skills;
};