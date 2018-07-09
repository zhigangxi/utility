#include "UserQuest.h"
#include "utility.h"
#include "Quest.h"
#include "user.h"
#include "Force.h"
#include "WorldCity.h"
#include "utility.h"
#include "UserBattle.h"
#include "WorldPath.h"
#include "HeroTmpl.h"

uint64_t CUserQuest::m_curQuestId;

CUserQuest * CUserQuest::CreateQuest(NetMsg::QuestType type,CUser *user)
{
	CUserQuest *quest = new CUserQuest;
	if (!quest->Init(type,user))
	{
		delete quest;
		return nullptr;
	}
	return quest;
}

bool CUserQuest::Init(NetMsg::QuestType type,CUser *user)
{
	m_QuestData.set_type(type);
	m_QuestData.set_state(NetMsg::QuestState::NotAcceptQuest);
	if(GetId() == 0)
		SetId(GenerateUID());

	if (type != NetMsg::VisitQuest)
	{
		uint32_t num = CQuestMgr::get_const_instance().GetDialogNum(type);
		if (num > 0)
			m_QuestData.mutable_data()->set_dialog(CUtility::RandomInt(0, num));
	}
	switch (type)
	{
	case NetMsg::ConstructAgriculture:
		return InitConsAgri(user);
	case NetMsg::ConstructBusiness:
		return InitConsBusi(user);
	case NetMsg::ConstructMilitary:
		return InitConsMilit(user);
	case NetMsg::BattleCateran:
		return InitBattleCateran(user);
	case NetMsg::VisitQuest:
	{
		bool retVal = InitVisitQuest(user);
		uint32_t heroClass = m_QuestData.mutable_data()->mutable_visit()->heroclass();
		uint32_t num = CQuestMgr::get_const_instance().GetVisitDiaNum(heroClass);
		if(num > 0)
			m_QuestData.mutable_data()->set_dialog(CUtility::RandomInt(0, num));
		return retVal;
	}
	case NetMsg::ConscriptionQuest:
		return InitQuestConscrip(user);
	case NetMsg::Discovery:
		return InitDiscovery(user);
	default:
		break;
	}
	return false;
}

void CUserQuest::SetId(uint64_t id)
{
	m_QuestData.set_uid(id);
}

uint64_t CUserQuest::GetId()
{
	return m_QuestData.uid();
}

bool CUserQuest::InitData(const char * str)
{
	if (str == nullptr)
		return false;

	m_updateToDb = true;
	return CUtility::StrToMsg((char*)str, &m_QuestData);
}

void CUserQuest::GetDataStr(std::string & str)
{
	CUtility::MsgToStr(&m_QuestData, str);
}

const NetMsg::Award *CUserQuest::GetAward()
{
	NetMsg::QuestData *data = m_QuestData.mutable_data();
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
	{
		const NetMsg::Award &a = data->agriculture().award();
		return &a;
	}
	case NetMsg::ConstructBusiness:
	{
		const NetMsg::Award &a = data->business().award();
		return &a;
	}
	case NetMsg::ConstructMilitary:
	{
		const NetMsg::Award &a = data->consmill().award();
		return &a;
	}
	case NetMsg::BattleCateran:
	{
		const NetMsg::Award &a = data->battle().award();
		return &a;
	}
	case NetMsg::VisitQuest:
	{
		const NetMsg::Award &a = data->visit().award();
		return &a;
	}
	case NetMsg::ConscriptionQuest:
	{
		const NetMsg::Award &a = data->conscription().award();
		return &a;
	}
	case NetMsg::Discovery:
	{
		const NetMsg::Award &a = data->discovery().award();
		return &a;
	}
	}
	return nullptr;
}

uint32_t CUserQuest::GetCity()
{
	NetMsg::QuestData *data = m_QuestData.mutable_data();
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
		return data->agriculture().cityid();
	case NetMsg::ConstructBusiness:
		return data->business().cityid();
	case NetMsg::ConstructMilitary:
		return data->consmill().cityid();
	case NetMsg::VisitQuest:
		return data->visit().city();
	case NetMsg::ConscriptionQuest:
		return data->conscription().cityid();
	case NetMsg::Discovery:
		return data->discovery().city();
	}
	return 0;
}

