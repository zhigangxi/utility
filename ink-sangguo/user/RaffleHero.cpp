#include "RaffleHero.h"
#include "ReadWriteUser.h"
#include "utility.h"
#include "user.h"

bool CRaffleHero::Init()
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "id","groupid","rarities","chances","price_type","one_price","ten_price" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "feast_pool"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		RafflePool pool;
		pool.id = i->GetVal<uint32_t>(0);
		pool.groupId = i->GetVal<uint32_t>(1);
		char *p[20];
		int num = CUtility::SplitLine(p, 20, i->GetVal(2), ',');
		for (int i = 0; i < num; i++)
		{
			pool.rarities.push_back(atoi(p[i]));
		}
		if (num != CUtility::SplitLine(p, 20, i->GetVal(3), ','))
			return false;
		for (int i = 0; i < num; i++)
		{
			pool.chances.push_back(atoi(p[i]));
		}
		pool.priceType = i->GetVal<uint32_t>(4);
		pool.onePrice = i->GetVal<uint32_t>(5);
		pool.tenPrice = i->GetVal<uint32_t>(6);
		m_pools.insert(std::make_pair(pool.id,pool));
	}
	
	fieldNames = { "groupid","rarity","hero_id","chance"};
	datas.clear();
	if (!CReadWriteUser::ReadData(datas, fieldNames, "feast_hero"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		RaffleHero rh;
		rh.groupId = i->GetVal<uint32_t>(0);
		rh.rarity = i->GetVal<uint32_t>(1);
		rh.heroId = i->GetVal<uint32_t>(2);
		rh.chance = i->GetVal<uint32_t>(3);
		m_groupHeros.push_back(rh);
	}
	return true;
}

CRaffleHero::LeftTimes CRaffleHero::GetLeftRaffTimes(CUser * user, uint32_t id) const
{
	auto &addData = user->GetAddData();	
	auto raffTimes = addData.mutable_rafftimes();
	time_t now = time(nullptr);
	tm *nowTm = localtime(&now);
	int day = nowTm->tm_yday + 1;
	LeftTimes times = { MAX_BUY_TIMES ,GIVE_FIVE_STAR_HERO_TIMES - 1 };
	if (addData.raffday() != day)
	{
		addData.set_raffday(day);
		for (auto i = raffTimes->begin(); i != raffTimes->end(); i++)
		{
			i->set_usetimes(0);
		}
		return times;
	}
	
	for (auto i = raffTimes->begin(); i != raffTimes->end(); i++)
	{
		if (i->id() == id)
		{
			times.tolTimes -= i->usetimes();
			times.oneRaffTimes -= i->onerafftimes() % 10;
			return times;
		}
	}
	return times;
}

CRaffleHero::RaffCost CRaffleHero::GetCost(uint32_t id, uint32_t num) const
{
	RaffCost cost;
	cost.price = 0;
	cost.priceTye = -1;
	auto iter = m_pools.find(id);
	if (iter == m_pools.end())
		return cost;
	if (num == 1)
		cost.price = iter->second.onePrice;
	else if (num == 10)
		cost.price = iter->second.tenPrice;
	else
		return cost;
	cost.priceTye = iter->second.priceType;
	return cost;
}

void CRaffleHero::RaffleHeros(CUser *user,uint32_t id, uint32_t num, std::vector<uint32_t>& heros) const
{
	if (num != 1 && num != 10)
		return;
	auto &addData = user->GetAddData();
	auto raffTimes = addData.mutable_rafftimes();
	if (raffTimes->size() <= 0)
	{
		heros.push_back(514);
		//heros.push_back(35);
		return;
	}
	
	auto iter = m_pools.find(id);
	if (iter == m_pools.end())
		return;

	auto &pool = iter->second;
	std::vector<uint32_t> chances;
	std::vector<const RaffleHero*> raffHeros;

	auto leftTimes = GetLeftRaffTimes(user, id);

	bool haveFiveStar = false;
	for (uint32_t i = 0; i < num; i++)
	{
		chances.clear();
		raffHeros.clear();
		uint32_t pos = CUtility::RandomChance(pool.chances);
		uint32_t rarity = pool.rarities[pos];
		if (rarity == 5)
			haveFiveStar = true;
		if ((num == 1 && leftTimes.oneRaffTimes == 0) || (i == 9 && !haveFiveStar))
			rarity = 5;
		for (auto i = m_groupHeros.begin(); i != m_groupHeros.end(); i++)
		{
			if (i->groupId == pool.groupId && rarity == i->rarity)
			{
				chances.push_back(i->chance);
				raffHeros.push_back(&(*i));
			}
		}
		pos = CUtility::RandomChance(chances);
		heros.push_back(raffHeros[pos]->heroId);
	}
	if (id == 2)
	{
		auto raffTimes = addData.mutable_rafftimes();
		for (auto i = raffTimes->begin(); i != raffTimes->end(); i++)
		{
			if (i->id() == id)
			{
				return;
			}
		}
		if (heros.size() > 0)
		{
			heros[0] = 35;
		}
	}
}

void CRaffleHero::AddUseTimes(CUser * user, uint32_t id,uint32_t num) const
{
	auto &addData = user->GetAddData();
	auto raffTimes = addData.mutable_rafftimes();
	for (auto i = raffTimes->begin(); i != raffTimes->end(); i++)
	{
		if (i->id() == id)
		{
			i->set_usetimes(i->usetimes() + num);
			i->set_onerafftimes(i->onerafftimes() + 1);
			return;
		}
	}
	auto times = raffTimes->Add();
	times->set_id(id);
	times->set_usetimes(num);
}
