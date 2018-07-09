#pragma once
#include "macros.h"
#include <stdint.h>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include <unordered_map>

class CHeroInst;
class CUser;
class CItemInst;

class CEvolveCost
{
public:
	//Star	AwakenLevel	PromotionLevel	Materials	NumMaterials	Coin
	DEFINE_PROPERTY(uint32_t, m_star, Star);
	DEFINE_PROPERTY(uint32_t, m_awakenLevel, AwakenLevel);
	DEFINE_PROPERTY(uint32_t, m_promotionLevel, PromotionLevel);
	DEFINE_PROPERTY(uint32_t, m_coin, Coin);
	void SetMaterials(char *materials, char *num);
	struct NeedMaterial
	{
		uint32_t material;
		uint32_t num;
	};
	std::vector<NeedMaterial> *GetMaterials()
	{
		return &m_materials;
	}
private:
	std::vector<NeedMaterial> m_materials;
};

class CEvolve :public boost::serialization::singleton<CEvolve>
{
public:
	bool Init();

	struct Cost
	{
		int coin = 0;
		CHeroInst *useHero = nullptr;
		CHeroInst *hero = nullptr;
		std::vector<CItemInst*> items;
		std::vector<uint32_t> useNums;
	};
	//0成功，1 NoMoney，2 NoMaterial,3 EvolveError
	int EvolveHero(CUser *user, uint64_t heroId, uint64_t eatHeroId, Cost &cost) const;
	uint32_t GetAwakenLevel(uint32_t heroId) const;
private:
	uint32_t GetAwakenLevel(CHeroInst *hero) const;
	CEvolveCost *GetCost(uint32_t star, uint32_t AwakenLevel, uint32_t PromotionLevel) const;
	uint32_t GetMask(uint32_t star, uint32_t AwakenLevel,uint32_t PromotionLevel) const;//star<<16|AwakenLevel<<8|PromotionLevel
	uint32_t GetToHeroId(uint32_t from) const;
	uint32_t GetFromHeroId(uint32_t to) const;

	std::unordered_map<uint32_t, uint32_t> m_fromTos;//fromid,toid
	std::unordered_map<uint32_t, uint32_t> m_toFroms;//toid,fromid

	std::unordered_map<uint32_t, CEvolveCost*> m_costs;//mask,CEvolveCost*
};