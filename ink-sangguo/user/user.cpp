#include "user.h"
#include "HeroInst.h"
#include "ReadWriteUser.h"
#include "UserQuest.h"
#include "UserBattle.h"
#include "ItemInst.h"
#include "ItemTmpl.h"
#include "Rank.h"
#include <iostream>
#include "ReadWriteUser.h"
#include "utility.h"
#include "LearnAndVsCD.h"
#include "GeneralSoulMgr.h"
#include "ResourceInst.h"
std::vector<uint32_t> CUser::m_leveupExp;
std::unordered_map<uint32_t, GetFriendshipByGetHero> CUser::m_FriendshipsWhenGetHero;

bool CUser::InitLevelUpExp()
{
	if (m_leveupExp.size() > 0)
		return true;
	std::list<CReadData> datas;
	std::vector<const char*> fields = { "exp" };
	if (!CReadWriteUser::ReadData(datas, fields, "player_levelup", "order by level"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		m_leveupExp.push_back(i->GetVal<uint32_t>(0));
	}
	return true;
}

bool CUser::InitFriendshipsWhenGetHero()
{
	std::list<GetFriendshipByGetHero*> firendships;
	if(!CReadWriteUser::ReadFriendshipWhenGetHero(firendships))
	return false;
	
	for (auto i = firendships.begin(); i != firendships.end(); i++)
	{
		GetFriendshipByGetHero fri;
		fri.star = (*i)->star;
		fri.first = (*i)->first;
		fri.normal = (*i)->normal;
		m_FriendshipsWhenGetHero.insert(std::make_pair(fri.star, fri));
	}
	return true;
}
CUser::~CUser()
{
	for (auto i = m_heros.begin(); i != m_heros.end(); i++)
	{
		delete i->second;
	}
	m_heros.clear();

	std::for_each(m_quests.begin(), m_quests.end(), [](CUserQuest *quest) {
		delete quest;
	});
	m_quests.clear();

	for (auto i = m_battles.begin(); i != m_battles.end(); i++)
	{
		delete i->second;
	}
	m_battles.clear();

	for (auto i = m_items.begin(); i != m_items.end(); i++)
	{
		delete i->second;
	}
	m_items.clear();

	for (auto i = m_resourceFields.begin(); i != m_resourceFields.end(); i++)
	{
		delete i->second;
	}
	m_resourceFields.clear();
}

void CUser::AddExp(int exp)
{
	/*const uint32_t levelUpExp[] = { 110,240,480,800,1280,1800,2760,3800,5160,6800,8860,10800,13760,16800,20000,24000,28600,33600,39600,43560,48000,52800,58600,64000,72000,79200,88000,96800,136800,150000,165000,180000,198000,217800,240000,264000,298000,327800,360000,378000,396900,416000,436800,458000,480000,504000,528000,554400,600000,636000,680000,720000,763200,808000,856000,906000,960000,1020000,1080000,1160000,1240000,1320000,1400000,1480000,1560000,1640000,1720000,1800000,1920000,2040000,2160000,2280000,2400000,2520000,2640000,2760000,2880000,3000000,3120000,3300000,3480000,3660000,3840000,4020000,4200000,4380000,4560000,4740000,4920000,5160000,5400000,5640000,5880000,6120000,6360000,6600000,6840000,7080000,7320000 };

	const uint32_t levelNum = sizeof(levelUpExp) / sizeof(levelUpExp[0]) + 1;
	if (m_level <= 0)
		return;

	int tempExp = m_exp;
	tempExp += exp;
	while (m_level < levelNum && tempExp >= levelUpExp[m_level - 1])
	{
		tempExp -= levelUpExp[m_level - 1];
		m_level++;
	}

	SetExp(tempExp);*/
	m_exp += exp;
	CUtility::AddExp(m_leveupExp, m_exp, m_level);
}

bool CUser::FirstHaveHero(uint32_t heroId)
{
	for (int i = 0; i < m_addData.firsthavehero_size(); i++)
	{
		if (m_addData.firsthavehero(i) == heroId)
			return true;
	}
	return false;
}

bool CUser::RecruitHero(uint32_t recruitId)
{
	for (int i = 0; i < m_addData.recruitheros_size(); i++)
	{
		if (m_addData.recruitheros(i) == recruitId)
			return true;
	}
	return false;
}

void CUser::AddRecruitHero(uint32_t recruitId)
{
	m_addData.add_recruitheros(recruitId);
}

void CUser::AddFirstHaveHero(uint32_t heroId)
{
	m_addData.add_firsthavehero(heroId);
}

void CUser::ReadMail()
{
	if (m_readMail)
		return;
	m_readMail = true;
	CReadWriteUser::ReadMail(this);
}

void CUser::ReadFriend()
{
	if (m_readFriend)
		return;
	m_readFriend = true;

	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "friend_id" };
	char con[128];
	snprintf(con, sizeof(con), "where user_id=%u", m_userId);
	if (!CReadWriteUser::ReadData(datas, fieldNames, "user_friend", con))
	{
		std::cout << "read :" << m_userId << " friend error" << std::endl;
		return;
	}

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		FriendInfo info;
		info.id = i->GetVal<uint32_t>(0);
		info.isNew = false;
		m_friends.push_back(info);
	}
}

