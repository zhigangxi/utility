#include <string.h>
#include "utility.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "user.h"
#include "GlobalVal.h"
#include "socket.h"
#include "OnlineUser.h"
#include "Force.h"
#include "WorldCity.h"
#include "HeroInst.h"
#include "ElectionTitle.h"
#include "ItemTmpl.h"
#include "ItemInst.h"
#include <sstream>  
#include <iostream> 
#include "log.h"
#include <random>
#include <algorithm>
#include <iterator>
#include <numeric>
#include "ResourceTmpl.h"
#include "ResourceInst.h"
#include "WorldPath.h"
#include "NetMsg.pb.h"
CSocket *CUtility::m_socket;
COnlineUser *CUtility::m_onlineUser;

void CUtility::Init()
{
	m_socket = CGlobalVal::GetData<CSocket>("server_socket");
	m_onlineUser = CGlobalVal::GetData<COnlineUser>("online_user");
	CLogs::Init();
}

size_t CUtility::StrlenUtf8(const char *s)
{
	size_t i = 0, j = 0;
	while (s[i]) {
		if ((s[i] & 0xc0) != 0x80) j++;
		i++;
	}
	return j;
}

int CUtility::SplitLine(char **templa, int templatecount, char *pkt,char space)
{
	if (pkt == nullptr)
		return 0;

	int i = 0;
	while (*pkt == space)
		++pkt;
	while (*pkt != 0)
	{
		if ((*pkt == '\r') || (*pkt == '\n') || (*pkt == '\t'))
		{
			memmove(pkt,pkt+1,strlen(pkt+1)+1);
		}
		else if (i == 0)
		{
			templa[i] = pkt;
			++i;
		}
		else if ((*pkt == space) && (i < templatecount))
		{
			*pkt = 0;
			++pkt;
			while (*pkt == space)
				++pkt;
			templa[i] = pkt;
			++i;
		}
		else
		{
			++pkt;
		}
		if(i >= templatecount)
			break;
	}
	return i;
}

int64_t CUtility::GetMicroseconds()
{
	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	boost::posix_time::time_duration time_from_epoch = boost::posix_time::microsec_clock::universal_time() - epoch;

	return time_from_epoch.total_microseconds();
	//int64_t tNow = time_from_epoch.total_seconds();
}

void CUtility::MsgToStr(google::protobuf::Message * msg, std::string & str)
{
	std::string serStr;
	msg->SerializeToString(&serStr);

	str.resize(serStr.size() * 2);
	Hexify((unsigned char *)str.c_str(), (unsigned char *)serStr.c_str(),serStr.size());
}

bool CUtility::StrToMsg(const char *str, google::protobuf::Message * msg)
{
	if (str == nullptr)
		return false;
	std::string serStr;
	size_t len = strlen(str);
	if (len < 2)
		return false;

	serStr.resize(len / 2);
	UnHexify((unsigned char *)serStr.c_str(), str);
	return msg->ParseFromString(serStr);
}


int CUtility::UnHexify(unsigned char *obuf, const char *ibuf)
{
	unsigned char c, c2;
	int len = strlen(ibuf) / 2;
	assert(!(strlen(ibuf) % 1)); // must be even number of bytes

	while (*ibuf != 0)
	{
		c = *ibuf++;
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - 10;
		else
			assert(0);

		c2 = *ibuf++;
		if (c2 >= '0' && c2 <= '9')
			c2 -= '0';
		else if (c2 >= 'a' && c2 <= 'f')
			c2 -= 'a' - 10;
		else if (c2 >= 'A' && c2 <= 'F')
			c2 -= 'A' - 10;
		else
			assert(0);

		*obuf++ = (c << 4) | c2;
	}

	return len;
}

void CUtility::Hexify(unsigned char *obuf, const unsigned char *ibuf, int len)
{
	unsigned char l, h;

	while (len != 0)
	{
		h = (*ibuf) / 16;
		l = (*ibuf) % 16;

		if (h < 10)
			*obuf++ = '0' + h;
		else
			*obuf++ = 'a' + h - 10;

		if (l < 10)
			*obuf++ = '0' + l;
		else
			*obuf++ = 'a' + l - 10;

		++ibuf;
		len--;
	}
}

