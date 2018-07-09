#include "HeroInst.h"
#include "SkillInst.h"
#include "SkillTmpl.h"
#include "HeroTmpl.h"
#include "utility.h"

uint64_t CHeroInst::m_curUId = 0;
std::vector<uint32_t> CHeroInst::m_strengthExp;
std::vector<uint32_t> CHeroInst::m_leadershipExp;
std::vector<uint32_t> CHeroInst::m_wisdomExp;
std::vector<uint32_t> CHeroInst::m_charmExp;
std::vector<uint32_t> CHeroInst::m_luckExp;

bool CHeroInst::Init(uint32_t heroId)
{
	CHeroTmpl *tmpl = CHeroMgr::get_const_instance().GetHero(heroId);
	if (tmpl == nullptr)
		return false;

	SetHeroTmpl(tmpl);
	SetHeroId(heroId);
	if(GetId() == 0)
		SetId(GenerateUID());

	return true;
}

bool CHeroInst::AddSkill(CSkillInst & skill)
{
	for (auto i = m_skills.begin(); i != m_skills.end(); i++)
	{
		if (i->GetSkillId() == skill.GetSkillId())
		{
			if (skill.GetLevel() > i->GetLevel())
			{
				i->SetLevel(skill.GetLevel());
				i->SetExp(0);
				return true;
			}
			return false;
		}
	}
	m_skills.push_back(skill);
	return true;
}

CSkillInst * CHeroInst::GetSkill(uint32_t skillId)
{
	for (auto i = m_skills.begin(); i != m_skills.end(); i++)
	{
		if (i->GetSkillId() == skillId)
			return &(*i);
	}
	return nullptr;
}

void CHeroInst::AddExp(int exp)
{
	const uint32_t levelUpExp[] = { 100,110,120,130,140,150,160,170,180,500,550,600,650,700,750,800,850,900,950,1500,1600,1700,1800,1900,2000,2100,2200,2300,2400,5000,5500,6000,6500,7000,7500,8000,8500,9000,9500,15000,16000,17000,18000,19000,20000,21000,22000,23000,24000,50000,55000,60000,65000,70000,75000,80000,85000,90000,95000,150000,160000,170000,180000,190000,200000,210000,220000,230000,240000,500000,550000,600000,650000,700000,750000,800000,850000,900000,950000,1500000,1600000,1700000,1800000,1900000,2000000,2100000,2200000,2300000,2400000,5000000,5500000,6000000,6500000,7000000,7500000,8000000,8500000,9000000,9500000 };
	const uint32_t levelNum = sizeof(levelUpExp) / sizeof(levelUpExp[0]) + 1;
	if (m_level <= 0)
		return;

	int tempExp = m_exp;
	tempExp += exp;
	uint32_t oldLevel = m_level;
	while (m_level < levelNum && tempExp >= levelUpExp[m_level - 1])
	{
		tempExp -= levelUpExp[m_level - 1];
		m_level++;
	}

	SetExp(tempExp);
	if (m_level != oldLevel)
	{
		InitSkill();
	}
}

uint32_t CHeroInst::GetTolStrength()
{
	uint32_t lv = m_strength.lv;
	if (m_heroTmpl != nullptr)
		lv += m_heroTmpl->GetStrength();
	return lv;
}
uint32_t CHeroInst::GetTolLeadership()
{
	uint32_t lv = m_leadership.lv;
	if (m_heroTmpl != nullptr)
		lv += m_heroTmpl->GetLeadership();
	return lv;
}
uint32_t CHeroInst::GetTolWisdom()
{
	uint32_t lv = m_wisdom.lv;
	if (m_heroTmpl != nullptr)
		lv += m_heroTmpl->GetWisdom();
	return lv;
}
uint32_t CHeroInst::GetTolCharm()
{
	uint32_t lv = m_charm.lv;
	if (m_heroTmpl != nullptr)
		lv += m_heroTmpl->GetCharm();
	return lv;
}
uint32_t CHeroInst::GetTolLuck()
{
	uint32_t lv = m_luck.lv;
	if (m_heroTmpl != nullptr)
		lv += m_heroTmpl->GetLuck();
	return lv;
}

void CHeroInst::InitSkill()
{
	if (m_heroTmpl == nullptr)
		return;
	auto skills = m_heroTmpl->GetSkills();
	for (auto i = skills.begin(); i != skills.end(); i++)
	{
		CSkillInst skill;
		switch (i->type)
		{
		case CHeroTmpl::ConditionStar:
			if (m_heroTmpl->GetStar() >= i->requireStar && skill.Init(i->skillId))
			{
				skill.SetLevel(i->skillLevel);
				AddSkill(skill);
			}
			break;
		case CHeroTmpl::ConditionLevel:
			if (m_level >= i->requireLevel && skill.Init(i->skillId))
			{
				skill.SetLevel(i->skillLevel);
				AddSkill(skill);
			}
			break;
		case CHeroTmpl::ConditionPLevel:
			if (m_starLevel >= i->requirePLevel && skill.Init(i->skillId))
			{
				skill.SetLevel(i->skillLevel);
				AddSkill(skill);
			}
			break;
		default:
			break;
		}
	}
}

void CHeroInst::AddStrength(int add)
{
	m_strength.exp += add;
	CUtility::AddExp(m_strengthExp, m_strength.exp, m_strength.lv);
}

void CHeroInst::AddLeadership(int add)
{
	m_leadership.exp += add;
	CUtility::AddExp(m_leadershipExp, m_leadership.exp, m_leadership.lv);
}

void CHeroInst::AddWisdom(int add)
{
	m_wisdom.exp += add;
	CUtility::AddExp(m_wisdomExp, m_wisdom.exp, m_wisdom.lv);
}
void CHeroInst::AddCharm(int add)
{
	m_charm.exp += add;
	CUtility::AddExp(m_charmExp, m_charm.exp, m_charm.lv);
}
void CHeroInst::AddLuck(int add)
{
	m_luck.exp += add;
	CUtility::AddExp(m_luckExp, m_luck.exp, m_luck.lv);
}