void CUser::AddFriend(uint32_t id)
{
	for (auto i = m_friends.begin(); i != m_friends.end(); i++)
	{
		if (i->id == id)
			return;
	}
	FriendInfo info;
	info.id = id;
	info.isNew = true;
	m_friends.push_back(info);
}

CHeroInst *CUser::AddHero(uint32_t heroId ,bool isExchangeGeneralSoul)
{
	CHeroInst *hero = new CHeroInst;
	if (!hero->Init(heroId))
	{
		delete hero;
		return nullptr;
	}
	auto tmpl = hero->GetHeroTmpl();
	if (tmpl != nullptr)
	{
		if (!FirstHaveHero(tmpl->GetClass()))
		{
			AddFirstHaveHero(tmpl->GetClass());
			if(false == isExchangeGeneralSoul)
			{
				auto val = GetFriendShipWhenGetHeroFromTable(tmpl->GetClass());
				AddHeroFriendship(tmpl->GetClass(), std::get<0>(val)); //数值需要从数据库读取
			}
		}
		else
		{
			auto val = GetFriendShipWhenGetHeroFromTable(tmpl->GetClass());
			AddHeroFriendship(tmpl->GetClass(), std::get<1>(val));
		}
	}
	hero->InitSkill();

	m_heros.insert(std::make_pair(hero->GetId(), hero));
	return hero;
}

void CUser::AddHero(CHeroInst * hero)
{
	if (hero == nullptr)
		return;
	
	m_heros.insert(std::make_pair(hero->GetId(), hero));
}

CResourceInst *CUser::AddResourceField(uint32_t fieldId)
{
	std::unordered_map<uint32_t, uint32_t> userResource;

	for(auto i = m_resourceFields.begin();i != m_resourceFields.end();i++)
	{
		auto res = i->second;
		if( 0 == res->GetResourceClose())
			userResource.insert(std::make_pair(res->GetPosID(), res->GetPosID()));
	}


	CResourceInst * resField = new CResourceInst;
	if (!resField->Init(fieldId,this->GetLevel(), this->GetForce(), userResource))
	{
		delete resField;
		return nullptr;
	}

	m_resourceFields.insert(std::make_pair(resField->GetId(), resField));
	return resField;
}

void CUser::AddResourceFields(CResourceInst * resField)
{
	if (resField == nullptr)
		return;
	m_resourceFields.insert(std::make_pair(resField->GetId(), resField));
}

CResourceInst * CUser::GetResourceField(uint64_t id)
{
	auto iter = m_resourceFields.find(id);
	if (iter == m_resourceFields.end())
		return nullptr;

	return iter->second;
}

bool CUser::AddQuest(CUserQuest * quest)
{
	if (GetQuestNum() >= MAX_QUEST_NUM)
		return false;
	m_quests.push_back(quest);
	return true;
}

CHeroInst * CUser::GetHero(uint64_t id)
{
	auto iter = m_heros.find(id);
	if (iter == m_heros.end())
		return nullptr;

	return iter->second;
}

