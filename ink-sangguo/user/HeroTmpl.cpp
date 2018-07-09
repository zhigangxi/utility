#include "HeroTmpl.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include "ReadWriteUser.h"
#include "utility.h"
#include <boost/lexical_cast.hpp>
#include "Evolve.h"

void CHeroTmpl::AddSkill(uint32_t id, uint32_t level, std::string condition)
{
	SkillInfo skill;
	skill.skillId = id;
	skill.skillLevel = level;
	skill.type = ConditionNone;

	std::vector<std::string> split;
	boost::split(split, condition, boost::is_any_of(">="), boost::algorithm::token_compress_on);
	if (split.size() > 1)
	{
		if (split[0] == "level")
		{
			skill.type = ConditionLevel;
			skill.requireLevel = boost::lexical_cast<uint32_t>(split[1]);
		}
		else if (split[0] == "plevel")
		{
			skill.type = ConditionPLevel;
			skill.requirePLevel = boost::lexical_cast<uint32_t>(split[1]);
		}
		else if (split[0] == "star")
		{
			skill.type = ConditionStar;
			skill.requirePLevel = boost::lexical_cast<uint32_t>(split[1]);
		}
		m_skills.push_back(skill);
	}
}

CHeroTmpl *CHeroMgr::GetHero(uint32_t id) const
{
	std::unordered_map<uint32_t, CHeroTmpl*>::const_iterator iter = m_heros.find(id);
	if (iter == m_heros.end())
		return nullptr;
	return iter->second;
}

std::vector<uint32_t>* CHeroMgr::GetCanUnlockSkills() const
{
	return nullptr;
}

bool CHeroMgr::InitHero()
{
	if (m_heros.size() > 0)
		return true;

	std::list<CHeroTmpl*> heros;
	CReadWriteUser::ReadHero(heros);

	if (heros.size() <= 0)
		return false;

	auto copySkillId = [](std::vector<CHeroTmpl::SkillInfo> &from, std::vector<uint32_t> &to) {
		for (auto i = from.begin(); i != from.end(); i++)
		{
			if (std::find(to.begin(), to.end(), i->skillId) == to.end())
			{
				to.push_back(i->skillId);
			}
		}
	};

	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		if (m_heroClass.find((*i)->GetClass()) == m_heroClass.end())
		{
			m_heroClass.insert(std::make_pair((*i)->GetClass(), (*i)->GetId()));
		}
		m_heros.insert(std::make_pair((*i)->GetId(), *i));
		auto iter = m_canUnlockSkills.find((*i)->GetClass());
		if (iter == m_canUnlockSkills.end())
		{
			std::vector<uint32_t> skills;
			copySkillId((*i)->GetSkills(), skills);
			m_canUnlockSkills.insert(std::make_pair((*i)->GetClass(), skills));
		}
		else
		{
			copySkillId((*i)->GetSkills(), iter->second);
		}
	}

	return true;
}

uint32_t CHeroMgr::GetHeroId(uint32_t heroClass) const
{
	auto iter = m_heroClass.find(heroClass);
	if (iter != m_heroClass.end())
		return iter->second;
	return 0;
}

bool CHeroMgr::InitOther()
{
	auto &classMgr = CHeroClassMgr::get_const_instance();
	auto &evole = CEvolve::get_const_instance();

	for (auto i = m_heros.begin(); i != m_heros.end(); i++)
	{
		auto heroClass = classMgr.GetHeroClass(i->second->GetClass());
		if (heroClass == nullptr)
			return false;
		i->second->SetForce(heroClass->GetForce());
		i->second->SetSex(heroClass->GetSex());
		if (evole.GetAwakenLevel(i->second->GetId()) == 0)
			m_awakenLevel0.push_back(i->second);
	}
	return true;
}

CHeroClass * CHeroClassMgr::GetHeroClass(uint32_t id) const
{
	auto iter = m_heroClasses.find(id);
	if(iter == m_heroClasses.end())
		return nullptr;
	return iter->second;
}

bool CHeroClassMgr::Init()
{
	std::list<CHeroClass*> heros;
	CReadWriteUser::ReadHeroClass(heros);
	if(heros.size() <= 0)
		return false;

	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		m_heroClasses.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}