int CUtility::RandomInt(int min, int max)
{
	if (min >= max)
		return min;
	if (max - min + 1 == 0)
		return 0;
	int r = rand();
	r %= (max - min + 1);
	return r + min;
}

uint32_t CUtility::RandomChance(const std::vector<uint32_t> &changes)
{
	if (changes.empty())
		return 0;

	uint32_t tol = 0;
	for (auto i = changes.begin(); i != changes.end(); i++)
	{
		tol += *i;
	}
	uint32_t r = RandomInt(1, tol);
	for (uint32_t i = 0; i < changes.size(); i++)
	{
		if (changes[i] != 0 && r < changes[i])
			return i;
		r -= changes[i];
	}
	return 0;
}

void CUtility::CopyAwards(const std::vector<CAward::AwardItem>& inAwards, NetMsg::Award * outAward)
{
	for (auto i = inAwards.begin(); i != inAwards.end(); i++)
	{
		NetMsg::AwardItem *item = outAward->add_items();
		item->set_id(i->itemId);
		item->set_num(i->num);
		item->set_type(i->type);
	}
}

void CUtility::CopyAwards(const NetMsg::Award * inAwards, std::vector<CAward::AwardItem>& outAward)
{
	for (int i = 0; i < inAwards->items_size(); i++)
	{
		CAward::AwardItem item;
		item.itemId = inAwards->items(i).id();
		item.type = (CAward::AwardItemType)inAwards->items(i).type();
		item.num = inAwards->items(i).num();
		outAward.push_back(item);
	}
}

void CUtility::GetAward(uint32_t awardId, CUser *user, NetMsg::Award *netAward)
{
	if (netAward == nullptr)
		return;
	
	CAward *award = CAwardMgr::get_const_instance().GetAward(awardId);
	if (award != nullptr)
	{
		netAward->set_awardid(awardId);
		std::vector<CAward::AwardItem> awards;
		AwardData data;
		data.Level = user->GetLevel();
		award->GenerateAward(awards, data);
		netAward->clear_items();
		CUtility::CopyAwards(awards, netAward);
	}
}

void CUtility::AddExp(std::vector<uint32_t>& levelUpExp, uint32_t & exp, uint32_t & level)
{
	uint32_t levelNum = levelUpExp.size();
	
	while (level < levelNum && exp >= levelUpExp[level])
	{
		exp -= levelUpExp[level];
		level++;
	}
}

void CUtility::UpdateUserInfo(CUser *user, uint64_t mask)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	NetMsg::PlayerData *player = updateUser->mutable_player();
	updateUser->set_mask(mask);

	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateNick)) != 0)
	{
		std::string nick = user->GetNick();
		char *n[3];
		if (3 == CUtility::SplitLine(n, 3, (char*)nick.c_str(), '|'))
		{
			player->set_firstname(n[0]);
			player->set_secondname(n[1]);
			player->set_lastname(n[2]);
		}
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateLevel)) != 0)
	{
		player->set_level(user->GetLevel());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateExp)) != 0)
	{
		player->set_exp(user->GetExp());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateStamina)) != 0)
	{
		player->set_stamina(user->GetStamina());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateCoin)) != 0)
	{
		player->set_coin(user->GetCoin());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateExploit)) != 0)
	{
		player->set_exploit(user->GetExploit());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateGold)) != 0)
	{
		player->set_gold(user->GetGold());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateVipLevel)) != 0)
	{
		player->set_viplevel(user->GetVipLevel());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateCityId)) != 0)
	{
		player->set_cityid(user->GetCurCity());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateCampCityId)) != 0)
	{
		player->set_campcityid(user->GetCampCity());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateForce)) != 0)
	{
		player->set_force(user->GetForce());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateNewGuide)) != 0)
	{
		player->set_newguide(user->GetNewGuide());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateTitle)) != 0)
	{
		player->set_title(user->GetTitle());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateContribution)) != 0)
	{
		player->set_contribution(user->GetContribution());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateReputation)) != 0)
	{
		player->set_reputation(user->GetReputation());
	}
	if ((mask & (1 << NetMsg::UpdatePlayerAck::UpdateRank)) != 0)
	{
		player->set_rank(user->GetRank());
	}

	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

