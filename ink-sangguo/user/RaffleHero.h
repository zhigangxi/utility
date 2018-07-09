#pragma once
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <vector>

class CUser;

class CRaffleHero :public boost::serialization::singleton<CRaffleHero>
{
public:
	bool Init();
	struct LeftTimes
	{
		int tolTimes;
		int oneRaffTimes;
	};
	LeftTimes GetLeftRaffTimes(CUser *user, uint32_t id) const;
	struct RaffCost
	{
		int priceTye;
		uint32_t price;
	};
	RaffCost GetCost(uint32_t id, uint32_t num) const;
	void RaffleHeros(CUser *user,uint32_t id, uint32_t num,std::vector<uint32_t> &heros) const;
	void AddUseTimes(CUser *user, uint32_t id,uint32_t num) const;
private:
	struct RafflePool
	{
		//ID	GroupID	Rarities	Chances	PriceType	OnePrice	TenPrice
		uint32_t id;
		uint32_t groupId;
		std::vector<uint32_t> rarities;
		std::vector<uint32_t> chances;
		uint32_t priceType;
		uint32_t onePrice;
		uint32_t tenPrice;
	};
	struct RaffleHero
	{
		//GroupID	Rarity	HeroID	Chance
		uint32_t groupId;
		uint32_t rarity;
		uint32_t heroId;
		uint32_t chance;
	};
	std::unordered_map<uint32_t, RafflePool> m_pools;
	std::vector<RaffleHero> m_groupHeros;

	const uint32_t MAX_BUY_TIMES = 50;
	const uint32_t GIVE_FIVE_STAR_HERO_TIMES = 10;
};