void CUser::DelHero(uint64_t id)
{
	auto iter = m_heros.find(id);
	delete iter->second;
	if (iter != m_heros.end())
		m_heros.erase(iter);
}

CUserQuest * CUser::FindQuest(uint64_t id)
{
	for (auto i = m_quests.begin(); i != m_quests.end(); i++)
	{
		if ((*i)->GetId() == id)
			return *i;
	}
	return nullptr;
}

void CUser::GetQuestList(NetMsg::QuestListAck & ack)
{
	for (auto i = m_quests.begin(); i != m_quests.end(); i++)
	{
		ack.add_questlist((*i)->GetId());
	}	
}

void CUser::AddContribution(int con)
{
	m_contribution += con;
	/*auto rank = CRankMgr::get_const_instance().GetRankByExp(m_contribution);
	m_rank = rank->GetLevel();*/
}

void CUser::ReadHeros()
{
	if (m_readHero)
		return;
	m_readHero = true;
	CReadWriteUser::ReadUserHero(this);
}

void CUser::InitFormation()
{
	if (m_heroFormation.formation_size() >= FORMATION_NUM)
		return;
	
	m_heroFormation.Clear();
	for (int i = 0; i < FORMATION_NUM; i++)
	{
		NetMsg::Formation *formation = m_heroFormation.add_formation();
		formation->Clear();
		for (int i = 0; i < FORMATION_HERO_NUM; i++)
		{
			formation->add_heroid(0);
		}
	}
}

void CUser::SetFormation(uint32_t pos, const NetMsg::Formation &formation)
{
	if (pos >= (uint32_t)m_heroFormation.formation_size())
		return;
	m_heroFormation.mutable_formation(pos)->CopyFrom(formation);
	m_heroFormation.set_curformation(pos);
}

uint32_t CUser::GetAdvEventId()
{
	if (m_advEnentId == 0)
	{
		for (int i = 0; i < m_advInfoAck.events_size(); i++)
		{
			NetMsg::AdvantureEvent *enent = m_advInfoAck.mutable_events(i);
			if (enent->id() > m_advEnentId)
				m_advEnentId = enent->id();
		}
	}
	m_advEnentId++;
	return m_advEnentId;
}

CUserBattle * CUser::GetBattle(uint64_t id)
{
	auto iter = m_battles.find(id);
	if(iter == m_battles.end())
		return nullptr;
	return iter->second;
}

void CUser::AddBattle(CUserBattle * battle)
{
	m_battles.insert(std::make_pair(battle->GetId(), battle));
}

void CUser::ReadBattle()
{
	if (m_readBattle)
		return;
	m_readBattle = true;

	CReadWriteUser::ReadUserBattle(this);
}

void CUser::AddItem(CItemInst *item)
{
	m_items.insert(std::make_pair(item->GetId(), item));
}

CItemInst * CUser::GetItem(uint64_t id)
{
	auto iter = m_items.find(id);
	if(iter == m_items.end())
		return nullptr;
	return iter->second;
}

CItemInst * CUser::GetItemByTmpl(uint32_t tmplId)
{
	for (auto i = m_items.begin(); i != m_items.end(); i++)
	{
		CItemTmpl *tmpl = i->second->GetItemTmpl();
		if (tmpl != nullptr && tmpl->GetId() == tmplId)
			return i->second;
	}
	return nullptr;
}

void CUser::DelItem(uint64_t id)
{
	auto iter = m_items.find(id);
	if (iter == m_items.end())
		return;
	delete iter->second;
	m_items.erase(iter);
}

void CUser::ReadItem()
{
	if (m_readItem)
		return;
	m_readItem = true;
	CReadWriteUser::ReadUserItem(this);
}

void CUser::ReadResource()
{
	if (m_readResource)
		return;
	m_readResource = true;
	CReadWriteUser::ReadUserResourceField(this);
}

void CUser::DelResourceField(uint64_t id)
{
	auto iter = m_resourceFields.find(id);
	if (iter == m_resourceFields.end())
		return;
	delete iter->second;
	m_resourceFields.erase(iter);
}

bool CUser::CouldGetFriendshipByLevel(uint32_t heroClass)
{
	Friendship *fri;
	auto iter = m_heroFriendships.find(heroClass);
	if (iter == m_heroFriendships.end())
	{

	}
	else
	{
		fri = &(iter->second);
		if(fri->lv <1)
		return false;
	}
	return true;
}

