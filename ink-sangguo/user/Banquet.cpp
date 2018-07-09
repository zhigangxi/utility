#include "Banquet.h"
#include "ReadWriteUser.h"
#include "utility.h"
#include "HeroTmpl.h"
#include "user.h"
#include "WorldCity.h"
#include "Force.h"

bool CBanquet::Init()
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "id","force","rank","hero_num","price_type","price","discount","guest_of_honour","five_star_num","five_star_chance"
		 ,"four_star_chance","three_star_chance","self_force","dura_time" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "banquet"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		BanquetInfo *info = new BanquetInfo;
		info->id = i->GetVal<uint32_t>(0);
		info->force = i->GetVal<uint32_t>(1);
		info->rank = i->GetVal<uint32_t>(2);
		info->heroNum = i->GetVal<uint32_t>(3);
		info->priceType = i->GetVal<uint32_t>(4);
		info->price = i->GetVal<uint32_t>(5);
		info->discount = i->GetVal<float>(6);
		info->guestOfHonour = i->GetVal<uint32_t>(7);
		char *p[20];
		int num = CUtility::SplitLine(p, 20, i->GetVal(8), ',');
		for (int i = 0; i < num; i++)
		{
			info->fiveStarNum.push_back(atoi(p[i]));
		}
		if (num != CUtility::SplitLine(p, 20, i->GetVal(9), ','))
		{
			return false;
		}
		for (int i = 0; i < num; i++)
		{
			info->fiveStarChance.push_back(atoi(p[i]));
		}
		info->fourStarChance = i->GetVal<uint32_t>(10);
		info->threeStarChance = i->GetVal<uint32_t>(11);
		info->selfForce = i->GetVal<float>(12);
		info->duraTime = i->GetVal<uint32_t>(13);
		uint32_t mask = CUtility::MakeMask(info->force, info->rank);
		m_banquetInfos.insert(std::make_pair(mask, info));
		m_banquetInfosById.insert(std::make_pair(info->id, info));
	}
	return true;
}

bool CBanquet::Create(CUser *user) 
{
	uint32_t mask = CUtility::MakeMask(user->GetForce(), user->GetRank());
	auto iter = m_banquetInfos.find(mask);
	if (iter == m_banquetInfos.end())
		return false;

	auto &addData = user->GetAddData();
	auto banInfo = addData.mutable_banquetinfos();
	uint32_t id = 0;
	for (auto i = banInfo->begin(); i != banInfo->end(); i++)
	{
		if (i->mutable_info()->uid() > id)
			id = i->mutable_info()->uid();
	}
	id++;
	auto userBanInfo = banInfo->Add();
	auto banHero = userBanInfo->mutable_info();
	banHero->set_uid(id);
	
	auto info = iter->second;
	std::vector<uint32_t> heros;
	SelectHeros(user, info, heros);
	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		banHero->add_heros(*i);
		banHero->add_banquet(false);
	}

	banHero->set_banquetid(info->id);
	banHero->set_leftfreetimes(1);
	banHero->set_lefttime(info->duraTime);

	userBanInfo->mutable_info()->CopyFrom(*banHero);
	userBanInfo->set_createtime(time(nullptr));
	userBanInfo->set_duratime(info->duraTime);

	return true;
}

bool CBanquet::GetPrice(uint32_t id, uint32_t & priceType, uint32_t & price, float &discount) const
{
	auto iter = m_banquetInfosById.find(id);
	if(iter == m_banquetInfosById.end())
		return false;
	priceType = iter->second->priceType;
	price = iter->second->price;
	discount = iter->second->discount;
	return true;
}

void CBanquet::SelectHeros(CUser * user, BanquetInfo *info, std::vector<uint32_t>& heros)
{
	uint32_t fiveStarNum = 0;
	SelectedHeros selHeros;
	selHeros.config = info;
	GetSelectedHeros(user, selHeros);

	uint32_t firstHero = SelFirstHero(user, selHeros);
	int tolNum = info->heroNum;
	if (firstHero != 0)
	{
		heros.push_back(firstHero);
		auto hero = CHeroMgr::get_const_instance().GetHero(firstHero);
		if (hero != nullptr && hero->GetStar() == 5)
			fiveStarNum++;
		else
			tolNum--;
	}
	
	DelSelHero(user, firstHero, selHeros);

	uint32_t pos = CUtility::RandomChance(info->fiveStarChance);
	uint32_t num = info->fiveStarNum[pos];
	uint32_t heroId;
	if (num > fiveStarNum)
	{
		for (int i = 0; i < num - fiveStarNum; i++)
		{
			heroId = SelFiveStar(user, selHeros);
			DelSelHero(user, heroId, selHeros);
			heros.push_back(heroId);
		}
		fiveStarNum = num;
	}

	int toleChance = info->fourStarChance + info->threeStarChance;
	int r;
	for (int i = 0; i < tolNum - fiveStarNum; i++)
	{
		r = CUtility::RandomInt(1, toleChance);
		if (r <= info->fourStarChance)
		{
			heroId = SelFourStar(user, selHeros);
		}
		else
		{
			heroId = SelThreeStar(user, selHeros);
		}
		DelSelHero(user, heroId, selHeros);
		heros.push_back(heroId);
	}
}

