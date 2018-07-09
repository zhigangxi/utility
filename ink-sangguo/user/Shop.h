#pragma once
#include "macros.h"
#include <stdint.h>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include <unordered_map>
#include "NetMsg.pb.h"

class CShopTmpl
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_refreshType, RefreshType);
	DEFINE_PROPERTY(uint32_t, m_buyTime, BuyTime);
	DEFINE_PROPERTY(bool, m_canRefresh, CanRefresh);
	DEFINE_PROPERTY(uint32_t, m_currency, Currency);
	DEFINE_PROPERTY(uint32_t, m_refreshPrice, RefreshPrice);
	DEFINE_PROPERTY(uint32_t, m_refreshItem, RefreshItem);
	DEFINE_PROPERTY(uint32_t, m_goodsNum, GoodsNum);
	void SetRefreshData(const char *);
	void AddGroup(uint32_t groupId, uint32_t chance,uint32_t groupMinNum);
	void GetShopItemAck(NetMsg::ShopItemAck *ack);
private:
	std::vector<float> m_refreshData;
	std::vector<uint32_t> m_groupIds;
	std::vector<uint32_t> m_groupChances;
	std::vector<uint32_t> m_groupMinNum;
};

class CShopGoods
{
public:
	DEFINE_PROPERTY(uint32_t, m_groupId, GroupId);
	DEFINE_PROPERTY(uint32_t, m_itemId, ItemId);
	DEFINE_PROPERTY(uint32_t, m_itemNum, ItemNum);
	DEFINE_PROPERTY(int, m_currency, Currency);
	DEFINE_PROPERTY(int, m_price, Price);
};

class CShopMgr :public boost::serialization::singleton<CShopMgr>
{
public:
	bool Init();
	const std::vector<CShopGoods*> *GetShopGoods(uint32_t groupId) const;
	void GetGoods(uint32_t groupId, NetMsg::ShopItem *item) const;
	CShopTmpl *GetShop(uint32_t id) const;
private:
	std::unordered_map<uint32_t, CShopTmpl*> m_shops;
	std::unordered_map<uint32_t, std::vector<CShopGoods*>> m_shopGoodsGroup;
};