void CUser::AddHeroFriendship(uint32_t heroClass, int friendship)
{
	Friendship *fri;
	auto iter = m_heroFriendships.find(heroClass);
	if (iter == m_heroFriendships.end())
	{
		Friendship f;
		f.exp = friendship;
		auto r = m_heroFriendships.insert(std::make_pair(heroClass, f));
		fri = &(r.first->second);
	}
	else
	{
		iter->second.exp += friendship;
		fri = &(iter->second);
	}
	uint32_t datasize = CLearnAndVsCDMgr::get_const_instance().GetDataSize();
	std::vector<uint32_t> expVec(datasize);
	for(int i=0; i<datasize; i++)
	{
		CLearnAndVsCD cd;	
		if(CLearnAndVsCDMgr::get_const_instance().GetCDinfo(i + 1, cd))
		{
			//expVec.push_back(cd.exp);
			expVec[i] = cd.exp;
		}
	}

	//const uint32_t levelUpExp[] = { 100,200,500,800,1000 };
	const uint32_t levelUpExp[] = { expVec[0],expVec[1],expVec[2],expVec[3],expVec[4] };
	const uint32_t levelNum = sizeof(levelUpExp) / sizeof(levelUpExp[0]) + 1;
	
	while (fri->lv <= levelNum && fri->exp >= levelUpExp[fri->lv])
	{
		fri->exp -= levelUpExp[fri->lv];
		fri->lv++;
	}
}

void CUser::SetHeroFriendship(uint32_t heroClass, int exp, uint32_t lv)
{
	Friendship fri;
	fri.exp = exp;
	fri.lv = lv;
	auto iter = m_heroFriendships.find(heroClass);
	if (iter == m_heroFriendships.end())
	{
		m_heroFriendships.insert(std::make_pair(heroClass, fri));
	}
	else
	{
		iter->second = fri;
	}
}

CUser::Friendship CUser::GetHeroFriendship(uint32_t heroClass)
{
	auto iter = m_heroFriendships.find(heroClass);
	if (iter == m_heroFriendships.end())
	{
		return Friendship();
	}
	else
	{
		return iter->second;
	}
}

void CUser::GetHeroFriendship(NetMsg::HeroFriendshipAck * ack)
{
	for (auto i = m_heroFriendships.begin(); i != m_heroFriendships.end(); i++)
	{
		NetMsg::HeroFriendship *fri = ack->add_herofri();
		fri->set_heroclass(i->first);
		fri->set_friendshipexp(i->second.exp);
		fri->set_friendshiplv(i->second.lv);
	}
}

// add by lyle


void CUser::AddUserLearnVSCD(uint32_t heroClass, int CDTime)
{
	UserLearnCD* soul;
	auto iter = m_userLearnVsCD.find(heroClass);
	if (iter == m_userLearnVsCD.end())
	{
		UserLearnCD cd;
		cd.heroClass = heroClass;
		cd.userCD = CDTime;
		auto r = m_userLearnVsCD.insert(std::make_pair(heroClass, cd));
		soul = &(r.first->second);
	}
	else
	{
		iter->second.userCD += CDTime;
	}
}
void CUser::SetUserLearnVSCD(uint32_t heroClass, int CDTime)
{
	UserLearnCD cd;
	cd.userCD = CDTime;
	cd.heroClass = heroClass;
	auto iter = m_userLearnVsCD.find(heroClass);
	if (iter == m_userLearnVsCD.end())
	{
		m_userLearnVsCD.insert(std::make_pair(heroClass, cd));
	}
	else
	{
		iter->second = cd;
	}
}