void CHeroClassMgr::GetCityHero(uint32_t id, std::vector<uint32_t>& heros) const
{
	for (auto i = m_heroClasses.begin(); i != m_heroClasses.end(); i++)
	{
		CHeroClass *hero = i->second;
		if (hero->GetCity() == id)
			heros.push_back(hero->GetId());
	}
}

void CHeroClassMgr::GetForceHero(uint32_t force, std::vector<uint32_t>& heros) const
{
	for (auto i = m_heroClasses.begin(); i != m_heroClasses.end(); i++)
	{
		CHeroClass *hero = i->second;
		if (hero->GetForce() == force)
			heros.push_back(hero->GetId());
	}
}

void CHeroClassMgr::GetNotForceHero(uint32_t force, std::vector<uint32_t>& heros) const
{
	for (auto i = m_heroClasses.begin(); i != m_heroClasses.end(); i++)
	{
		CHeroClass *hero = i->second;
		if (hero->GetForce() != force)
			heros.push_back(hero->GetId());
	}
}

void CHeroClass::SetProperty(char * exp,char *lv)
{
	char *p[5];
	char *p1[5];
	if (CUtility::SplitLine(p, 5, exp, ',') != 5)
		return;
	if (CUtility::SplitLine(p1, 5, lv, ',') != 5)
		return;
	m_strength.exp = (uint32_t)atoi(p[0]);
	m_leadership.exp = (uint32_t)atoi(p[1]);
	m_wisdom.exp = (uint32_t)atoi(p[2]);
	m_charm.exp = (uint32_t)atoi(p[3]); 
	m_luck.exp = (uint32_t)atoi(p[4]);

	m_strength.lv = (uint32_t)atoi(p1[0]);
	m_leadership.lv= (uint32_t)atoi(p1[1]);
	m_wisdom.lv = (uint32_t)atoi(p1[2]);
	m_charm.lv = (uint32_t)atoi(p1[3]);
	m_luck.lv = (uint32_t)atoi(p1[4]);
}

void CHeroClass::SetCanLearnPro(char * str)
{
	char *p[5];
	if (CUtility::SplitLine(p, 5, str, ',') != 5)
		return;
	for (int i = 0; i < 5; i++)
	{
		m_canLearnPro[i] = boost::lexical_cast<bool>(p[i]);
	}
}

void CHeroClass::SetSkill(char * skillId, char * skillLv, char * skillExp)
{
	char *skillIds[20];
	char *skillLvs[20];
	char *skillExps[20];
	int n1, n2, n3;
	n1 = CUtility::SplitLine(skillIds, 20, skillId, ',');
	n2 = CUtility::SplitLine(skillLvs, 20, skillLv, ',');
	n3 = CUtility::SplitLine(skillExps, 20, skillExp, ',');
	if (n1 != n2 || n1 != n3)
		return;

	for (int i = 0; i < n1; i++)
	{
		SkillHC skill;
		skill.skillId = boost::lexical_cast<uint32_t>(skillIds[i]);
		skill.lv = boost::lexical_cast<uint32_t>(skillLvs[i]);
		skill.exp = boost::lexical_cast<uint32_t>(skillExps[i]);
		m_skills.push_back(skill);
	}
}

void CHeroClass::SetGiftInterest(char * gift, char * interest)
{
	char *ids[100];
	char *interests[100];
	int n1 = CUtility::SplitLine(ids, 100, gift, ',');
	int n2 = CUtility::SplitLine(interests, 100, interest, ',');
	if (n1 != n2)
		return;
	for (int i = 0; i < n1; i++)
	{
		uint32_t id = boost::lexical_cast<uint32_t>(ids[i]);
		float in = boost::lexical_cast<float>(interests[i]);
		m_giftInterests.insert(std::make_pair(id,in));
	}
}
void CHeroClass::GetGiftInterest(uint32_t itemId ,float & persentVal)
{
	auto iter = m_giftInterests.find(itemId);
	if (iter == m_giftInterests.end())
	{
		persentVal = 1.0f;
	}
	else
	{
		persentVal = iter->second;
	}
}