CUser * CUtility::GetOnlineUser(uint32_t userId)
{
	return m_onlineUser->GetUser(userId);
}

uint32_t CUtility::RandOtherCity(CUser * user)
{
	CForce *force = CForceMgr::get_const_instance().GetForce(user->GetForce());
	if (force == nullptr)
		return 0;
	std::vector<uint32_t> citys;
	std::list<CWorldCity*> *worldCitys = force->GetCitys();
	auto copyCityId = [&](CWorldCity *city) {
		if (city->GetId() != user->GetCampCity())
			citys.push_back(city->GetId());
	};

	std::for_each(worldCitys->begin(), worldCitys->end(), copyCityId);
	if (citys.size() <= 0)
		return 0;
	return RandomSel(citys);
}

void CUtility::CopyResource(CResourceInst * in, NetMsg::ResourceInfoAck * out)
{
	uint32_t resourceId = in->GetResourceID();
	auto resTmpl = CResourceMgr::get_const_instance().GetResourceInfoByIndex(resourceId);
	if(resTmpl)
	{
		time_t nowTime = time(nullptr);
		int leftTime = in->GetCreateTime() + resTmpl->GetResCDTime() - nowTime;
		if(leftTime <=0 )
		{
			in->SetResourceClose(1);
			return;
		}

		NetMsg::ResourceInfo *resInfo = out->add_info();
		resInfo->set_star(resTmpl->GetStar());
		uint32_t type = resTmpl->GetResType();
		resInfo->set_type((NetMsg::ResourceType)type);
		uint32_t resNum = in->GetResourceNum();
		resInfo->set_lefttime(leftTime);
		resInfo->set_resourcenum(resNum);
		resInfo->set_posid(in->GetPosID());
		resInfo->set_uid(in->GetId());
	}
}

void CUtility::CopyHero(CHeroInst * in, NetMsg::HeroAck * out)
{
	NetMsg::HeroInfo *hero = out->add_heros();
	auto copyAttr = [](HeroAttr in, NetMsg::HeroAttr *out) {
		out->set_exp(in.exp);
		out->set_lv(in.lv);
	};
	copyAttr(in->GetCharm(), hero->mutable_extracharm());
	copyAttr(in->GetLeadership(), hero->mutable_extraleadership());
	copyAttr(in->GetLuck(), hero->mutable_extraluck());
	copyAttr(in->GetStrength(), hero->mutable_extrastrength());
	copyAttr(in->GetWisdom(), hero->mutable_extrawisdom());
	hero->set_id(in->GetHeroId());
	hero->set_level(in->GetLevel());
	hero->set_starlevel(in->GetStarLevel());
	hero->set_udid(in->GetId());
	hero->set_exp(in->GetExp());
	time_t now = time(nullptr);
	if (in->GetCDTime() > now)
		hero->set_cdtime((uint32_t)(in->GetCDTime() - now));
	else
		hero->set_cdtime(0);

	for (auto i = in->GetSkills().begin(); i != in->GetSkills().end(); i++)
	{
		NetMsg::SkillInfo *skill = hero->add_skills();
		skill->set_skillexp(i->GetExp());
		skill->set_skillid(i->GetSkillId());
		skill->set_skilllevel(i->GetLevel());
		skill->set_uid(i->GetId());
	}
}

