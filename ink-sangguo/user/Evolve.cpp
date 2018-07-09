#include "Evolve.h"
#include "user.h"
#include "HeroInst.h"
#include "ReadWriteUser.h"
#include "utility.h"
#include "HeroTmpl.h"
#include "ItemInst.h"
#include <iostream>

bool CEvolve::Init()
{
	std::list<CEvolveCost*> costs;
	CReadWriteUser::ReadEvolveCost(costs);
	if (costs.size() <= 0)
		return false;

	CEvolveCost *cost;
	for (auto i = costs.begin(); i != costs.end(); i++)
	{
		cost = *i;
		uint32_t mask = GetMask(cost->GetStar(), cost->GetAwakenLevel(), cost->GetPromotionLevel());
		//std::cout << cost->GetStar() << "," << cost->GetAwakenLevel() << "," << cost->GetPromotionLevel() << std::endl;
		m_costs.insert(std::make_pair(mask, cost));
	}

	std::list<std::pair<uint32_t, uint32_t>> fromTos;
	CReadWriteUser::ReadEvolve(fromTos);

	if (fromTos.size() <= 0)
		return false;

	for (auto i = fromTos.begin(); i != fromTos.end(); i++)
	{
		m_fromTos.insert(std::make_pair(i->first, i->second));
		m_toFroms.insert(std::make_pair(i->second, i->first));
	}
	return true;
}

int CEvolve::EvolveHero(CUser * user, uint64_t heroId, uint64_t eatHeroId,Cost &tolCost) const
{
	CHeroInst *hero = user->GetHero(heroId);
	CHeroInst *eatHero = user->GetHero(eatHeroId);
	if (hero == nullptr || eatHero == nullptr)
		return 3;
	CHeroTmpl *tmpl = hero->GetHeroTmpl();
	
	if (tmpl == nullptr)
		return 3;
	tolCost.useHero = eatHero;
	tolCost.hero = hero;

	uint32_t heroTmplId = hero->GetHeroId();
	uint32_t eatTmplId = heroTmplId;
	
	for (int i = 0; i < 10; i++)
	{
		heroTmplId = GetFromHeroId(heroTmplId);
		if (heroTmplId != 0)
			eatTmplId = heroTmplId;
		else
			break;
	}
	if (eatTmplId != eatHero->GetHeroId())
		return 3;

	uint32_t awakenLv = GetAwakenLevel(hero);
	uint32_t promotionLv = hero->GetStarLevel();
	uint32_t star = tmpl->GetStar();

	uint32_t tmplId = tmpl->GetId();
	uint32_t toTmplId = GetToHeroId(tmplId);
	uint32_t toToTempId = GetToHeroId(toTmplId);
	bool isTop = (toToTempId == 0);
	if (toTmplId == 0)
		return 3;

	CEvolveCost *cost;
	const int MAX_PLV = 9;
	int coin = 0;
	for (int i = 0; i <= eatHero->GetStarLevel(); i++)
	{
		if (promotionLv + 1 > MAX_PLV)
		{
			star++;
			cost = GetCost(star, awakenLv + 1, 0);
			promotionLv = 0;
			if (cost != nullptr)
			{
				std::vector<CEvolveCost::NeedMaterial> *materials = cost->GetMaterials();
				for (auto i = materials->begin(); i != materials->end(); i++)
				{
					CItemInst *item = user->GetItemByTmpl(i->material);
					if (item == nullptr || item->GetNum() < i->num)
						return 2;
					tolCost.items.push_back(item);
					tolCost.useNums.push_back(i->num);
				}
				coin += cost->GetCoin();
				tmplId = toTmplId;
			}
			else
			{
				break;
			}
			if (isTop)
				break;
		}
		else
		{
			cost = GetCost(star, awakenLv, promotionLv + 1);
			if (cost == nullptr)
				break;
			coin += cost->GetCoin();
			promotionLv++;
		}
	}
	
	if (user->GetCoin() < coin)
		return 1;

	tolCost.coin = coin;
	if (tmplId != tmpl->GetId())
	{
		hero->Init(tmplId);
	}
	hero->SetStarLevel(promotionLv);
	hero->InitSkill();
	return 0;
}

uint32_t CEvolve::GetAwakenLevel(uint32_t heroId) const
{
	uint32_t retVal = 0;
	uint32_t fromId = 0;
	for (int i = 0; i < 10; i++)
	{
		fromId = GetFromHeroId(heroId);
		if (fromId == 0)
			break;
		heroId = fromId;
		retVal++;
	}
	return retVal;
}

uint32_t CEvolve::GetAwakenLevel(CHeroInst * hero) const
{
	CHeroTmpl *tmpl = hero->GetHeroTmpl();
	if (tmpl == nullptr)
		return 0;

	return GetAwakenLevel(tmpl->GetId());
}

CEvolveCost * CEvolve::GetCost(uint32_t star, uint32_t AwakenLevel, uint32_t PromotionLevel) const
{
	uint32_t mask = GetMask(star, AwakenLevel, PromotionLevel);
	auto iter = m_costs.find(mask);
	if (iter == m_costs.end())
		return nullptr;
	return iter->second;
}

uint32_t CEvolve::GetMask(uint32_t star, uint32_t AwakenLevel, uint32_t PromotionLevel) const
{
	return uint32_t((star<<16)|(AwakenLevel<<8)|(PromotionLevel));
}

uint32_t CEvolve::GetToHeroId(uint32_t from) const
{
	auto iter = m_fromTos.find(from);
	if (iter == m_fromTos.end())
		return 0;
	return iter->second;
}

uint32_t CEvolve::GetFromHeroId(uint32_t to) const
{
	auto iter = m_toFroms.find(to);
	if (iter == m_toFroms.end())
		return 0;
	return iter->second;
}

void CEvolveCost::SetMaterials(char * materials, char * num)
{
	char *mats[20];
	char *nums[20];
	int n = CUtility::SplitLine(mats, 20, materials, ',');
	int n1 = CUtility::SplitLine(nums, 20, num, ',');
	if (n != n1 || num <= 0)
		return;

	for (int i = 0; i < n; i++)
	{
		NeedMaterial need;
		need.material = atoi(mats[i]);
		need.num = atoi(nums[i]);
		m_materials.push_back(need);
	}
}