void CUserQuest::GetCityAward(CUser *user ,uint32_t & type)
{
	switch (m_QuestData.type())
	{
		case NetMsg::Discovery:
		{
			NetMsg::QuestData *data = m_QuestData.mutable_data();
			type = data->discovery().type();
			switch (type)
			{
			case NetMsg::FindItem:
				{
					CUtility::AddCityItem(user->GetCampCity(), data->discovery().item(),1);
				}
				break;
			case NetMsg::FindHero:
				{
					CUtility::AddCityHero(user->GetCampCity(), data->discovery().name(), data->discovery().level());
				}
				break;
			default:
				break;
			}
		}
		break;
		default:
		break;
	}
}
void CUserQuest::RefreshAward(CUser *user)
{
	NetMsg::Award *award = nullptr;
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
	{		
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestConsAgriculture *consAgr = questData->mutable_agriculture();
		award = consAgr->mutable_award();
		break;
	}
	case NetMsg::ConstructBusiness:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestConsBusiness *consAgr = questData->mutable_business();
		award = consAgr->mutable_award();
		break;
	}
	case NetMsg::ConstructMilitary:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestConsMilitary *consAgr = questData->mutable_consmill();
		award = consAgr->mutable_award();
		break;
	}
	case NetMsg::BattleCateran:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestBattle *battle = questData->mutable_battle();
		award = battle->mutable_award();
		break;
	}
	case NetMsg::VisitQuest:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestVisit *visit = questData->mutable_visit();
		award = visit->mutable_award();
		break;
	}
	case NetMsg::ConscriptionQuest:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		NetMsg::QuestConscription *cons = questData->mutable_conscription();
		award = cons ->mutable_award();
		break;
	}
	/*case NetMsg::Discovery:
	{
		NetMsg::QuestData *questData = m_QuestData.mutable_data();
		auto dis = questData->mutable_discovery();
		award = dis->mutable_award();
		break;
	}*/

	}
	if (award == nullptr || award->awardid() == 0)
		return;
	uint32_t awardId = award->awardid();
	CUtility::GetAward(awardId, user, award);
}

bool CUserQuest::InitConsAgri(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	ConsAgriculture *data = quest->GetQuestData<ConsAgriculture>(user->GetTitle());
	if (data == nullptr)
		return false;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	NetMsg::QuestConsAgriculture *consAgr = questData->mutable_agriculture();
	
	CUtility::GetAward(data->awardId, user,consAgr->mutable_award());
	
	int per = (int)(100 * data->selfCityPer);
	int r = rand() % 100;
	if (r < per)
		consAgr->set_cityid(user->GetCampCity());
	else
	{
		uint32_t city = CUtility::RandOtherCity(user);
		if (city == 0)
			return false;
		consAgr->set_cityid(city);
	}
	consAgr->set_curtarget(0);
	uint32_t place = CUtility::RandomSel(data->places);
	consAgr->set_place(place);
	consAgr->set_target(data->target);
	consAgr->set_title(data->title);
	return true;
}

bool CUserQuest::InitConsBusi(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	ConsBusiness *data = quest->GetQuestData<ConsBusiness>(user->GetTitle());
	if (data == nullptr)
		return false;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	NetMsg::QuestConsBusiness *consAgr = questData->mutable_business();

	CUtility::GetAward(data->awardId, user, consAgr->mutable_award());

	int per = (int)(100 * data->selfCityPer);
	int r = rand() % 100;
	if (r < per)
		consAgr->set_cityid(user->GetCampCity());
	else
	{
		uint32_t city = CUtility::RandOtherCity(user);
		if (city == 0)
			return false;
		consAgr->set_cityid(city);
	}
	consAgr->set_curtarget(0);
	uint32_t place = CUtility::RandomSel(data->places);
	consAgr->set_place(place);
	consAgr->set_target(data->target);
	consAgr->set_title(data->title);
	return true;
}

bool CUserQuest::InitConsMilit(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	ConsMilitary *data = quest->GetQuestData<ConsMilitary>(user->GetTitle());
	if (data == nullptr)
		return false;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	NetMsg::QuestConsMilitary *consAgr = questData->mutable_consmill();

	CUtility::GetAward(data->awardId, user, consAgr->mutable_award());

	int per = (int)(100 * data->selfCityPer);
	int r = rand() % 100;
	if (r < per)
		consAgr->set_cityid(user->GetCampCity());
	else
	{
		uint32_t city = CUtility::RandOtherCity(user);
		if (city == 0)
			return false;
		consAgr->set_cityid(city);
	}
	consAgr->set_curtarget(0);
	uint32_t place = CUtility::RandomSel(data->places);
	consAgr->set_place(place);
	consAgr->set_target(data->target);
	consAgr->set_title(data->title);
	return true;
}

