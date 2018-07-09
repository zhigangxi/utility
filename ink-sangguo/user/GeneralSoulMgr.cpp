#include "GeneralSoulMgr.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include <algorithm>
#include <boost/format.hpp>
#include "HeroInst.h"

bool CGeneralSoulMgr::Init()
{
	std::list<GeneralSoulCost*> costs;
	m_exchangeGeneralSoulCost.clear();
	if(!CReadWriteUser::ReadExchangeGeneralSoulCost(costs))
		return false;

	for (auto i = costs.begin(); i != costs.end(); i++)
	{
		GeneralSoulCost soulCost;
		soulCost.star = (*i)->star;
		soulCost.cost = (*i)->cost;
		m_exchangeGeneralSoulCost.insert(std::make_pair(soulCost.star, soulCost));
	}

	return true;
}

int CGeneralSoulMgr::GetStarByHeroClass(uint32_t heroClass)const
{
	auto &classMgr = CHeroClassMgr::get_const_instance();
	auto hero = classMgr.GetHeroClass(heroClass);
	int starNum = hero->GetBaseStar();
	return starNum;
}

bool CGeneralSoulMgr::GeneralSoulExchangeHeroSuccess(uint32_t heroClass, CUser::GeneralSoul & soul) const
{
	bool isSuccess = false;
	int starNum = GetStarByHeroClass(heroClass);

	for (auto i = m_exchangeGeneralSoulCost.begin(); i != m_exchangeGeneralSoulCost.end(); i++)
	{
		auto iter = m_exchangeGeneralSoulCost.find(starNum);
		if (iter == m_exchangeGeneralSoulCost.end())
		{
			return false;
		}
		else
		{
			GeneralSoulCost val = iter->second;
			isSuccess = ((soul.num) >= val.cost) ? true : false;
		}
	}
	return isSuccess;
}

void CGeneralSoulMgr::doExchangeSoul(CUser*user, uint32_t heroClass)const
{
	int starNum = GetStarByHeroClass(heroClass);
	auto iter = m_exchangeGeneralSoulCost.find(starNum);
	if (iter == m_exchangeGeneralSoulCost.end())
	{
		return ;
	}
	else
	{
		GeneralSoulCost val = iter->second;
		int num = val.cost;
		user->AddHeroGeneralSoul(heroClass, -num);
	}
}