CHeroInst *CUtility::AddUserHero(CUser * user, uint32_t heroId, bool isExchangeGeneralSoul)
{
	auto tmpl = CHeroMgr::get_const_instance().GetHero(heroId);
	if (tmpl == nullptr)
		return nullptr;

	uint32_t heroClass = tmpl->GetClass();
	CUser::Friendship fri = user->GetHeroFriendship(heroClass);
	CHeroInst *hero = user->AddHero(heroId, isExchangeGeneralSoul);
	if (hero == nullptr)
		return nullptr;
	
	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	CopyHero(hero, heroAck);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);

	if(false == isExchangeGeneralSoul)
	{
		CUser::Friendship newFri = user->GetHeroFriendship(heroClass);
		if (fri.exp != newFri.exp || fri.lv != newFri.lv)
		{
			NetMsg::HeroFriendshipAck *heroFriAck = new NetMsg::HeroFriendshipAck;
			SharedMsg ack(heroFriAck);
			auto heroFri = heroFriAck->add_herofri();
			heroFri->set_heroclass(heroClass);
			heroFri->set_friendshipexp(newFri.exp);
			heroFri->set_friendshiplv(newFri.lv);
			m_socket->SendMsg(user->GetSession(), ack);
		}
	}
	return hero;
}

CResourceInst* CUtility::AddUserResourceField(CUser *user, uint32_t fieldId)
{
	auto tmpl = CResourceMgr::get_const_instance().GetResourceInfoByIndex(fieldId);
	if (tmpl == nullptr)
		return nullptr;

	CResourceInst *resField = user->AddResourceField(fieldId);
	if (resField == nullptr)
		return nullptr;

	NetMsg::ResourceInfoAck * resAck = new NetMsg::ResourceInfoAck;
	CopyResource(resField,resAck);
	SharedMsg ack(resAck);
	m_socket->SendMsg(user->GetSession(), ack);
	return resField;
}

void CUtility::SendMsg(CUser *user, SharedMsg & msg)
{
	m_socket->SendMsg(user->GetSession(), msg);
}

bool CUtility::HaveHeroAward(std::vector<CAward::AwardItem> &awards)
{
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		if (IsHeroAward(i->type))
			return true;
	}
	return false;
}

void CUtility::AddUserAwards(CUser *user,const NetMsg::Award *netAward)
{
	if (netAward != nullptr)
	{
		std::vector<CAward::AwardItem> items;
		CUtility::CopyAwards(netAward, items);
		CUtility::AddUserAwards(user,items);
		NetMsg::GiveAward *giveAward = new NetMsg::GiveAward;
		giveAward->mutable_award()->CopyFrom(*netAward);
		SharedMsg sendAward(giveAward);
		m_socket->SendMsg(user->GetSession(), sendAward);
	}
}
void CUtility::AddUserAwards(CUser *user, std::vector<CAward::AwardItem> &awards, uint32_t heroClass)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();
	uint32_t level = user->GetLevel();
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		switch (i->type)
		{
		case CAward::ITEM:
			AddUserItem(user, i->itemId, i->num);
			break;
		case CAward::EXP:
			mask |= 1 << NetMsg::UpdatePlayerAck::UpdateExp;
			user->AddExp(i->num);
			player->set_exp(user->GetExp());
			break;
		case CAward::COIN:
			mask |= 1 << NetMsg::UpdatePlayerAck::UpdateCoin;
			user->SetCoin(user->GetCoin() + i->num);
			player->set_coin(user->GetCoin());
			break;
		case CAward::GOLD:
			mask |= 1 << NetMsg::UpdatePlayerAck::UpdateGold;
			user->SetGold(user->GetGold() + i->num);
			player->set_gold(user->GetGold());
			break;
		case CAward::CONTRIBUTION:
		{
			mask |= 1 << NetMsg::UpdatePlayerAck::UpdateContribution;
			uint32_t rank = user->GetRank();
			user->AddContribution(i->num);
			CElectionTitle::get_mutable_instance().AddElectionData(user->GetUserId(), i->num);

			if (rank != user->GetRank())
			{
				mask |= 1 << NetMsg::UpdatePlayerAck::UpdateRank;
				player->set_rank(user->GetRank());
			}
			player->set_contribution(user->GetContribution());
			break;
		}
		case CAward::REPUTATION:
			mask |= 1 << NetMsg::UpdatePlayerAck::UpdateReputation;
			user->SetReputation(user->GetReputation() + i->num);
			player->set_reputation(user->GetReputation());
			break;
		case CAward::HERO:
		{
			CUtility::AddUserHero(user, i->itemId);
			break;
		}
		case CAward::FRIENDSHIP:
		{
			if (heroClass != 0)
			{
				user->AddHeroFriendship(heroClass, i->num);
			}
			break;
		}
		default:
			break;
		}
	}
	if (user->GetLevel() != level)
	{
		mask |= 1 << NetMsg::UpdatePlayerAck::UpdateLevel;
		player->set_level(user->GetLevel());
	}
	updateUser->set_mask(mask);
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::AddHeroAwards(CUser *user, CHeroInst *hero, std::vector<CAward::AwardItem> &awards)
{
	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		switch (i->type)
		{
		case CAward::EXP_HERO:
			hero->AddExp(i->num);
			break;
		case CAward::STRENGTH_HERO:
		{
			hero->AddStrength(i->num);
			break;
		}
		case CAward::LEADERSHIP_HERO:
			hero->AddLeadership(i->num);
			break;
		case CAward::WISDOM_HERO:
			hero->AddWisdom(i->num);
			break;
		case CAward::CHARM_HERO:
			hero->AddCharm(i->num);
			break;
		case CAward::LUCK_HERO:
			hero->AddLuck(i->num);
			break;
		}
	}
	CUtility::CopyHero(hero, heroAck);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::UseBoxItem(CUser * user, CItemInst * item, uint32_t num)
{
	std::vector<CAward::AwardItem> awards;

	for (uint32_t i = 0; i < num; i++)
	{
		CItemTmpl * tmpl = item->GetItemTmpl();
		if (tmpl == nullptr)
			continue;
		CAward *award = CAwardMgr::get_const_instance().GetAward(tmpl->GetExtId());
		if (award != nullptr)
		{
			AwardData data;
			data.Level = user->GetLevel();
			award->GenerateAward(awards, data);
		}
	}
	NetMsg::GiveAward *giveAward = new NetMsg::GiveAward;
	NetMsg::Award *netAward = giveAward->mutable_award();
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		NetMsg::AwardItem *at = netAward->add_items();
		at->set_id(i->itemId);
		at->set_type(i->type);
		at->set_num(i->num);
	}
	AddUserAwards(user, awards);
	SharedMsg ack(giveAward);
	m_socket->SendMsg(user->GetSession(), ack);
}