bool CUserQuest::InitBattleCateran(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	std::list<BattleCateran*> battleCats;
	quest->GetQuestDatas(battleCats);	
	if (battleCats.size() <= 0)
		return false;

	struct IdChance
	{
		uint32_t id;
		uint32_t chance;
	};
	std::vector<IdChance> idChances;
	uint32_t chance = 0;
	for (auto i = battleCats.begin(); i != battleCats.end(); i++)
	{
		if ((*i)->chance <= 0)
			continue;
		chance += (*i)->chance;
		IdChance id;
		id.id = (*i)->id;
		id.chance = chance;
		idChances.push_back(id);
	}
	if (idChances.size() <= 0)
		return false;

	uint32_t c = (uint32_t)CUtility::RandomInt(0, chance);
	int id = 0;
	for (auto i = idChances.rbegin(); i != idChances.rend(); i++)
	{
		if (c <= i->chance)
		{
			id = i->id;
			break;
		}
	}
	if (id == 0)
		return false;

	BattleCateran *data = quest->GetQuestData<BattleCateran>(id);
	if (data == nullptr)
		return false;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	NetMsg::QuestBattle *battle = questData->mutable_battle();
	battle->set_battleid(CUserBattle::GenerateUID());
	battle->set_enemyteam(data->enemyTeam);

	CUtility::GetAward(data->awardId, user, battle->mutable_award());

	uint32_t cityId = 0;
	float r = CUtility::RandomInt(0, 100);
	r = r / 100.0f;
	if (r < data->selfCityPer)
	{
		cityId = user->GetCampCity();
	}
	else
	{
		cityId = CUtility::RandOtherCity(user);
	}
	CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
	if (city == nullptr)
		return false;

	std::vector<uint32_t> paths;
	CWorldPathMgr::get_const_instance().GetPaths(city->GetWayPoint(), paths);
	if (paths.size() <= 0)
		return false;
	uint32_t path = CUtility::RandomSel(paths);
	battle->set_enemypath(path);
	r = CUtility::RandomInt(data->onRoadBegin*100, data->onRoadEnd*100) / 100.0f;
	battle->set_percent(r);
	battle->set_lefttime(data->duration);
	return true;
}

bool CUserQuest::InitVisitQuest(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	QuestVisit *data = quest->GetQuestData<QuestVisit>(user->GetTitle());
	if (data == nullptr)
		return false;

	uint32_t r = CUtility::RandomInt(1, data->inSelfforce + data->inOtherforce);
	std::vector<uint32_t> heros;
	if (r <= data->inSelfforce)
		CHeroClassMgr::get_const_instance().GetForceHero(user->GetForce(), heros);
	else
		CHeroClassMgr::get_const_instance().GetNotForceHero(user->GetForce(), heros);
	if (heros.size() <= 0)
		return false;

	auto VISIT_CHANCE = CQuestMgr::get_const_instance().GetVisitExtData();
	const uint32_t VISIT_NUM = VISIT_CHANCE.size();

	uint32_t pos = CUtility::RandomChance(data->raritysChance);
	uint32_t raricy = data->raritys[pos];

	std::vector<uint32_t> chances;
	std::vector<VisitExtra*> visitExt;
	for (uint32_t i = 0; i < VISIT_NUM; i++)
	{
		if (VISIT_CHANCE[i].rarity == raricy)
		{
			chances.push_back(VISIT_CHANCE[i].chance);
			visitExt.push_back(&(VISIT_CHANCE[i]));
		}
	}
	pos = CUtility::RandomChance(chances);
	uint32_t heroId = VISIT_CHANCE[pos].id;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	NetMsg::QuestVisit *visit = questData->mutable_visit();
	CHeroClass *hero = CHeroClassMgr::get_const_instance().GetHeroClass(heroId);
	if (hero == nullptr)
		return false;

	CUtility::GetAward(data->awardId, user, visit->mutable_award());

	visit->set_city(hero->GetCity());
	visit->set_heroclass(heroId);
	visit->set_place(data->place);
	return true;
}

bool CUserQuest::InitQuestConscrip(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	QuestConscrip *data = quest->GetQuestData<QuestConscrip>(user->GetTitle());
	if (data == nullptr)
		return false;

	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	auto *consAgr = questData->mutable_conscription();

	CUtility::GetAward(data->awardId, user, consAgr->mutable_award());
	consAgr->set_cityid(user->GetCampCity());
	consAgr->set_curtarget(0);
	consAgr->set_target(data->target);
	uint32_t place = data->place;
	consAgr->set_place(place);
	consAgr->set_title(data->title);
	return true;
}

