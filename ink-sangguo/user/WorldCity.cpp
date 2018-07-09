#include "WorldCity.h"
#include "ReadWriteUser.h"
#include "HeroTmpl.h"
#include "utility.h"

void CWorldCity::SetCityItemInfo(uint32_t itemId, uint32_t itemNum)
{
	SCityItem cityItem;
	cityItem.itemId = itemId;
	cityItem.itemNum = itemNum;
	auto iter = m_CityItems.find(itemId);
	if (iter == m_CityItems.end())
	{
		m_CityItems.insert(std::make_pair(itemId, cityItem));
	}
	else
	{
		iter->second = cityItem;
	}
}

void CWorldCity::AddCityItemInfo(uint32_t itemId, uint32_t itemNum)
{
	auto iter = m_CityItems.find(itemId);
	if (iter == m_CityItems.end())
	{
		SCityItem c;
		c.itemId = itemId;
		c.itemNum = itemNum;
		auto r = m_CityItems.insert(std::make_pair(c.itemId, c));
	}
	else
	{
		iter->second.itemNum += itemNum;
	}
}

CWorldCity::SCityItem CWorldCity::GetCityItem(uint32_t itemId)
{
	auto iter = m_CityItems.find(itemId);
	if (iter == m_CityItems.end())
	{
		return SCityItem();
	}
	else
	{
		return iter->second;
	}
}

void CWorldCity::GetCityItem(NetMsg::CityItemAck * ack)
{
	for (auto i = m_CityItems.begin(); i != m_CityItems.end(); i++)
	{
		NetMsg::CityItemInfo *info = ack->add_cityitem();
		info->set_itemid(i->second.itemId);
		info->set_itemnum(i->second.itemNum);
	}
}

void CWorldCity::AddCityHero(uint32_t heroId, const std::string & name, uint32_t heroLevel)
{
	SCityHero cityHero;
	///std::shared_ptr<SCityHero> cityHero (new SCityHero);
	cityHero.heroId = heroId;
	cityHero.heroName = name;
	cityHero.heroLevel = heroLevel;
	m_CityHeros.push_back(cityHero);
}
void CWorldCity::GetCityHero(NetMsg::CityHeroAck * ack)
{
	for (auto i = m_CityHeros.begin(); i != m_CityHeros.end(); i++)
	{
		SCityHero cityHero = *i;
		NetMsg::CityHeroInfo *info = ack->add_cityhero();
		info->set_heroid(cityHero.heroId);
		info->set_heroname(cityHero.heroName);
		info->set_herolevel(cityHero.heroLevel);
	}
}

void CWorldCity::CopyCityItem(NetMsg::CityInfoAck *outCity)
{
	for(auto i = m_CityItems.begin(); i != m_CityItems.end(); i++)
	{
		SCityItem *item = &(i->second);
		if(item)
		{
			outCity->add_iteminfo()->set_itemid(item->itemId);
			outCity->add_iteminfo()->set_itemnum(item->itemNum);
		}
	}
}
void CWorldCity::CopyCityHero(NetMsg::CityInfoAck *outCity)
{
	for (auto i = m_CityHeros.begin(); i != m_CityHeros.end(); i++)
	{
		SCityHero *hero = &(*i);
		if (hero)
		{
			outCity->add_heroinfo()->set_heroid(hero->heroId);
			outCity->add_heroinfo()->set_heroname(hero->heroName.c_str());
			outCity->add_heroinfo()->set_herolevel(hero->heroLevel);
		}
	}
}


CWorldCity * CWorldCityMgr::GetCity(uint32_t id) const
{
	auto iter = m_worldCitys.find(id);
	if (iter == m_worldCitys.end())
		return nullptr;

	return iter->second;
}

uint32_t CWorldCityMgr::RandomCityId() const
{
	return CUtility::RandomSel(m_cityIds);
}


bool CWorldCityMgr::Init()
{
	if (!CHeroClassMgr::get_mutable_instance().Init())
		return false;

	std::list<CWorldCity*> citys;
	CReadWriteUser::ReadWorldCity(citys);
	if (citys.size() <= 0)
		return false;

	CWorldCity *city;
	const CHeroClassMgr &herosMgr = CHeroClassMgr::get_const_instance();
	for (auto i = citys.begin(); i != citys.end(); i++)
	{
		city = *i;
		std::vector<uint32_t> &heros = city->GetHeros();
		herosMgr.GetCityHero(city->GetId(),heros);
		m_worldCitys.insert(std::make_pair((*i)->GetId(), *i));
		m_cityIds.push_back(city->GetId());
	}
	
	return true;
}

void CWorldCityMgr::Save() const
{
	for (auto i = m_worldCitys.begin(); i != m_worldCitys.end(); i++)
	{
		CReadWriteUser::UpdateWorldCity(i->second);
	}
}