bool CUtility::HaveUserAward(std::vector<CAward::AwardItem> &awards)
{
	for (auto i = awards.begin(); i != awards.end(); i++)
	{
		switch (i->type)
		{
		case CAward::ITEM:
		case CAward::EXP:
		case CAward::COIN:
		case CAward::GOLD:
		case CAward::CONTRIBUTION:
		case CAward::REPUTATION:
		case CAward::HERO:
			return true;
		}
	}
	return false;
}

bool CUtility::IsHeroAward(int type)
{
	switch (type)
	{
	case CAward::EXP_HERO:
	case CAward::STRENGTH_HERO:
	case CAward::LEADERSHIP_HERO:
	case CAward::WISDOM_HERO:
	case CAward::CHARM_HERO:
	case CAward::LUCK_HERO:
		return true;
	}
	return false;
}

bool CUtility::AddCityItem(uint32_t cityId, uint32_t itemId, uint32_t num)
{
	CWorldCity * worldCity = CWorldCityMgr::get_const_instance().GetCity(cityId);
	if (worldCity)
	{
		worldCity->AddCityItemInfo(itemId, num);
		//std::string fileName = "addCityItem.txt";
		//CLogs::SetLogFileName(fileName);
		//stringstream str;
		//str << "ctiyID == ";
		//str << cityId ;
		//str << "itemId == ";
		//str << itemId;
		//str << "itemNum == ";
		//str << num;
		//CLogs::WriteLog(str.str().c_str());
	}
	return true;
}

