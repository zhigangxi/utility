#ifndef _USER_H_
#define _USER_H_
#include <stdint.h>
#include <bitset>
#include <string>
#include "session.h"
#include "macros.h"
#include <unordered_map>
#include <time.h>
#include "NetMsg.pb.h"
#include "ServerPb.pb.h"
#include <vector>
#include <list>
#include "UserMail.h"

class CHeroInst;
class CUserQuest;
class CUserBattle;
class CItemInst;
class CResourceInst;

struct GeneralSoulCost
{
	GeneralSoulCost()
		:star(0), cost(0)
	{

	}
	uint32_t star;
	uint32_t cost;
};

struct ItemReq
{
	ItemReq()
	:uid(0), num(0)
	{

	}
	uint64_t uid;
	uint32_t num;
};
struct GetFriendshipByGetHero
{
	GetFriendshipByGetHero()
		:star(0), first(0), normal(0)
	{

	}
	uint32_t star;
	uint32_t first;
	uint32_t normal;
};

class CUser
{
public:
	CUser() :
		m_userId(0),
		m_accountId(0),
		m_level(1),
		m_exp(0),
		m_stamina(200),
		m_coin(0),
		m_gold(1000),
		m_vipLevel(0),
		m_channelId(0),
		m_newGuide(0),
		m_curCity(0),
		m_campCity(0),
		m_force(0),
		m_beginRunTime(0),
		m_title(0),
		m_reputation(0),
		m_contribution(0),
		m_rank(0),
		m_battleId(0),
		m_sex(0),
		m_icon(0),
		m_loginTime(0),
		m_exploit(0),
		m_isInWorldWarScene(false)
	{

	}
	virtual ~CUser();
	void Clear()
	{
		m_heros.clear();
		m_battles.clear();
		m_items.clear();
		m_quests.clear();
		m_resourceFields.clear();
	}
	void AddExp(int exp);
	CHeroInst *AddHero(uint32_t heroId,bool isExchangeGeneralSoul = false);
	CResourceInst * AddResourceField(uint32_t fieldId);

	void AddResourceFields(CResourceInst * resField);
	CResourceInst * GetResourceField(uint64_t id);

	void AddHero(CHeroInst *hero);

	void ReadHeros();

	void InitFormation();

	void SetFormation(uint32_t pos, const NetMsg::Formation &);

	bool AddQuest(CUserQuest *quest);

	std::vector<CUserQuest*> *GetQuests()
	{
		return &m_quests;
	}

	CHeroInst *GetHero(uint64_t id);
	void DelHero(uint64_t id);

	uint32_t GetMaxQuestNum()
	{
		return MAX_QUEST_NUM;
	}
	CUserQuest *FindQuest(uint64_t id);
	void GetQuestList(NetMsg::QuestListAck &ack);
	uint32_t GetQuestNum()
	{
		return m_quests.size();
	}
	void AddContribution(int con);

	DEFINE_PROPERTY(std::string, m_nick, Nick);
	DEFINE_PROPERTY(uint32_t, m_userId, UserId);
	DEFINE_PROPERTY(uint32_t, m_accountId, AccountId);
	DEFINE_PROPERTY_REF(UserSession, m_session, Session);

	DEFINE_PROPERTY(uint32_t, m_level, Level);
	DEFINE_PROPERTY(uint32_t, m_exp, Exp);
	DEFINE_PROPERTY(uint32_t, m_stamina, Stamina);//粮草
	DEFINE_PROPERTY(uint32_t, m_provisions, Provisions);//补给品
	DEFINE_PROPERTY(uint32_t, m_wood, Wood);
	DEFINE_PROPERTY(uint32_t, m_stone, Stone);
	DEFINE_PROPERTY(uint32_t, m_iron, ResIron);
	DEFINE_PROPERTY(uint32_t, m_coin, Coin);
	DEFINE_PROPERTY(uint32_t, m_gold, Gold);
	DEFINE_PROPERTY(uint32_t, m_exploit, Exploit); //军功
	DEFINE_PROPERTY(uint32_t, m_vipLevel, VipLevel);

	DEFINE_PROPERTY(uint16_t, m_channelId, ChannelId);

	DEFINE_PROPERTY(uint32_t, m_newGuide, NewGuide);

	DEFINE_PROPERTY(int, m_curCity, CurCity);

	DEFINE_PROPERTY(uint32_t, m_campCity, CampCity);
	DEFINE_PROPERTY(uint32_t, m_force, Force);

	DEFINE_PROPERTY(time_t, m_beginRunTime, BeginRunTime);

	DEFINE_PROPERTY(uint32_t, m_title, Title);
	DEFINE_PROPERTY(uint32_t, m_reputation, Reputation);//声望
	DEFINE_PROPERTY(uint32_t, m_contribution, Contribution);//贡献
	DEFINE_PROPERTY(uint32_t, m_rank, Rank);
	DEFINE_PROPERTY(uint64_t, m_battleId, BattleId);
	DEFINE_PROPERTY(uint32_t, m_sex, Sex);
	DEFINE_PROPERTY(uint64_t, m_icon, Icon);
	DEFINE_PROPERTY(time_t, m_loginTime, LoginTime);
	DEFINE_PROPERTY(bool, m_isInWorldWarScene, IsInWorldWarScene);

	DEFINE_PROPERTY_REF(NetMsg::RunPathAck, m_runPath, RunPath);

	DEFINE_PROPERTY_REF(NetMsg::HeroFormationAck, m_heroFormation, HeroFormation);

	DEFINE_PROPERTY_REF(NetMsg::ShopItemAck, m_shopItem, ShopItem);

