#include "ReadWriteUser.h"
#include "SkillTmpl.h"

CSkillTmpl *CSkillMgr::GetSkill(uint32_t id) const
{
	std::unordered_map<uint32_t, CSkillTmpl*>::const_iterator iter = m_skills.find(id);
	if (iter == m_skills.end())
		return nullptr;
	return iter->second;
}

bool CSkillMgr::InitSkill()
{
	if (m_skills.size() > 0)
		return true;

	std::list<CSkillTmpl*> skills;
	CReadWriteUser::ReadSkill(skills);

	if (skills.size() <= 0)
		return false;

	for (auto i = skills.begin(); i != skills.end(); i++)
	{
		m_skills.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}