void CUser::AddHeroGeneralSoul(uint32_t heroClass, int soulNum)
{
	GeneralSoul* soul;
	auto iter = m_heroGeneralSouls.find(heroClass);
	if (iter == m_heroGeneralSouls.end())
	{
		GeneralSoul s;
		s.num = soulNum;
		auto r = m_heroGeneralSouls.insert(std::make_pair(heroClass, s));
		soul = &(r.first->second);
	}
	else
	{
		iter->second.num += soulNum;
	}
}
void CUser::SetHeroGeneralSoul(uint32_t heroClass, int soulNum)
{
	GeneralSoul soul;
	soul.num = soulNum;
	auto iter = m_heroGeneralSouls.find(heroClass);
	if (iter == m_heroGeneralSouls.end())
	{
		m_heroGeneralSouls.insert(std::make_pair(heroClass, soul));
	}
	else
	{
		iter->second = soul;
	}
}

CUser::GeneralSoul CUser::GetHeroGeneralSoul(uint32_t heroClass)
{
	auto iter = m_heroGeneralSouls.find(heroClass);
	if (iter == m_heroGeneralSouls.end())
	{
		return GeneralSoul();
	}
	else
	{
		return iter->second;
	}
}

void CUser::GetHeroGeneralSoul(NetMsg::HeroGeneralSoulAck *ack)
{
	for (auto i = m_heroGeneralSouls.begin(); i != m_heroGeneralSouls.end(); i++)
	{
		NetMsg::HeroGeneralSoul *soul = ack->add_herosoul();
		soul->set_heroclass(i->first);
		soul->set_num(i->second.num);
	}
}
CUser::UserLearnCD CUser::GetUserLearnCD(uint32_t heroClass)
{
	auto iter = m_userLearnVsCD.find(heroClass);
	if (iter == m_userLearnVsCD.end())
	{
		return UserLearnCD();
	}
	else
	{
		return iter->second;
	}
}

bool CUser::CouldBeLearn(uint32_t heroClass)
{
	auto user = GetUserLearnCD(heroClass);
	time_t now = time(nullptr);
	if(user.userCD <= now)
	{
		return true;
	}
	return false;
}

void CUser::doAddUserCD(uint32_t heroClass, int starLv)
{
	time_t now = time(nullptr);
	CLearnAndVsCD cd;
	CLearnAndVsCDMgr::get_const_instance().GetCDinfo(starLv, cd);
	SetUserLearnVSCD(heroClass, now + cd.teacher_learn_cd);
}

void CUser::GetUserLearnCD(NetMsg::UserLearnVsCDAck *ack)
{
	for (auto i = m_userLearnVsCD.begin(); i != m_userLearnVsCD.end(); i++)
	{
		NetMsg::UserLearnVsCDInfo *cd = ack->add_cdinfo();
		cd->set_heroclass(i->first);
		cd->set_userlearnvscd(i->second.userCD);
	}
}

void CUser::GetUserLearnCDToClient(NetMsg::UserLearnVsCDAck *ack)
{
	for (auto i = m_userLearnVsCD.begin(); i != m_userLearnVsCD.end(); i++)
	{
		NetMsg::UserLearnVsCDInfo *cd = ack->add_cdinfo();
		cd->set_heroclass(i->first);
		time_t now = time(nullptr);
		if (i->second.userCD > now)
			cd->set_userlearnvscd((uint32_t)(i->second.userCD - now));
		else
			cd->set_userlearnvscd(0);
	}
}


std::tuple<int,int> CUser::GetFriendShipWhenGetHeroFromTable(uint32_t heroClass)
{
	std::tuple<int,int> val = {0,0};
	int starNum = CGeneralSoulMgr::get_const_instance().GetStarByHeroClass(heroClass);
	if (m_FriendshipsWhenGetHero.size() <= 0)
		return val;

	auto iter = m_FriendshipsWhenGetHero.find(starNum);
	if (iter == m_FriendshipsWhenGetHero.end())
	{
		return val;
	}
	else
	{
		GetFriendshipByGetHero val = iter->second;
		int first = val.first;
		int normal = val.normal;
		return{ first,normal };
	}
	return val;
}

void CUser::GetCurBattleHeros(std::vector<uint64_t>& heros)
{
	uint32_t pos = m_heroFormation.curformation();
	if (pos >= m_heroFormation.formation_size())
		return;

	const NetMsg::Formation &formation = m_heroFormation.formation(pos);
	for (uint32_t i = 0; i < formation.heroid_size(); i++)
	{
		if(formation.heroid(i) != 0)
			heros.push_back(formation.heroid(i));
	}
}

