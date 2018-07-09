#pragma once
#include <string>
#include <stdint.h>
#include <list>
#include <vector>
#include "NetMsg.pb.h"
#include "database.h"
#include <string>

class CDataBase;

struct AccountInfo
{
	std::string name;
	std::string passwd;
	uint32_t id;
};

class CUser;
class CHeroTmpl;
class CItemTmpl;
class CForce;
class CSkillTmpl;
class CWorldPath;
class CWorldCity;
class CQuest;
struct ConsAgriculture;
struct ConsBusiness;
struct ConsMilitary;
struct BattleCateran;
struct QuestVisit;
struct GeneralSoulCost;
struct GetFriendshipByGetHero;

class CRank;
class CRankType;
class CUserTopList;
class CTitle;
class CAward;
class CEnemyTeam;
class CEnemy;
class CHeroClass;
struct AwardGroup;
class CShopTmpl;
class CShopGoods;
class CWorldWarScene;
class CAdvantureConf;
class CAdvantureEvent;
class CEvolveCost;
class CUserMail;
class CLearnAndVsCD;
class CResourceTmpl;
class CResourcePos;

struct LearnPropertyInfo;

class CReadData
{
public:
	template<typename Type>
	Type GetVal(int pos)
	{
		Type t = (Type)0;
		try
		{
			t = boost::lexical_cast<Type>(m_datas[pos]);
		}
		catch (...)
		{
			return t;
		}
		return t;
	}
	char *GetVal(int pos)
	{
		return (char*)m_datas[pos].c_str();
	}
	void SetData(CDataBase *db,int row);
private:
	std::vector<std::string> m_datas;
};

class CReadWriteUser
{
public:
	static bool CreateAccount(AccountInfo &);
	static void ReadAccount(AccountInfo &);
	static void ReadUser(CUser &);
	static void WriteUser(CUser &);
	static bool CreateUser(CUser &);

	static bool ReadBadWrods(std::list<std::string> &);

	static void InitDb();

	static void ReadServerList(std::list<NetMsg::ServerInfo> &serverList);

	static void ReadHero(std::list<CHeroTmpl*> &heros);

	static void ReadItem(std::list<CItemTmpl*> &items);

	static bool ReadForce(std::list<CForce*> &forces);

	static void ReadSkill(std::list <CSkillTmpl*> &skills);

	static void ReadUserHero(CUser *user);

	static void ReadUserResourceField(CUser * user);

	static void ReadUId();

	static void ReadWorldPath(std::list<CWorldPath*> &worldPaths);

	static void ReadWorldCity(std::list<CWorldCity*> &citys);

	static void ReadQuest(std::list<CQuest*> &quests);

	static void ReadConsAgriculture(std::list<ConsAgriculture*> &dataList);

	static void ReadConsBusiness(std::list<ConsBusiness*> &dataList);

	static void ReadBattleCateran(std::list<BattleCateran*> &battls);

	static void ReadConsMilitary(std::list<ConsMilitary*> &dataList);

	static bool ReadOwnReputaion(CUserTopList * topList);

	static bool ReadOwnLevel(CUserTopList * topList);
	
	static bool ReadRanks(std::list<CRank*> &ranks);

	static bool ReadRankReputation(std::list<CRankType*>& rankreputations);

	static bool ReadRankLevel(std::list<CRankType*>& ranklevel);

	static bool ReadTitles(std::list<CTitle*> &titles);

	static void ReadAwards(std::list < CAward*> &awards);

	static void ReadAwardGroups(std::list<AwardGroup*> &awardGroups);

	static void ReadShop(std::list<CShopTmpl*> &shops);
	static void ReadShopGoods(std::list<CShopGoods*> &shopGoods);

	static void ReadWorldWarScene(CWorldWarScene* scene);

	static void ReadUserQuest(CUser*);

	static bool ReadEnemyTeam(std::list<CEnemyTeam*> &teams);

	static bool ReadEnemy(std::list<CEnemy*> &enemys);

	static void ReadUserBattle(CUser *user);
	static void WriteUserBattle(CUser *user);

	static void GetUserNick(std::string &nick);
	static void UpdateTmpUseNick();
	static void UseNick(std::string &nick);

	static void ReadUserItem(CUser *user);
	static void WriteUserItem(CUser *user);

	static void WriteUserHero(CUser *user);
	static void WriteUserResourceField(CUser *user);

	template<typename Type> static void MakeInCondition(Type id, std::string &str)
	{
		if (!str.empty())
			str.append(",");
		str.append(std::to_string(id));
	}

	static void ReadHeroClass(std::list<CHeroClass*> &heros);

	static bool ReadQuestVisit(std::list<QuestVisit*> &quests);

	static bool ReadAdvConf(CAdvantureConf *);
	static void ReadAdvEvent(std::list<CAdvantureEvent*> &events);

	static void ReadEvolveCost(std::list<CEvolveCost*> &costs);
	static void ReadEvolve(std::list<std::pair<uint32_t, uint32_t>> &fromTos);

	static bool ReadLearnSkill(int &friendship,int &critBase,int &critTimes,int &generalSoul);
	static bool ReadLearnPropeyty(std::list<LearnPropertyInfo*> &infos);

	static bool ReadHeroPropertyExp();

	static bool ReadData(std::list<CReadData> &datas, std::vector<const char *> &fieldNames,const char *table,const char *con = nullptr);

	static bool UpdateWorldCity(CWorldCity *city);

	static void WriteMail(CUserMail *mail);
	static void UpdateMailIsRead(uint32_t mailId);
	static void DelMail(CUser *user);
	static void ReadMail(CUser *user);
	static void UpdateMailGiveAward(uint32_t mailId);
	static void UpdateUserTitle(uint32_t userId, uint32_t title);
	static bool ReadExchangeGeneralSoulCost(std::list<GeneralSoulCost*>& costs);
	static bool ReadFriendshipWhenGetHero(std::list<GetFriendshipByGetHero*>& friendShips);
	static bool ReadHeroFriendshipLevelUp(std::list<CLearnAndVsCD*>& infos);
	static bool ReadResourceField(std::list<CResourceTmpl*> & infos);
	static bool ReadResourcePos(std::list<CResourcePos*> & infos);
private:
	static void WriteUserQuest(CUser *);
	static void WriteUserFriend(CUser *);
	static void WriteOnlineLog(CUser *);

	static CDataBase *m_db;
};

