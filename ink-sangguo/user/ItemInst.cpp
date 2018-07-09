#include "ItemInst.h"
#include "ItemTmpl.h"

uint64_t CItemInst::m_curUId;

bool CItemInst::Init(uint32_t itemId,uint32_t num)
{
	if (num <= 0)
		return false;

	CItemTmpl *tmpl = CItemMgr::get_const_instance().GetItem(itemId);
	if (tmpl == nullptr)
		return false;
	m_tmpl = tmpl;
	SetItemId(itemId);
	SetNum(num);
	if (GetId() == 0)
		SetId(GenerateUID());
	return true;
}