uint32_t RandomItem(std::vector<uint32_t> &chance,std::vector<uint32_t> &pool,uint32_t &findAward,uint32_t &notFindAward)
{
	uint32_t pos = CUtility::RandomChance(chance);
	if (pos >= pool.size())
		return 0;
	auto itemPool = CQuestMgr::get_mutable_instance().GetDisItems();
	std::vector<DiscoveryItemPool*> items;
	for (auto i = itemPool->begin(); i != itemPool->end(); i++)
	{
		if ((*i)->id == pool[pos])
			items.push_back(*i);
	}
	if (items.size() <= 0)
		return 0;
	auto item = CUtility::RandomSel(items);
	findAward = item->findAward;
	notFindAward = item->notFindAward;
	return item->item;
}

uint32_t RandomHero(std::vector<uint32_t> &chance, std::vector<uint32_t> &pool,uint32_t &findAwrd,uint32_t &notFindAwrd,std::string &name)
{
	uint32_t pos = CUtility::RandomChance(chance);
	if (pos >= pool.size())
		return 0;
	auto heroPool  = CQuestMgr::get_mutable_instance().GetDisHeros();
	std::vector<DiscoveryHeroPool*> heros;
	for (auto i = heroPool->begin(); i != heroPool->end(); i++)
	{
		if ((*i)->id == pool[pos])
			heros.push_back(*i);
	}
	if (heros.size() <= 0)
		return 0;
	
	auto hero = CUtility::RandomSel(heros);
	findAwrd = hero->findAward;
	notFindAwrd = hero->notFindAward;

	auto names = CQuestMgr::get_mutable_instance().GetDisHeroName();
	std::vector<DiscoveryHeroName*> heroNames;
	auto heroTmpl = CHeroMgr::get_const_instance().GetHero(hero->hero);
	if (heroTmpl == nullptr)
		return 0;

	for (auto i = names->begin(); i != names->end(); i++)
	{
		if ((*i)->sex == heroTmpl->GetSex())
			heroNames.push_back(*i);
	}
	auto selHero = CUtility::RandomSel(heroNames);
	name = selHero->name;
	return hero->hero;
}

bool CUserQuest::InitDiscovery(CUser *user)
{
	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
	if (quest == nullptr)
		return false;
	QuestDiscovery *data = quest->GetQuestData<QuestDiscovery>(user->GetTitle());
	if (data == nullptr)
		return false;

	int itemChance = (int)(100 * data->InCityChance);
	int r = CUtility::RandomInt(1, 100);
	bool findItem = false;
	if (r <= itemChance)
		findItem = true;
	bool success = true;
	bool findSuccess = false;
	std::vector<uint32_t> chances;
	chances.push_back(data->ErrorChance);
	chances.push_back(data->FindChance);
	chances.push_back(data->NotFindChance);
	uint32_t pos = CUtility::RandomChance(chances);
	if (pos == 0)
		success = false;
	else if (pos == 1)
		findSuccess = true;
	
	NetMsg::QuestData *questData = m_QuestData.mutable_data();
	uint32_t num = CQuestMgr::get_const_instance().GetDestroyDiaNum(data->Title);
	if (num > 0)
		questData->set_dialog(CUtility::RandomInt(0, num));
	auto *dis = questData->mutable_discovery();

	if (!success)
	{
		CUtility::GetAward(data->ErrorAwardID, user, dis->mutable_award());
	}
	else if (findSuccess)
	{
		CUtility::GetAward(data->FindAwardID, user, dis->mutable_award());
	}
	else
	{
		CUtility::GetAward(data->NotFindAwardID, user, dis->mutable_award());
	}
	dis->set_title(data->Title);
	if (success)
	{
		if (findItem)
		{
			uint32_t findAward, notFindAward;
			uint32_t item = RandomItem(data->ItemPoolChance, data->ItemPool,findAward,notFindAward);
			if (findSuccess)
			{
				dis->set_type(NetMsg::QuestDiscoveryFinishedType::FindItem);
				CUtility::GetAward(findAward, user, dis->mutable_award());
			}
			else
			{
				dis->set_type(NetMsg::QuestDiscoveryFinishedType::NotFindItem);
				CUtility::GetAward(notFindAward, user, dis->mutable_award());
			}
			dis->set_item(item);
		}
		else
		{
			uint32_t findAward, notFindAward;
			std::string name;
			uint32_t hero = RandomHero(data->HeroPoolChance, data->HeroPool, findAward, notFindAward,name);
			if (findSuccess)
			{
				dis->set_type(NetMsg::QuestDiscoveryFinishedType::FindHero);
				CUtility::GetAward(findAward, user, dis->mutable_award());
			}
			else
			{
				dis->set_type(NetMsg::QuestDiscoveryFinishedType::NotFindHero);
				CUtility::GetAward(notFindAward, user, dis->mutable_award());
			}
			dis->set_hero(hero);
			dis->set_name(name);
			dis->set_level(user->GetLevel());
		}
	}
	else
	{
		if (findItem)
		{
			dis->set_type(NetMsg::QuestDiscoveryFinishedType::FindItemError);
		}
		else
		{
			dis->set_type(NetMsg::QuestDiscoveryFinishedType::FindHeroError);
		}
	}
	int cityChance = (int)(data->InCityChance * 100);
	r = CUtility::RandomInt(1, 100);
	if (r <= cityChance)
	{
		dis->set_city(CWorldCityMgr::get_const_instance().RandomCityId());
		dis->set_place(CUtility::RandomSel(data->Place));
	}
	else
	{//in road
		CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(CWorldCityMgr::get_const_instance().RandomCityId());
		if (city == nullptr)
			return false;
		std::vector<uint32_t> paths;
		CWorldPathMgr::get_const_instance().GetPaths(city->GetWayPoint(), paths);
		if (paths.size() <= 0)
			return false;
		uint32_t path = CUtility::RandomSel(paths);
		dis->set_enemypath(path);
		float r = CUtility::RandomInt(1, 100) / 100.0f;
		dis->set_percent(r);
	}
	return true;
}