uint32_t CBanquet::SelFirstHero(CUser *user, SelectedHeros &selHeros)
{
	auto &addData = user->GetAddData();
	BanquetInfo *info = selHeros.config;

	uint32_t firstHero = 0;
	if (info->guestOfHonour == FIRST_HERO)
	{
		firstHero = addData.selhero();
	}
	else if (info->guestOfHonour == GOVERNOR)
	{
		auto city = CWorldCityMgr::get_const_instance().GetCity(user->GetCampCity());
		if (city != nullptr)
			firstHero = CHeroMgr::get_const_instance().GetHeroId(city->GetGovernor());
	}
	else if (info->guestOfHonour == LEADER)
	{
		auto force = CForceMgr::get_const_instance().GetForce(user->GetForce());
		if (force != nullptr)
			firstHero = CHeroMgr::get_const_instance().GetHeroId(force->GetLeader());
	}
	else if (info->guestOfHonour == FIVE_STAR)
	{
		if(selHeros.selfFiveStar.size() > 0)
			return CUtility::RandomSel(selHeros.selfFiveStar);
	}
	return firstHero;
}

void CBanquet::GetSelectedHeros(CUser *user, SelectedHeros &selHeros)
{
	auto heros = CHeroMgr::get_const_instance().GetAwakenLevel0Hero();
	std::vector<uint32_t> *fiveStar;
	std::vector<uint32_t> *fourStar;
	std::vector<uint32_t> *threeStar;
	for (auto i = heros->begin(); i != heros->end(); i++)
	{
		auto hero = *i;
		if (hero->GetForce() == user->GetForce())
		{
			fiveStar = &(selHeros.selfFiveStar);
			fourStar = &(selHeros.selfFourStar);
			threeStar = &(selHeros.selfThreeStar);
		}
		else
		{
			fiveStar = &(selHeros.otherFiveStar);
			fourStar = &(selHeros.otherFourStar);
			threeStar = &(selHeros.otherThreeStar);
		}
		if (hero->GetStar() == 5)
			fiveStar->push_back(hero->GetId());
		else if(hero->GetStar() == 4)
			fourStar->push_back(hero->GetId());
		else if(hero->GetStar() == 3)
			threeStar->push_back(hero->GetId());
	}
}

bool CBanquet::DelSelfHero(CUser *user, uint32_t heroId,SelectedHeros &selHeros)
{
	auto delStarFun = [](std::vector<uint32_t> &heros, uint32_t delId) {
		for (auto i = heros.begin(); i != heros.end(); i++)
		{
			if (*i == delId)
			{
				heros.erase(i);
				return true;
			}
		}
		return false;
	};
	if (delStarFun(selHeros.selfFiveStar, heroId))
		return true;
	if (delStarFun(selHeros.selfFourStar, heroId))
		return true;
	if (delStarFun(selHeros.selfThreeStar, heroId))
		return true;
	return false;
}

bool CBanquet::DelOtherHero(CUser *user, uint32_t heroId,SelectedHeros &selHeros)
{
	auto delStarFun = [](std::vector<uint32_t> &heros, uint32_t delId) {
		for (auto i = heros.begin(); i != heros.end(); i++)
		{
			if (*i == delId)
			{
				heros.erase(i);
				return true;
			}
		}
		return false;
	};
	if (delStarFun(selHeros.otherFiveStar, heroId))
		return true;
	if (delStarFun(selHeros.otherFourStar, heroId))
		return true;
	if (delStarFun(selHeros.otherThreeStar, heroId))
		return true;
	return false;
}

bool InChance(float chance)
{
	return CUtility::RandomInt(1, 100) <= chance*100;
}

uint32_t CBanquet::SelFiveStar(CUser *user, SelectedHeros &selHeros)
{
	if (InChance(selHeros.config->selfForce) && selHeros.selfFiveStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.selfFiveStar);
	}
	else if(selHeros.otherFiveStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.otherFiveStar);
	}
	return 0;
}

uint32_t CBanquet::SelFourStar(CUser *user, SelectedHeros &selHeros)
{
	if (InChance(selHeros.config->selfForce) && selHeros.selfFourStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.selfFourStar);
	}
	else if (selHeros.otherFourStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.otherFourStar);
	}
	return 0;
}

uint32_t CBanquet::SelThreeStar(CUser *user, SelectedHeros &selHeros)
{
	if (InChance(selHeros.config->selfForce) && selHeros.selfThreeStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.selfThreeStar);
	}
	else if (selHeros.otherThreeStar.size() > 0)
	{
		return CUtility::RandomSel(selHeros.otherThreeStar);
	}
	return 0;
}