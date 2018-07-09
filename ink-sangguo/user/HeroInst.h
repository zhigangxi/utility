#pragma once
#include <stdint.h>
#include "macros.h"
#include <stdint.h>
#include <vector>
#include "SkillInst.h"
#include <time.h>
#include "HeroTmpl.h"

class CHeroInst
{
public:
	CHeroInst() : m_id(0),
		m_heroId(0),
		m_level(1),
		m_starLevel(0),
		m_heroTmpl(nullptr),
		m_updateToDb(false),
		m_cdTime(0),
		m_exp(0)
	{

	}
	bool Init(uint32_t heroId);
	bool AddSkill(CSkillInst &skill);
	CSkillInst *GetSkill(uint32_t skillId);
	void AddExp(int exp);
	uint32_t GetTolStrength();
	uint32_t GetTolLeadership();
	uint32_t GetTolWisdom();
	uint32_t GetTolCharm();
	uint32_t GetTolLuck();

	void InitSkill();

	std::vector<CSkillInst> & GetSkills()
	{
		return m_skills;
	}
	
	void AddStrength(int add);
	void AddLeadership(int add);
	void AddWisdom(int add);
	void AddCharm(int add);
	void AddLuck(int add);

	DEFINE_PROPERTY(uint64_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_heroId, HeroId);
	DEFINE_PROPERTY(uint32_t, m_level, Level);
	DEFINE_PROPERTY(uint32_t, m_starLevel, StarLevel);
	DEFINE_PROPERTY_REF(HeroAttr, m_strength, Strength);
	DEFINE_PROPERTY_REF(HeroAttr, m_leadership, Leadership);
	DEFINE_PROPERTY_REF(HeroAttr, m_wisdom, Wisdom);
	DEFINE_PROPERTY_REF(HeroAttr, m_charm, Charm);
	DEFINE_PROPERTY_REF(HeroAttr, m_luck, Luck);
	DEFINE_PROPERTY(CHeroTmpl*, m_heroTmpl, HeroTmpl);

	DEFINE_PROPERTY(bool, m_updateToDb, UpdateToDb);//update or insert db

	DEFINE_PROPERTY(time_t, m_cdTime, CDTime);
	DEFINE_PROPERTY(time_t, m_learnCdTime, LearnCdTime);
	DEFINE_PROPERTY(uint32_t, m_exp, Exp);

	DEFINE_STATIC_UID(uint64_t, m_curUId);
	
	DEFINE_STATIC_PRO_REF(std::vector<uint32_t> ,m_strengthExp,StrengthExp);
	DEFINE_STATIC_PRO_REF(std::vector<uint32_t> ,m_leadershipExp,LeadershipExp);
	DEFINE_STATIC_PRO_REF(std::vector<uint32_t> ,m_wisdomExp,WisdomExp);
	DEFINE_STATIC_PRO_REF(std::vector<uint32_t> ,m_charmExp,CharmExp);
	DEFINE_STATIC_PRO_REF(std::vector<uint32_t> ,m_luckExp,LuckExp);
private:	
	const static int MAX_SKILL_NUM = 8;

	std::vector<CSkillInst> m_skills;
};
