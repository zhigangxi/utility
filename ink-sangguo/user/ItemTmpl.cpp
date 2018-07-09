#include "ItemTmpl.h"
#include "ReadWriteUser.h"

CItemTmpl *CItemMgr::GetItem(uint32_t id) const
{
	std::unordered_map<uint32_t, CItemTmpl*>::const_iterator iter = m_items.find(id);
	if (iter == m_items.end())
		return nullptr;
	return iter->second;
}

bool CItemMgr::InitItem()
{
	if (m_items.size() > 0)
		return true;

	std::list<CItemTmpl*> items;
	CReadWriteUser::ReadItem(items);

	if (items.size() <= 0)
		return false;

	for (auto i = items.begin(); i != items.end(); i++)
	{
		m_items.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}


uint32_t CItemMgr::GetItemBySkillIdAndLevel(uint32_t skillId, uint32_t skillLevel)const
{
	uint32_t id = -1;
	for(auto i = m_items.begin(); i != m_items.end(); i++)
	{
		CItemTmpl * item = i->second;
		if(skillId == item->GetExtId() && CItemTmpl::ITEM_BOOK_SKILL == item->GetType() && skillLevel == item->GetExtVal2())
		{
			id = item->GetId();
			return id;
		}
	}
	return id;
}
