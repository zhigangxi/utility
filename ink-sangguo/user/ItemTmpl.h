#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>

class CItemTmpl
{
public:
	enum ItemType {
		INVALID = -1,
		ITEM_MATERIAL = 0,
		ITEM_BOOK_EXP,
		ITEM_BOOK_SKILL,
		ITEM_GIFT,
		ITEM_VISIT,
		ITEM_BOX,
		ITEM_QUEST,
		ITEM_SHOP_REFRESH,
		ITEM_EMPTY_BOOK_SKILL,
		NUM
	};

	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_type, Type);
	DEFINE_PROPERTY(uint32_t, m_star, Star);
	DEFINE_PROPERTY(uint32_t, m_extId, ExtId);
	DEFINE_PROPERTY(uint32_t, m_extVal, ExtVal);
	DEFINE_PROPERTY(uint32_t, m_extVal2, ExtVal2);
};

class CItemMgr :public boost::serialization::singleton<CItemMgr>
{
public:
	CItemTmpl *GetItem(uint32_t id) const;
	bool InitItem();
	uint32_t GetItemBySkillIdAndLevel(uint32_t skillId, uint32_t skillLevel) const;
private:
	std::unordered_map<uint32_t, CItemTmpl*> m_items;
};
