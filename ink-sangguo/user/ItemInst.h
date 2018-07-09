#pragma once
#include "macros.h"
#include <stdint.h>

class CItemTmpl;

class CItemInst
{
public:
	CItemInst() :m_id(0),m_updatToDb(false)
	{

	}
	bool Init(uint32_t itemId,uint32_t num);
	DEFINE_PROPERTY(uint64_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_itemId, ItemId);
	DEFINE_PROPERTY(uint32_t, m_num, Num);
	DEFINE_PROPERTY(CItemTmpl*, m_tmpl, ItemTmpl);
	DEFINE_PROPERTY(bool, m_updatToDb, UpdateToDb);

	DEFINE_STATIC_UID(uint64_t, m_curUId);
};