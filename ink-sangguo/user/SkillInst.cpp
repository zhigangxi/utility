#include "SkillInst.h"
#include "SkillTmpl.h"

uint64_t CSkillInst::m_curUId = 0;

bool CSkillInst::Init(uint32_t heroId)
{
	if (m_skillTmpl != nullptr)
		return true;

	CSkillTmpl *tmpl = CSkillMgr::get_const_instance().GetSkill(heroId);
	if (tmpl == nullptr)
		return false;

	SetSkillTmpl(tmpl);
	SetSkillId(heroId);
	if(GetId() == 0)
		SetId(GenerateUID());
	return true;
}

void CSkillInst::AddExp(uint32_t exp)
{
	if (m_skillTmpl == nullptr)
		return;
	const int MAX_SKILL_LEVEL = 5;
	if (m_level >= MAX_SKILL_LEVEL)
		return;

	m_exp += exp;
	auto addSkillExp = [=](const int exp[]) {
		while (m_level < MAX_SKILL_LEVEL && m_exp >= exp[m_level])
		{
			m_exp -= exp[m_level];
			m_level++;
		}
	};
	
	switch (m_skillTmpl->GetLevelUpId())
	{
	case 1:
	{
		const int exps[] = { 1, 20, 500, 15000, 600000 };
		addSkillExp(exps);
		break;
	}
	case 2:
	{
		const int exps[] = { 1, 16, 400, 12000, 480000 };
		addSkillExp(exps);
		break;
	}
	case 3:
	{
		const int exps[] = { 1, 10, 250, 7500, 300000 };
		addSkillExp(exps);
		break;
	}
	default:
		break;
	}
}