bool CUtility::AddCityHero(uint32_t cityId, const std::string & heroName, uint32_t heroLevel)
{
	CWorldCity * worldCity = CWorldCityMgr::get_const_instance().GetCity(cityId);
	if (worldCity)
	{
		worldCity->AddCityHero(cityId, heroName, heroLevel);
	}
	return true;
}
bool CUtility::AddUserItem(CUser *user, uint32_t itemId, uint32_t num)
{
	CItemInst *item = user->GetItemByTmpl(itemId);
	if (item == nullptr)
	{
		item = new CItemInst;
		if (!item->Init(itemId, num))
		{
			delete item;
			return false;
		}
		else
		{
			user->AddItem(item);
		}
	}
	else
	{
		item->SetNum(item->GetNum() + num);
	}

	NetMsg::ItemAck *itemAck = new NetMsg::ItemAck;
	auto info = itemAck->add_items();
	info->set_id(item->GetItemTmpl()->GetId());
	info->set_uid(item->GetId());
	info->set_num(item->GetNum());
	SharedMsg ack(itemAck);
	m_socket->SendMsg(user->GetSession(), ack);
	return true;
}

void CUtility::GetAwards(CUser *user, std::vector<uint32_t> &awardIds, std::vector<CAward::AwardItem> &awards)
{
	for (auto i = awardIds.begin(); i != awardIds.end(); i++)
	{
		CAward *award = CAwardMgr::get_const_instance().GetAward(*i);
		if (award != nullptr)
		{
			AwardData data;
			data.Round = user->GetCycleBattle().curround();
			data.Level = user->GetLevel();
			award->GenerateAward(awards, data);
		}
	}
}

void CUtility::AddGold(CUser *user, int add)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();

	mask |= 1 << NetMsg::UpdatePlayerAck::UpdateGold;
	user->SetGold(user->GetGold() + add);
	updateUser->set_mask(mask);
	player->set_gold(user->GetGold());
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::AddExp(CUser *user, int add)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();

	mask |= 1 << NetMsg::UpdatePlayerAck::UpdateExp;
	uint32_t level = user->GetLevel();
	user->AddExp(add);
	if (user->GetLevel() != level)
	{
		mask |= 1 << NetMsg::UpdatePlayerAck::UpdateLevel;
		player->set_level(user->GetLevel());
	}
	updateUser->set_mask(mask);
	player->set_exp(user->GetExp());
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::AddCoin(CUser *user, int coin)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();

	mask |= 1 << NetMsg::UpdatePlayerAck::UpdateCoin;
	user->SetCoin(user->GetCoin() + coin);
	updateUser->set_mask(mask);
	player->set_coin(user->GetCoin());
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::AddExploit(CUser *user, int expolit)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();

	mask |= 1 << NetMsg::UpdatePlayerAck::UpdateExploit;
	user->SetExploit(user->GetExploit() + expolit);
	updateUser->set_mask(mask);
	player->set_exploit(user->GetExploit());
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}


bool CUtility::DelPrice(CUser * user, uint32_t priceType, uint32_t price)
{
	switch (priceType)
	{
	case COIN:
		if (price <= user->GetCoin())
		{
			int c = price;
			AddCoin(user, -c);
			return true;
		}
		break;
	case GOLD:
		if (price <= user->GetGold())
		{
			int g = price;
			AddGold(user, -g);
			return true;
		}
		break;
	case PROVISIONS:
		if (price <= user->GetStamina())
		{
			int s = price;
			AddStamina(user, -s);
			return true;
		}
	}
	return false;
}

