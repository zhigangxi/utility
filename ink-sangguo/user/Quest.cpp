#include "Quest.h"
#include "Expressions.h"
#include "ReadWriteUser.h"
#include <time.h>
#include <algorithm> 
#include "Expressions.h"
#include "HeroInst.h"
#include "utility.h"

bool CQuestMgr::InitQuest()
{
	std::list<CQuest*> quests;
	CReadWriteUser::ReadQuest(quests);
	if (quests.size() <= 0)
		return false;

	for (auto i = quests.begin(); i != quests.end(); i++)
	{
		NetMsg::QuestType type = (NetMsg::QuestType)((*i)->GetId());
		if (!InitQuestData(type,*i))
			return false;
		m_quests.insert(std::make_pair(type, *i));
	}

	std::list<CReadData> datas;
	std::vector<const char *> fields = {"quest_id","dialogs"};
	if (!CReadWriteUser::ReadData(datas, fields, "quest_dialog"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		uint32_t id = i->GetVal<uint32_t>(0);
		if (m_questDialogNums.find(id) == m_questDialogNums.end())
		{
			char *p[20];
			uint32_t num = CUtility::SplitLine(p, 20, i->GetVal(1), ',');
			m_questDialogNums.insert(std::make_pair(id, num));
		}
	}

	datas.clear();
	fields = { "hero_class","get" };
	if (!CReadWriteUser::ReadData(datas, fields, "visit_quest_dialog"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		uint32_t id = i->GetVal<uint32_t>(0);
		if (m_visitDialogNums.find(id) == m_visitDialogNums.end())
		{
			char *p[20];
			uint32_t num = CUtility::SplitLine(p, 20, i->GetVal(1), ',');
			m_visitDialogNums.insert(std::make_pair(id, num));
		}
	}

	datas.clear();
	fields = { "type","get" };
	if (!CReadWriteUser::ReadData(datas, fields, "disc_quest_dia"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		uint32_t id = i->GetVal<uint32_t>(0);
		if (m_visitDialogNums.find(id) == m_visitDialogNums.end())
		{
			char *p[20];
			uint32_t num = CUtility::SplitLine(p, 20, i->GetVal(1), ',');
			m_destroyDialogNums.insert(std::make_pair(id, num));
		}
	}

	datas.clear();
	fields = { "id","item","not_find_award","find_award" };
	if (!CReadWriteUser::ReadData(datas, fields, "quest_item_pool"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		DiscoveryItemPool *item = new DiscoveryItemPool;
		item->id = i->GetVal<uint32_t>(0);
		item->item = i->GetVal<uint32_t>(1);
		item->notFindAward = i->GetVal<uint32_t>(2);
		item->findAward = i->GetVal<uint32_t>(3);
		m_discoveryItems.push_back(item);
	}

	datas.clear();
	fields = { "id","hero","not_find_award","find_award" };
	if (!CReadWriteUser::ReadData(datas, fields, "quest_hero_pool"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		DiscoveryHeroPool *item = new DiscoveryHeroPool;
		item->id = i->GetVal<uint32_t>(0);
		item->hero = i->GetVal<uint32_t>(1);
		item->notFindAward = i->GetVal<uint32_t>(2);
		item->findAward = i->GetVal<uint32_t>(3);
		m_discoveryHeros.push_back(item);
	}

	datas.clear();
	fields = { "id","sex","name"};
	if (!CReadWriteUser::ReadData(datas, fields, "quest_hero_name"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		DiscoveryHeroName *item = new DiscoveryHeroName;
		item->id = i->GetVal<uint32_t>(0);
		item->sex = i->GetVal<uint32_t>(1);
		item->name = i->GetVal(2);
		m_discoveryHeroNames.push_back(item);
	}

	datas.clear();
	fields = { "hero_cd","cd_cost_type","cd_cost","refresh_cost_type","refresh_cost" };
	if (!CReadWriteUser::ReadData(datas, fields, "quest_common"))
		return false;
	if (datas.size() <= 0)
		return false;
	auto data = *(datas.begin());
	m_commonInfo.heroCd = data.GetVal<int>(0);
	m_commonInfo.cdCostType = data.GetVal<int>(1);
	m_commonInfo.cdCost = data.GetVal<int>(2);
	m_commonInfo.refreshCostType = data.GetVal<int>(3);
	m_commonInfo.refreshCost = data.GetVal<int>(4);
	return true;
}

uint32_t CQuestMgr::GetDialogNum(uint32_t questId) const
{
	auto iter = m_questDialogNums.find(questId);
	if (iter == m_questDialogNums.end())
		return 0;
	return iter->second;
}

uint32_t CQuestMgr::GetVisitDiaNum(uint32_t heroClassId) const
{
	auto iter = m_visitDialogNums.find(heroClassId);
	if (iter == m_visitDialogNums.end())
		return 0;
	return iter->second;
}

uint32_t CQuestMgr::GetDestroyDiaNum(uint32_t id) const
{
	auto iter = m_destroyDialogNums.find(id);
	if (iter == m_destroyDialogNums.end())
		return 0;
	return iter->second;
}

CQuest * CQuestMgr::GetQuest(uint32_t id) const
{
	auto iter = m_quests.find(id);
	if(iter == m_quests.end())
		return nullptr;
	return iter->second;
}

bool CQuestMgr::InitQuestData(NetMsg::QuestType type, CQuest *quest)
{
	switch (type)
	{
	case NetMsg::ConstructAgriculture:
		return InitConsAgricultureData(quest);
	case NetMsg::ConstructBusiness:
		return InitConsBusinessData(quest);
	case NetMsg::ConstructMilitary:
		return InitConsMilitaryData(quest);
	case NetMsg::BattleCateran:
		return InitBattleCateran(quest);
	case NetMsg::VisitQuest:
		return InitQuestVisit(quest);
	case NetMsg::ConscriptionQuest:
		return InitQuestConscrip(quest);
	case NetMsg::Discovery:
		return Discovery(quest);
	default:
		return true;
	}
	return false;
}

bool CQuestMgr::InitConsAgricultureData(CQuest *quest)
{
	std::list<ConsAgriculture*> quests;
	CReadWriteUser::ReadConsAgriculture(quests);
	if(quests.size() <= 0)
		return false;

	auto fun = [=](ConsAgriculture *data) {
		quest->AddQuestData(data->title, data);
	};
	std::for_each(quests.begin(), quests.end(), fun);
	return true;
}

bool CQuestMgr::InitBattleCateran(CQuest *quest)
{
	std::list<BattleCateran*> battles;
	CReadWriteUser::ReadBattleCateran(battles);

	if (battles.size() <= 0)
		return false;

	auto fun = [=](BattleCateran *data) {
		quest->AddQuestData(data->id, data);
	};
	std::for_each(battles.begin(), battles.end(), fun);
	return true;
}
bool CQuestMgr::InitConsBusinessData(CQuest *quest)
{
	std::list<ConsBusiness*> quests;
	CReadWriteUser::ReadConsBusiness(quests);
	if (quests.size() <= 0)
		return false;

	auto fun = [=](ConsBusiness *data) {
		quest->AddQuestData(data->title, data);
	};
	std::for_each(quests.begin(), quests.end(), fun);
	return true;
}

bool CQuestMgr::InitConsMilitaryData(CQuest *quest)
{
	std::list<ConsMilitary*> quests;
	CReadWriteUser::ReadConsMilitary(quests);
	if (quests.size() <= 0)
		return false;

	auto fun = [=](ConsMilitary *data) {
		quest->AddQuestData(data->title, data);
	};
	std::for_each(quests.begin(), quests.end(), fun);
	return true;
}

bool CQuestMgr::InitQuestVisit(CQuest *quest)
{
	std::list<QuestVisit*> quests;
	if (!CReadWriteUser::ReadQuestVisit(quests))
		return false;

	if (quests.size() <= 0)
		return false;
	auto fun = [=](QuestVisit *data) {
		quest->AddQuestData(data->title, data);
	};
	std::for_each(quests.begin(), quests.end(), fun);

	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "id","rarity","chance" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "quest_visit_ext"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		VisitExtra data;
		data.id = i->GetVal<uint32_t>(0);
		data.rarity = i->GetVal<uint32_t>(1);
		data.chance = i->GetVal<uint32_t>(2);
		m_visitExtData.push_back(data);
	}
	return true;
}

bool CQuestMgr::InitQuestConscrip(CQuest *quest)
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "title","cost","award_id","place","target" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "quest_conscrip"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		QuestConscrip *data = new QuestConscrip;
		data->title = i->GetVal<uint32_t>(0);
		data->cost = i->GetVal<uint32_t>(1);
		data->awardId = i->GetVal<uint32_t>(2);
		data->place = i->GetVal<uint32_t>(3);
		data->target = i->GetVal<uint32_t>(4);
		quest->AddQuestData(data->title, data);
	}
	return true;
}

bool CQuestMgr::Discovery(CQuest *quest)
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "title","item_chance","error_chance","not_find_chance","find_chance","error_award","not_find_award","find_award",
		"in_city_chance","place","item_pool","item_pool_chance","hero_pool","hero_pool_chance" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "quest_discovery"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		QuestDiscovery *data = new QuestDiscovery;
		data->Title = i->GetVal<uint32_t>(0);
		data->ItemChance = i->GetVal<float>(1);
		data->ErrorChance = i->GetVal<uint32_t>(2);
		data->NotFindChance = i->GetVal<uint32_t>(3);
		data->FindChance = i->GetVal<uint32_t>(4);
		data->ErrorAwardID = i->GetVal<uint32_t>(5);
		data->NotFindAwardID = i->GetVal<uint32_t>(6);
		data->FindAwardID = i->GetVal<uint32_t>(7);
		data->InCityChance = i->GetVal<float>(8);
		data->Place = CUtility::ConvertToArray(i->GetVal(9));
		data->ItemPool = CUtility::ConvertToArray(i->GetVal(10));
		data->ItemPoolChance = CUtility::ConvertToArray(i->GetVal(11));
		data->HeroPool = CUtility::ConvertToArray(i->GetVal(12));
		data->HeroPoolChance = CUtility::ConvertToArray(i->GetVal(13));
		quest->AddQuestData(data->Title, data);
	}
	return true;
}

uint32_t CQuest::GetEffectNum(CHeroInst * hero)
{
	QuestData data;
	data.Strength = (float)hero->GetTolStrength();
	data.Leadership = (float)hero->GetTolLeadership();
	data.Wisdom = (float)hero->GetTolWisdom();
	data.Charm = (float)hero->GetTolCharm();
	return (uint32_t)DesignData::Expressions::sGetInstance().sQuestQuestEffect(m_effect, &data);
}

void CQuest::GetHeroAward(CHeroInst * hero, uint32_t effect, NetMsg::DoQuestAward * award)
{
	//Reserver预备兵
	//Population人口
	//money
	award->set_heroid(hero->GetId());
	for (auto i = m_awrads.begin(); i != m_awrads.end(); i++)
	{
		if (m_type == NetMsg::ConscriptionQuest)
		{
			auto heroTmpl = hero->GetHeroTmpl();
			if (heroTmpl == nullptr)
				return;

			switch (i->type)
			{
			case STRENGTH:
				effect = heroTmpl->GetStrength();
				break;
			case LEADERSHIP:
				effect = heroTmpl->GetLeadership();
				break;
			case WISDOM:
				effect = heroTmpl->GetWisdom();
				break;
			case CHARM:
				effect = heroTmpl->GetCharm();
				break;
			case LUCK:
				effect = heroTmpl->GetLuck();
				break;
			}
		}
		int val = (int)(i->val*effect);
		switch (i->type)
		{
		case STRENGTH:
			hero->AddStrength(val);
			break;
		case LEADERSHIP:
			hero->AddLeadership(val);
			break;
		case WISDOM:
			hero->AddWisdom(val);
			break;
		case CHARM:
			hero->AddCharm(val);
			break;
		case LUCK:
			hero->AddLuck(val);
			break;
		default:
			return;
		}
		award->add_awardtype(i->type);
		award->add_awardnum(val);
	}
}

bool CQuest::SetAward(char *type,char *val)
{
	char *types[20];
	char *vals[20];
	int typeNum = CUtility::SplitLine(types, 20, type, ',');
	int valNum = CUtility::SplitLine(vals, 20, val, ',');
	if (typeNum != valNum)
		return false;

	for (int i = 0; i < typeNum; i++)
	{
		Award a;
		a.type = atoi(types[i]);
		a.val = atof(vals[i]);
		switch (a.type)
		{
		case STRENGTH:
		case LEADERSHIP:
		case WISDOM:
		case CHARM:
		case LUCK:
			m_awrads.push_back(a);
			break;
		default:
			return false;
		}
	}
	return true;
}