	DEFINE_PROPERTY_REF(NetMsg::AdvantureInfoAck, m_advInfoAck, AdvInfoAck);

	DEFINE_PROPERTY_REF(ServerPB::UserCamps, m_camps, Camps);

	DEFINE_PROPERTY_REF(ServerPB::UserCycBattle, m_cycleBattle, CycleBattle);

	DEFINE_PROPERTY_REF(ServerPB::UserAddData, m_addData, AddData);

	DEFINE_PROPERTY_REF(std::list<CUserMail>, m_mails, Mails);

	DEFINE_PROPERTY_REF(NetMsg::ResourceDetailInfoAck, m_resDetailAck,ResourceDetailAck);

	const std::unordered_map<uint64_t, CHeroInst*> &GetHeros()
	{
		return m_heros;
	}
	const std::unordered_map<uint64_t, CResourceInst*> &GetResourceFields()
	{
		return m_resourceFields;
	}

	void ReadResource();
	uint32_t GetAdvEventId();
	CUserBattle *GetBattle(uint64_t id);
	void AddBattle(CUserBattle *battle);
	void ReadBattle();
	const std::unordered_map<uint64_t, CUserBattle*> *GetBattles()
	{
		return &m_battles;
	}
	void AddItem(CItemInst*);
	CItemInst *GetItem(uint64_t id);
	CItemInst *GetItemByTmpl(uint32_t tmplId);

	std::unordered_map<uint64_t, CItemInst*> *GetItems()
	{
		return &m_items;
	}
	void DelItem(uint64_t id);
	void DelResourceField(uint64_t id);
	void ReadItem();
	bool CouldGetFriendshipByLevel(uint32_t heroClass);
	void AddHeroFriendship(uint32_t heroClass, int friendship);
	void SetHeroFriendship(uint32_t heroClass, int exp, uint32_t lv);
	void AddHeroGeneralSoul(uint32_t heroClass, int soulNum);
	void SetHeroGeneralSoul(uint32_t heroClass, int soulNum);

	void AddUserLearnVSCD(uint32_t heroClass, int CDTime);
	void SetUserLearnVSCD(uint32_t heroClass, int CDTime);


	struct Friendship
	{
		Friendship() :exp(0), lv(0)
		{

		}
		int exp;
		uint32_t lv;
	};
	struct GeneralSoul
	{
		GeneralSoul()
			:num(0)
		{

		}
		uint32_t num;
	};

	struct UserLearnCD
	{
		UserLearnCD()
		:heroClass(0),
		 userCD(time(nullptr))
		{

		}
		uint32_t heroClass;
		time_t userCD;
	};

	Friendship GetHeroFriendship(uint32_t heroClass);
	void GetHeroFriendship(NetMsg::HeroFriendshipAck *ack);

	GeneralSoul GetHeroGeneralSoul(uint32_t heroClass);
	void GetHeroGeneralSoul(NetMsg::HeroGeneralSoulAck *ack);

	UserLearnCD GetUserLearnCD(uint32_t heroClass);
	void GetUserLearnCD(NetMsg::UserLearnVsCDAck *ack);
	void GetUserLearnCDToClient(NetMsg::UserLearnVsCDAck *ack);

	bool CouldBeLearn(uint32_t heroClass);
	void doAddUserCD(uint32_t heroClass, int starLv);
	void GetCurBattleHeros(std::vector<uint64_t> &heros);

	std::tuple<int, int> GetFriendShipWhenGetHeroFromTable(uint32_t heroClass);

	bool RecruitHero(uint32_t recruitId);
	void AddRecruitHero(uint32_t recruitId);
	void AddFirstHaveHero(uint32_t heroId);
	void ReadMail();
	
	struct FriendInfo
	{
		uint32_t id;
		bool isNew;
		bool isDel = false;
	};
	void ReadFriend();
	void AddFriend(uint32_t id);
	void DelFriend(uint32_t id)
	{
		for (auto i = m_friends.begin(); i != m_friends.end(); i++)
		{
			if (i->id == id)
			{
				i->isDel = true;
			}
		}
	}
	std::list<FriendInfo> &GetFriend()
	{
		return m_friends;
	}
	static bool InitLevelUpExp();
	static bool InitFriendshipsWhenGetHero();
private:
	bool FirstHaveHero(uint32_t heroId);

	static std::vector<uint32_t> m_leveupExp;

	std::unordered_map<uint64_t, CHeroInst*> m_heros;
	std::unordered_map<uint64_t, CUserBattle*> m_battles;
	std::unordered_map<uint64_t, CItemInst*> m_items;
	std::unordered_map<uint64_t, CResourceInst*> m_resourceFields;

	std::unordered_map<uint32_t, Friendship> m_heroFriendships;//heroClass Friendship
	std::unordered_map<uint32_t, GeneralSoul> m_heroGeneralSouls;  
	std::unordered_map<uint32_t, UserLearnCD> m_userLearnVsCD;
	static std::unordered_map<uint32_t, GetFriendshipByGetHero> m_FriendshipsWhenGetHero;
	
	bool m_readHero = false;
	bool m_readBattle = false;
	bool m_readItem = false;
	bool m_readMail = false;
	bool m_readFriend = false;
	bool m_readResource = false;

	std::list<FriendInfo> m_friends;

	uint32_t m_advEnentId = 0;
	std::vector<CUserQuest*> m_quests;
	const static int FORMATION_HERO_NUM = 5;
	const static int FORMATION_NUM = 3;
	const static int MAX_QUEST_NUM = 5;
};

#endif
