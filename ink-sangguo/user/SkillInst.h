#pragma once
#include "macros.h"
#include <stdint.h>

class CSkillTmpl;

class CSkillInst
{
public:
	CSkillInst() :m_id(0),m_exp(0),m_level(0), m_skillTmpl(nullptr)
	{

	}
	bool Init(uint32_t skillId);
	void AddExp(uint32_t exp);

	DEFINE_PROPERTY(uint64_t, m_id,Id);
	DEFINE_PROPERTY(uint32_t, m_skillId, SkillId);
	DEFINE_PROPERTY(uint32_t, m_exp, Exp);
	DEFINE_PROPERTY(uint32_t, m_level, Level);

	DEFINE_PROPERTY(CSkillTmpl*, m_skillTmpl, SkillTmpl);

	DEFINE_STATIC_UID(uint64_t, m_curUId);
};