void CUtility::AddStamina(CUser *user, int add)
{
	NetMsg::UpdatePlayerAck *updateUser = new NetMsg::UpdatePlayerAck;
	uint64_t mask = 0;
	NetMsg::PlayerData *player = updateUser->mutable_player();

	mask |= 1 << NetMsg::UpdatePlayerAck::UpdateStamina;
	user->SetStamina(user->GetStamina() + add);
	updateUser->set_mask(mask);
	player->set_stamina(user->GetStamina());
	SharedMsg ack(updateUser);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::AddAwards(CUser *user, std::vector<uint32_t> &awardIds)
{
	std::vector<CAward::AwardItem> awards;
	GetAwards(user, awardIds, awards);

	//for (auto award = awards.begin(); award != awards.end(); award++)
	{
		NetMsg::GiveAward *giveAward = new NetMsg::GiveAward;
		NetMsg::Award *netAward = giveAward->mutable_award();
		std::vector<uint64_t> heros;
		user->GetCurBattleHeros(heros);
		auto addHero = [=](std::vector<uint64_t> &heros, NetMsg::AwardItem *item) {
			if (IsHeroAward(item->type()))
			{
				for (auto i = heros.begin(); i < heros.end(); i++)
				{
					item->add_heroids(*i);
				}
			}
		};
		for (auto i = awards.begin(); i != awards.end(); i++)
		{
			NetMsg::AwardItem *at = netAward->add_items();
			at->set_id(i->itemId);
			at->set_type(i->type);
			at->set_num(i->num);
			addHero(heros, at);
		}
		AddUserAwards(user, awards);
		if (HaveHeroAward(awards))
		{
			for (auto i = heros.begin(); i != heros.end(); i++)
			{
				CHeroInst *hero = user->GetHero(*i);
				if (hero != nullptr)
					AddHeroAwards(user, hero, awards);
			}
		}
		SharedMsg ack(giveAward);
		m_socket->SendMsg(user->GetSession(), ack);
	}
}

std::vector<uint32_t> CUtility::ConvertToArray(char *str)
{
	std::vector<uint32_t> ret;
	char *p[100];
	int num = SplitLine(p, 100, str, ',');
	for (int i = 0; i < num; i++)
	{
		ret.push_back((uint32_t)atoi(p[i]));
	}
	return ret;
}

void CUtility::SendSysMail(CUser *user, const char *name,const char *title, const char *context, uint32_t awardId, NetMsg::MailType type)
{
	if (user == nullptr)
		return;
	CUserMail mail;
	mail.SetTitle(title);
	mail.SetContent(context);
	mail.SetFromId(1);
	mail.SetFromIcon(1);
	mail.SetFromName(name);
	mail.SetToId(user->GetUserId());
	mail.SetType(type);
	auto &award = mail.GetAward();
	if(awardId != 0)
		GetAward(awardId, user, &award);
	mail.SetSendTime(time(nullptr));
	mail.SetFromTitle(1);
	mail.WriteToDb();

	auto &userMail = user->GetMails();
	userMail.push_back(mail);

	NetMsg::MailInfoAck * ack = new NetMsg::MailInfoAck;
	SharedMsg ackMsg(ack);
	auto mailData = ack->add_mails();
	CUtility::CopyMail(user,&mail, mailData);
	m_socket->SendMsg(user->GetSession(), ackMsg);
}

void CUtility::SendOflineSysMail(uint32_t userId, const char *name,const char *title, const char *context, uint32_t awardId, NetMsg::MailType type)
{
	CUserMail mail;
	mail.SetTitle(title);
	mail.SetContent(context);
	mail.SetFromId(1);
	mail.SetFromIcon(1);
	mail.SetFromName(name);
	mail.SetToId(userId);
	mail.SetType(type);
	mail.SetFromTitle(1);
	auto &award = mail.GetAward();
	award.set_awardid(awardId);
	mail.SetSendTime(time(nullptr));
	mail.WriteToDb();
}

void CUtility::CopyMail(CUser *user,CUserMail * from, NetMsg::MailData * to)
{
	to->set_id(from->GetId());
	to->set_type((NetMsg::MailType)from->GetType());
	auto &award = from->GetAward();
	if (award.awardid() != 0 && award.items_size() <= 0)
	{
		GetAward(award.awardid(), user, &award);
	}
	if (award.items_size() > 0)
	{
		to->set_hasaward(true);
	}
	else
	{
		to->set_hasaward(false);
	}
	to->set_isgetaward(from->GetIsReceived());
	to->set_isread(from->GetIsRead());
	to->set_sendtime(from->GetSendTime());
	to->set_title(from->GetTitle());
	to->set_type(static_cast<::NetMsg::MailType>(from->GetType()));
	auto sender = to->mutable_sender();
	sender->set_icon(from->GetFromIcon());
	sender->set_id(from->GetFromId());
	sender->set_name(from->GetFromName());
	sender->set_title(from->GetFromTitle());
}

std::string CUtility::GetNick(const char *str)
{
	std::string ret;
	std::string tmp = str;
	char *p[3];
	if (3 == SplitLine(p, 3, (char*)tmp.c_str(), '|'))
	{
		ret = p[0];
		//ret.append(p[1]);
		ret.append(p[2]);
	}
	else
	{
		ret = str;
	}
	return ret;
}
void CUtility::SendUserAllResource(CUser *user)
{
	NetMsg::ResourceInfoAck * resAck = new NetMsg::ResourceInfoAck;
	SharedMsg ack(resAck);

	for (auto i = user->GetResourceFields().begin(); i != user->GetResourceFields().end(); i++)
	{
		auto resInst = i->second;
		if ( 0 == resInst->GetResourceClose())
			CUtility::CopyResource(resInst, resAck);
	}
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUtility::GetRandomVec(std::vector<uint32_t> & val)
{
	int n = val.size();
	// initialize numbers.
	std::vector<int> v(n);
	std::iota(v.begin(), v.end(), 0);

	// do random shuffle.
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(v.begin(), v.end(), g);

	for (int i = 0; i < v.size(); i++)
	{
		val[i] = v[i];
	}
}

uint32_t CUtility::GetRandomOneFromVector(std::vector<uint32_t> & vec)
{
	std::vector<uint32_t> valRandom(vec.size());
	GetRandomVec(valRandom);
	uint32_t randomOne = vec[valRandom[0]];
	return randomOne;
}

 void CUtility::SendResTeamRunPath(CUser* user,uint32_t teamPos)
{
	 //NetMsg::TeamRunPathAck *ack = new NetMsg::TeamRunPathAck;
	 //SharedMsg ackMsg(ack);
	 //auto & addData = user->GetAddData();
	 //auto pathInfos = addData.mutable_teampaths();
	 //for (auto i = pathInfos->begin(); i != pathInfos->end(); i++)
	 //{
		//	if(i->position() == teamPos)
		//	{
		//		ack->CopyFrom(*i);
		//	} 
	 //}
	 //m_socket->SendMsg(user->GetSession(), ackMsg);
}

 void CUtility::SendWorldWarStatus(CUser *user, SharedMsg &ackMsg)
 {
	 m_socket->SendMsg(user->GetSession(), ackMsg);
 }

//bool CUtility::CalculateRunPosCommon(NetMsg::RunPathAck *runAck, int dt, int speed)
//{
//	if (dt <= 0)
//		return false;
//
//	NetMsg::PathInfo *pathInfo = runAck->mutable_path();
//
//	int runDis = speed * dt;
//	uint32_t curWayPointInd = pathInfo->curwaypointind();
//	float curPer = pathInfo->curper();
//
//	if (curWayPointInd + 1 < (uint32_t)pathInfo->waypoint_size())
//	{
//		do
//		{
//			uint32_t start = pathInfo->waypoint(curWayPointInd);
//			uint32_t end = pathInfo->waypoint(curWayPointInd + 1);
//			const CWorldPath *path = CWorldPathMgr::get_const_instance().GetPath(start, end);
//			if (path == nullptr)
//			{
//				curWayPointInd = pathInfo->waypoint_size();
//				break;
//			}
//			int len;
//			if (curWayPointInd == pathInfo->curwaypointind())
//				len = (int)((path->GetLength()*(1 - curPer)));
//			else
//				len = path->GetLength();
//			if (runDis > len)
//				runDis -= len;
//			else
//			{
//				curPer = (len - runDis)*1.0f / len;
//				pathInfo->set_curwaypointind(curWayPointInd);
//				pathInfo->set_curper(curPer);
//				break;
//			}
//			curWayPointInd++;
//		} while (curWayPointInd < (uint32_t)pathInfo->waypoint_size() - 1);
//	}
//	if (curWayPointInd + 1 >= (uint32_t)pathInfo->waypoint_size())
//	{
//		runAck->set_state(NetMsg::ArrivedTarget);
//		return true;
//	}
//	return false;
//}