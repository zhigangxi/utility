#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include "NetMsg.pb.h"
#include "ServerPb.pb.h"
class CWorldCity
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_wayPoint, WayPoint);
	DEFINE_PROPERTY(uint32_t, m_battleScene, BattleScene);
	DEFINE_PROPERTY(uint32_t, m_governor, Governor);
	DEFINE_PROPERTY(uint32_t, m_agriculture, Agriculture);
	DEFINE_PROPERTY(uint32_t, m_business, Business);
	DEFINE_PROPERTY(uint32_t, m_military, Military);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY_REF(std::vector<uint32_t>, m_heros, Heros);

	DEFINE_PROPERTY(uint32_t, m_CultureDevelopment, CultureDevelopment);
	DEFINE_PROPERTY(uint32_t, m_Money, Money);
	DEFINE_PROPERTY(uint32_t, m_Provisions, Provisions);
	DEFINE_PROPERTY(uint32_t, m_MoneyDelta, MoneyDelta);
	DEFINE_PROPERTY(uint32_t, m_ProvisionsDelta, ProvisionsDelta);
	DEFINE_PROPERTY(uint32_t, m_Soldiers, Soldiers);
	DEFINE_PROPERTY(uint32_t, m_Reserver, Reserver);
	DEFINE_PROPERTY(uint32_t, m_Infantry, Infantry);
	DEFINE_PROPERTY(uint32_t, m_Archer, Archer);
	DEFINE_PROPERTY(uint32_t, m_Cavalry, Cavalry);
	DEFINE_PROPERTY(uint32_t, m_SwordShield, SwordShield);
	DEFINE_PROPERTY(uint32_t, m_CrossbowMan, CrossbowMan);
	DEFINE_PROPERTY(uint32_t, m_Elephant, Elephant);
	DEFINE_PROPERTY(uint32_t, m_InfantryWounded, InfantryWounded);
	DEFINE_PROPERTY(uint32_t, m_ArcherWounded, ArcherWounded);
	DEFINE_PROPERTY(uint32_t, m_CavalryWounded, CavalryWounded);
	DEFINE_PROPERTY(uint32_t, m_SwordShieldWounded, SwordShieldWounded);
	DEFINE_PROPERTY(uint32_t, m_CrossbowManWounded, CrossbowManWounded);
	DEFINE_PROPERTY(uint32_t, m_ElephantWounded, ElephantWounded);
	DEFINE_PROPERTY(uint32_t, m_Population, Population);
	DEFINE_PROPERTY(uint32_t, m_PopulationSpeed, PopulationSpeed);
	DEFINE_PROPERTY(uint32_t, m_Loyalty, Loyalty);
	DEFINE_PROPERTY(uint32_t, m_Durability, Durability);
	DEFINE_PROPERTY_REF(std::vector<uint32_t>, m_Specialties, Specialties);
	struct SCityItem
	{
		SCityItem()
		:itemId(0),
		itemNum(0),
		userId(0),
		timeWhenGet(0)
		{

		}
		uint32_t itemId;
		uint32_t itemNum;
		uint32_t userId;
		uint32_t timeWhenGet;
	};

	struct SCityHero
	{
		SCityHero()
		:heroId(0),
		heroLevel(0)
		{

		}
		uint32_t heroId;
		std::string heroName;
		uint32_t heroLevel;
	};


	void SetCityItemInfo(uint32_t itemId, uint32_t itemNum);
	void AddCityItemInfo(uint32_t itemId, uint32_t itemNum);
	CWorldCity::SCityItem GetCityItem(uint32_t itemId);
	void GetCityItem(NetMsg::CityItemAck * ack);

	//lolo

	void AddCityHero(uint32_t heroId, const std::string & name,uint32_t heroLevel);
	void GetCityHero(NetMsg::CityHeroAck * ack);

	void CopyCityItem(NetMsg::CityInfoAck *outCity);
	void CopyCityHero(NetMsg::CityInfoAck *outCity);

	private:
		std::unordered_map<uint32_t, SCityItem> m_CityItems;
		std::vector<SCityHero> m_CityHeros;
};

class CWorldCityMgr :public boost::serialization::singleton<CWorldCityMgr>
{
public:
	bool Init();
	void Save() const;
	CWorldCity *GetCity(uint32_t id) const;
	uint32_t RandomCityId() const;
private:
	std::unordered_map<uint32_t, CWorldCity*> m_worldCitys;
	std::vector<uint32_t> m_cityIds;
};
