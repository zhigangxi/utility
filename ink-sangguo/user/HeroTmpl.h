#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <vector>

struct HeroAttr
{
	HeroAttr() :exp(0), lv(0)
	{

	}
	uint32_t exp;
	uint32_t lv;
};

class CHeroTmpl
{
public:
	CHeroTmpl() :m_force(0)
	{

	}

	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY(uint32_t, m_soldierType, SoldierType);
	DEFINE_PROPERTY(uint32_t, m_class, Class);
	DEFINE_PROPERTY(std::string, m_title, Title);
	DEFINE_PROPERTY(uint32_t, m_star, Star);
	DEFINE_PROPERTY(uint32_t, m_captainPoint, CaptainPoint);
	DEFINE_PROPERTY(uint32_t, m_strength, Strength);
	DEFINE_PROPERTY(uint32_t, m_leadership, Leadership);
	DEFINE_PROPERTY(uint32_t, m_wisdom, Wisdom);
	DEFINE_PROPERTY(uint32_t, m_charm, Charm);
	DEFINE_PROPERTY(uint32_t, m_luck, Luck);
	DEFINE_PROPERTY(uint32_t, m_vsHealth, VsHealth);
	DEFINE_PROPERTY(uint32_t, m_sex, Sex);

	void AddSkill(uint32_t id, uint32_t level, std::string condition);

	enum ConditionType
	{
		ConditionNone,
		ConditionStar,
		ConditionLevel,
		ConditionPLevel,
	};
	struct SkillInfo
	{
		uint32_t skillId;
		uint32_t skillLevel;

		ConditionType type;
		union
		{
			uint32_t requireStar;
			uint32_t requireLevel;
			uint32_t requirePLevel;
		};
	};
	std::vector<SkillInfo> &GetSkills()
	{
		return m_skills;
	}
private:
	std::vector<SkillInfo> m_skills;
};

class CHeroMgr:public boost::serialization::singleton<CHeroMgr>
{
public:
	CHeroTmpl *GetHero(uint32_t id) const;
	std::vector<uint32_t> *GetCanUnlockSkills() const;
	bool InitHero();
	bool InitOther();//初始化势力和是否是原生卡，必须先初始化CHeroClassMgr,CEvolve
	const std::vector<CHeroTmpl*> *GetAwakenLevel0Hero() const
	{
		return &m_awakenLevel0;
	}
	uint32_t GetHeroId(uint32_t heroClass) const;
private:
	std::vector<CHeroTmpl*> m_awakenLevel0;
	std::unordered_map<uint32_t, uint32_t> m_heroClass;//heroClass,heroId
	std::unordered_map<uint32_t, CHeroTmpl*> m_heros;
	std::unordered_map<uint32_t, std::vector<uint32_t>> m_canUnlockSkills;//class,skills;
};

class CHeroClass
{
public:
	CHeroClass()
	{
		for (int i = 0; i < sizeof(m_canLearnPro) / sizeof(m_canLearnPro[0]); i++)
		{
			m_canLearnPro[i] =  false;
		}
	}
	
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY(uint32_t, m_city, City);
	DEFINE_PROPERTY(uint32_t, m_building, Building);
	DEFINE_PROPERTY(uint32_t, m_baseStar, BaseStar);
	DEFINE_PROPERTY_REF(HeroAttr, m_star, Star);
	DEFINE_PROPERTY_REF(HeroAttr,m_strength, Strength);
	DEFINE_PROPERTY_REF(HeroAttr, m_leadership,Leadership);
	DEFINE_PROPERTY_REF(HeroAttr, m_wisdom,Wisdom);
	DEFINE_PROPERTY_REF(HeroAttr, m_charm,Charm);
	DEFINE_PROPERTY_REF(HeroAttr, m_luck,Luck);
	DEFINE_PROPERTY(uint32_t, m_vsHealth, VsHealth);
	DEFINE_PROPERTY(uint32_t, m_sex, Sex);
	//Strength	Leadership	Wisdom	Charm	Luck		
	void SetProperty(char *exp,char *lv);

	void SetCanLearnPro(char *str);

	void SetSkill(char *skillId, char *skillLv, char *skillExp);

	void SetGiftInterest(char *gift, char *interest);
	void GetGiftInterest(uint32_t itemId, float & persentVal);
	struct SkillHC
	{
		uint32_t skillId;
		uint32_t lv;
		uint32_t exp;
	};
	std::vector<SkillHC> &GetSkills()
	{
		return m_skills;
	}
private:
	std::unordered_map<uint32_t, float> m_giftInterests;
	bool m_canLearnPro[5];
	std::vector<SkillHC> m_skills;
};

class CHeroClassMgr :public boost::serialization::singleton<CHeroClassMgr>
{
public:
	CHeroClass *GetHeroClass(uint32_t id) const;
	bool Init();
	void GetCityHero(uint32_t id, std::vector<uint32_t> &heros) const;
	void GetForceHero(uint32_t force, std::vector<uint32_t> &heros) const;
	void GetNotForceHero(uint32_t force, std::vector<uint32_t> &heros) const;
private:
	std::unordered_map<uint32_t, CHeroClass*> m_heroClasses;
};