void CUserQuest::DoQuest(uint32_t num)
{
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
	{
		auto data = m_QuestData.mutable_data()->mutable_agriculture();
		uint32_t cur = data->curtarget();
		cur += num;
		data->set_curtarget(cur);
		if (cur >= data->target())
			m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	case NetMsg::ConstructBusiness:
	{
		auto data = m_QuestData.mutable_data()->mutable_business();
		uint32_t cur = data->curtarget();
		cur += num;
		data->set_curtarget(cur);
		if (cur >= data->target())
			m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	case NetMsg::ConstructMilitary:
	{
		auto data = m_QuestData.mutable_data()->mutable_consmill();
		uint32_t cur = data->curtarget();
		cur += num;
		data->set_curtarget(cur);
		if (cur >= data->target())
			m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	case NetMsg::VisitQuest:
	{
		m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	case NetMsg::ConscriptionQuest:
	{
		auto data = m_QuestData.mutable_data()->mutable_conscription();
		uint32_t cur = data->curtarget();
		cur += num;
		data->set_curtarget(cur);
		if (cur >= data->target())
			m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	case NetMsg::Discovery:
	{
		m_QuestData.set_state(NetMsg::FinishQuest);
		break;
	}
	default:
		break;
	}
}
uint32_t CUserQuest::GetCost(CUser *user)
{
	if (m_QuestData.type() == NetMsg::ConscriptionQuest)
	{
		CQuest *quest = CQuestMgr::get_const_instance().GetQuest(m_QuestData.type());
		QuestConscrip *data = quest->GetQuestData<QuestConscrip>(user->GetTitle());
		if (data != nullptr)
			return data->cost;
	}
	return 0;
}

uint32_t CUserQuest::GetTargetNum()
{
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
		return m_QuestData.mutable_data()->mutable_agriculture()->target();
	case NetMsg::ConstructBusiness:
		return m_QuestData.mutable_data()->mutable_business()->target();
	case NetMsg::ConstructMilitary:
		return m_QuestData.mutable_data()->mutable_consmill()->target();
	case NetMsg::ConscriptionQuest:
		return m_QuestData.mutable_data()->mutable_conscription()->target();
	default:
		break;
	}
	return 0;
}

uint32_t CUserQuest::GetCurTarNum()
{
	switch (m_QuestData.type())
	{
	case NetMsg::ConstructAgriculture:
		return m_QuestData.mutable_data()->mutable_agriculture()->curtarget();
	case NetMsg::ConstructBusiness:
		return m_QuestData.mutable_data()->mutable_business()->curtarget();
	case NetMsg::ConstructMilitary:
		return m_QuestData.mutable_data()->mutable_consmill()->curtarget();
	case NetMsg::ConscriptionQuest:
		return m_QuestData.mutable_data()->mutable_conscription()->curtarget();
	default:
		break;
	}
	return 0;
}