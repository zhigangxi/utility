#include "Shop.h"
#include "ReadWriteUser.h"
#include "utility.h"

bool CShopMgr::Init()
{
	std::list<CShopTmpl*> shops;
	CReadWriteUser::ReadShop(shops);
	if (shops.size() <= 0)
		return false;

	std::list<CShopGoods*> shopGoods;
	CReadWriteUser::ReadShopGoods(shopGoods);
	if (shopGoods.size() <= 0)
		return false;

	for (auto i = shops.begin(); i != shops.end(); i++)
	{
		m_shops.insert(std::make_pair((*i)->GetId(), *i));
	}
	for (auto i = shopGoods.begin(); i != shopGoods.end(); i++)
	{
		auto iter = m_shopGoodsGroup.find((*i)->GetGroupId());
		if (iter == m_shopGoodsGroup.end())
		{
			std::vector<CShopGoods*> group;
			group.push_back(*i);
			m_shopGoodsGroup.insert(std::make_pair((*i)->GetGroupId(), group));
		}
		else
		{
			iter->second.push_back(*i);
		}
	}
	return true;
}

const std::vector<CShopGoods*>* CShopMgr::GetShopGoods(uint32_t groupId) const
{
	auto iter = m_shopGoodsGroup.find(groupId);
	if (iter == m_shopGoodsGroup.end())
		return nullptr;

	return &(iter->second);
}

void CShopMgr::GetGoods(uint32_t groupId, NetMsg::ShopItem * item) const
{
	const std::vector<CShopGoods*> *groups = GetShopGoods(groupId);
	if (groups == nullptr)
		return;

	CShopGoods *g = CUtility::RandomSel(*groups);
	item->set_currencytype(g->GetCurrency());
	item->set_itemid(g->GetItemId());
	item->set_itemnum(g->GetItemNum());
	item->set_price(g->GetPrice());
}

CShopTmpl * CShopMgr::GetShop(uint32_t id) const
{
	auto iter = m_shops.find(id);
	if (iter == m_shops.end())
		return nullptr;
	return iter->second;
}

void CShopTmpl::SetRefreshData(const char *str)
{
	m_refreshData.clear();
	char *p[32];
	int num = CUtility::SplitLine(p, 32,(char*) str, ',');
	for (int i = 0; i < num; i++)
	{
		m_refreshData.push_back(atof(p[i]));
	}
}

void CShopTmpl::AddGroup(uint32_t groupId, uint32_t chance,uint32_t groupMinNum)
{
	m_groupIds.push_back(groupId);
	m_groupChances.push_back(chance);
	m_groupMinNum.push_back(groupMinNum);
}

void CShopTmpl::GetShopItemAck(NetMsg::ShopItemAck * ack)
{
	ack->set_shopid(m_id);
	const CShopMgr &mgr = CShopMgr::get_const_instance();
	for (size_t i = 0; i < m_groupMinNum.size(); i++)
	{
		int num = m_groupMinNum[i];
		if (num > 0)
		{
			for (int j = 0; j < num; j++)
			{
				auto item = ack->add_items();
				item->set_buytimes(m_buyTime);
				mgr.GetGoods(m_groupIds[i],item);
			}
		}
	}

	for (int i = ack->items_size(); i < m_goodsNum; i++)
	{
		uint32_t pos = CUtility::RandomChance(m_groupChances);
		if (pos < m_groupIds.size())
		{
			auto item = ack->add_items();
			item->set_buytimes(m_buyTime);
			mgr.GetGoods(m_groupIds[pos], item);
		}
	}
}
