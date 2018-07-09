#include "ReadWriteUser.h"
#include "user.h"
#include "database.h"
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include "HeroTmpl.h"
#include "GlobalVal.h"
#include "ItemTmpl.h"
#include "Force.h"
#include "SkillTmpl.h"
#include "HeroInst.h"
#include "utility.h"
#include "WorldPath.h"
#include "WorldCity.h"
#include "Quest.h"
#include "Rank.h"
#include "RankReputation.h"
#include "UserTopList.h"
#include "Title.h"
#include "Award.h"
#include "UserQuest.h"
#include "EnemyTeam.h"
#include "UserBattle.h"
#include "ItemInst.h"
#include "Shop.h"
#include "WorldWarScene.h"
#include "WorldWarSceneBuilding.h"
#include "Advanture.h"
#include "Evolve.h"
#include "LearnProperty.h"
#include "UserMail.h"
#include "LearnAndVsCD.h"
#include "ResourceTmpl.h"
#include "ResourceInst.h"

using namespace std;
CDataBase *CReadWriteUser::m_db;

bool CReadWriteUser::CreateAccount(AccountInfo &account)
{
	boost::format fmt("select password from account where name='%1%'");
	string sql = (fmt%account.name).str();
	if(!m_db->Query(sql.c_str()))
		return false;

	char *pass = m_db->GetVal(0);
	if (pass != nullptr)
		return false;

	fmt.parse("INSERT INTO account (name,password) VALUES ('%1%', '%2%') RETURNING id;");
	sql = (fmt%account.name%account.passwd).str();
	if (!m_db->Query(sql.c_str()))
		return false;

	uint32_t id = m_db->GetVal<uint32_t>(0);
	if (id == 0)
		return false;

	account.id = id;
	return true;
}

void CReadWriteUser::ReadAccount(AccountInfo &account)
{
	boost::format fmt("select id,password from account where name='%1%'");
	string sql = (fmt%account.name).str();
	if (!m_db->Query(sql.c_str()))
		return;

	char *pass = m_db->GetVal(1);
	if (pass != nullptr)
	{
		account.passwd = pass;
		account.id = m_db->GetVal<uint32_t>(0);
	}
}

void CReadWriteUser::ReadUser(CUser &user)
{
	if (user.GetAccountId() == 0)
		return;

	boost::format fmt("select id,nick,level,exp,stamina,coin,gold,exploit,vip,new_guide,city,camp_city,force,run_path,"
		"run_begin_time,hero_formation,title,reputation,contribution,rank,battle_id,hero_friendship,hero_generalsoul,shops,"
		"adv_info_ack,camp,sex,icon,cyc_battle,add_data,learn_vs_cd,provisions,wood,stone,iron"
		" from user_info where account_id=%1%");
	string sql = (fmt%user.GetAccountId()).str();

	if (!m_db->Query(sql.c_str()))
		return;
	if (m_db->GetRowsNum() <= 0)
		return;

	user.SetUserId(m_db->GetVal<uint32_t>("id"));
	user.SetNick(m_db->GetVal("nick"));
	user.SetLevel(m_db->GetVal<int>("level"));
	user.SetExp(m_db->GetVal<uint32_t>("exp"));
	user.SetStamina(m_db->GetVal<uint32_t>("stamina"));
	user.SetProvisions(m_db->GetVal<uint32_t>("provisions"));
	user.SetWood(m_db->GetVal<uint32_t>("wood"));
	user.SetStone(m_db->GetVal<uint32_t>("stone"));
	user.SetResIron(m_db->GetVal<uint32_t>("iron"));
	user.SetCoin(m_db->GetVal<uint32_t>("coin"));
	user.SetGold(m_db->GetVal<uint32_t>("gold"));
	user.SetExploit(m_db->GetVal<uint32_t>("exploit"));
	user.SetVipLevel(m_db->GetVal<uint32_t>("vip"));
	user.SetNewGuide(m_db->GetVal<uint32_t>("new_guide"));
	user.SetCurCity(m_db->GetVal<int>("city"));
	user.SetCampCity(m_db->GetVal<uint32_t>("camp_city"));
	user.SetForce(m_db->GetVal<uint32_t>("force"));
	user.SetTitle(m_db->GetVal<uint32_t>("title"));
	user.SetReputation(m_db->GetVal<uint32_t>("reputation"));
	user.SetContribution(m_db->GetVal<uint32_t>("contribution"));
	user.SetRank(m_db->GetVal<uint32_t>("rank"));
	user.SetBattleId(m_db->GetVal<uint32_t>("battle_id"));
	user.SetSex(m_db->GetVal<uint32_t>("sex"));
	user.SetIcon(m_db->GetVal<uint32_t>("icon"));

	NetMsg::RunPathAck &runPath = user.GetRunPath();
	char *strRunPath = m_db->GetVal("run_path");
	if (strRunPath != nullptr && strlen(strRunPath) > 2)
		CUtility::StrToMsg(strRunPath, &runPath);

	user.SetBeginRunTime(m_db->GetVal<time_t>("run_begin_time"));

	NetMsg::HeroFormationAck &heroFormation = user.GetHeroFormation();
	char *strHeroFor = m_db->GetVal("hero_formation");
	if (strHeroFor == nullptr || strlen(strHeroFor) <= 0)
		user.InitFormation();
	else
		CUtility::StrToMsg(strHeroFor, &heroFormation);

	NetMsg::AdvantureInfoAck &advInfo = user.GetAdvInfoAck();
	char *strAdvInfo = m_db->GetVal("adv_info_ack");
	if (strAdvInfo != nullptr && strlen(strAdvInfo) > 2)
		CUtility::StrToMsg(strAdvInfo, &advInfo);

	NetMsg::HeroFriendshipAck friAck;
	char *strFriAck = m_db->GetVal("hero_friendship");
	if (strFriAck != nullptr && strlen(strFriAck) > 1)
	{
		CUtility::StrToMsg(strFriAck, &friAck);
		for (int i = 0; i < friAck.herofri_size(); i++)
		{
			NetMsg::HeroFriendship *f = friAck.mutable_herofri(i);
			user.SetHeroFriendship(f->heroclass(), f->friendshipexp(),f->friendshiplv());
		}
	}

	//add by lyle
	NetMsg::HeroGeneralSoulAck soulAck;
	char *strSoulAck = m_db->GetVal("hero_generalsoul");
	if(strSoulAck != nullptr && strlen(strSoulAck) > 1)
	{
		CUtility::StrToMsg(strSoulAck, &soulAck);
		for (int i = 0; i < soulAck.herosoul_size(); i++)
		{
			NetMsg::HeroGeneralSoul *s = soulAck.mutable_herosoul(i);
			user.SetHeroGeneralSoul(s->heroclass(), s->num());
		}
	}

	NetMsg::UserLearnVsCDAck CDAck;
	char *strCDAck = m_db->GetVal("learn_vs_cd");
	if (strCDAck != nullptr && strlen(strCDAck) > 1)
	{
		CUtility::StrToMsg(strCDAck, &CDAck);
		for (int i = 0; i < CDAck.cdinfo_size(); i++)
		{
			NetMsg::UserLearnVsCDInfo *cd = CDAck.mutable_cdinfo(i);
			user.SetUserLearnVSCD(cd->heroclass(),cd->userlearnvscd());
		}
	}
	ServerPB::UserCamps &camps = user.GetCamps();
	char *strCamps = m_db->GetVal("camp");
	if (strCamps != nullptr && strlen(strCamps) > 2)
		CUtility::StrToMsg(strCamps, &camps);

	auto &cycBattle = user.GetCycleBattle();
	char *strCycBatt = m_db->GetVal("cyc_battle");
	if (strCycBatt != nullptr && strlen(strCycBatt) > 2)
		CUtility::StrToMsg(strCycBatt, &cycBattle);

	auto &addData = user.GetAddData();
	char *strAddData = m_db->GetVal("add_data");
	CUtility::StrToMsg(strAddData, &addData);
}

void CReadWriteUser::WriteUser(CUser &user)
{
	if (user.GetUserId() == 0)
		return;

	boost::format fmt("update user_info set nick='%1%',level=%2%,exp=%3%,stamina=%4%,coin=%5%,gold=%6%,vip=%7%,new_guide=%8%,"
		"city=%9%,camp_city=%10%,force=%11%,run_path='%12%',run_begin_time=%13%,hero_formation='%14%',title=%15%,reputation=%16%,"
		"contribution=%17%,rank=%18%,battle_id=%19%,hero_friendship='%20%',shops='%21%',adv_info_ack='%22%',camp='%23%',"
		"sex=%24%,icon=%25%,cyc_battle='%26%',add_data='%27%',last_login=to_timestamp(%28%), exploit=%29%, hero_generalsoul='%30%',"
		"learn_vs_cd='%31%',provisions=%32%,wood=%33%,stone=%34%,iron=%35% "
		"where id=%36%");

	string strRunPath;
	NetMsg::RunPathAck &runPath = user.GetRunPath();
	if (runPath.path().startpath() != 0)
	{
		CUtility::MsgToStr(&runPath, strRunPath);
	}
	string strHeroFromation;
	NetMsg::HeroFormationAck &heroFormation = user.GetHeroFormation();
	CUtility::MsgToStr(&heroFormation, strHeroFromation);

	string heroFri;
	NetMsg::HeroFriendshipAck friAck;
	user.GetHeroFriendship(&friAck);
	CUtility::MsgToStr(&friAck, heroFri);

	string generalSoul;
	NetMsg::HeroGeneralSoulAck generalSoulAck;
	user.GetHeroGeneralSoul(&generalSoulAck);
	CUtility::MsgToStr(&generalSoulAck, generalSoul);

	string learnVsCD;//lyle
	NetMsg::UserLearnVsCDAck learnVsCDAck;
	user.GetUserLearnCD(&learnVsCDAck);
	CUtility::MsgToStr(&learnVsCDAck, learnVsCD);

	NetMsg::AdvantureInfoAck &advInfo = user.GetAdvInfoAck();
	string strAdvInfo;
	CUtility::MsgToStr(&advInfo, strAdvInfo);

	string strShops;
	string camp;
	ServerPB::UserCamps &userCamps = user.GetCamps();
	CUtility::MsgToStr(&userCamps, camp);

	string strCycBatt;
	auto &cycBattle = user.GetCycleBattle();
	CUtility::MsgToStr(&cycBattle, strCycBatt);

	string strAddData;
	auto &addData = user.GetAddData();
	CUtility::MsgToStr(&addData, strAddData);

	string sql = (fmt%user.GetNick() % user.GetLevel() % user.GetExp() % user.GetStamina() % user.GetCoin() % user.GetGold() % user.GetVipLevel()
		% user.GetNewGuide() % user.GetCurCity() % user.GetCampCity() % user.GetForce() % strRunPath % user.GetBeginRunTime() % strHeroFromation
		% user.GetTitle() % user.GetReputation() % user.GetContribution() % user.GetRank() %user.GetBattleId() % heroFri % strShops % strAdvInfo
		% camp %user.GetSex()%user.GetIcon() %strCycBatt % strAddData % time(nullptr) % user.GetExploit() % generalSoul % learnVsCD 
		% user.GetProvisions() % user.GetWood() % user.GetStone() % user.GetResIron() % user.GetUserId()).str() ;

	if (!m_db->Query(sql.c_str()))
	{
		cout << "WriteUser error:"<<sql<<"," <<m_db->GetErrMsg() << endl;
		return;
	}

	WriteUserHero(&user);
	WriteUserQuest(&user);
	WriteUserBattle(&user);
	WriteUserItem(&user);
	WriteUserResourceField(&user);
	DelMail(&user);
	WriteUserFriend(&user);
	WriteOnlineLog(&user);
}

void CReadWriteUser::WriteOnlineLog(CUser *user)
{
	if (user->GetLoginTime() == 0)
		return;

	boost::format fmt("insert into online_log(user_id,login_time) values (%1%,to_timestamp(%2%))");
	fmt%user->GetUserId() % user->GetLoginTime();
	
	if (!m_db->Query(fmt.str().c_str()))
	{
		cout << "write onine log error:" << m_db->GetErrMsg() << endl;
		return;
	}
}

void CReadWriteUser::WriteUserQuest(CUser *user)
{
	boost::format fmt;
	CUserQuest *quest;
	std::vector<CUserQuest*> *quests = user->GetQuests();
	for (auto i = quests->begin(); i != quests->end(); i++)
	{
		quest = *i;
		std::string questData;
		quest->GetDataStr(questData);
		if (quest->GetUpdateToDb())
		{
			fmt.parse("update user_quest set quest_id=%1%,state=%2%,data='%3%' where id=%4%");
			fmt%quest->GetQuestId() % quest->GetQuestState() % questData % quest->GetId();
		}
		else
		{
			fmt.parse("insert into user_quest (id,quest_id,state,data,user_id) values "
				"(%1%,%2%,%3%,'%4%',%5%)");
			fmt%quest->GetId() % quest->GetQuestId() % quest->GetQuestState() % questData%user->GetUserId();
		}

		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << "write user quest error:" << m_db->GetErrMsg() << endl;
		}
	}
}

void CReadWriteUser::WriteUserFriend(CUser *user)
{
	auto &friends = user->GetFriend();
	string delF;
	string insertF;
	for (auto i = friends.begin(); i != friends.end(); i++)
	{
		if (i->isDel)
			MakeInCondition(i->id, delF);
		else if (i->isNew)
		{
			if (!insertF.empty())
				insertF.append(",");
			insertF.append("(");
			insertF.append(std::to_string(user->GetUserId()));
			insertF.append(",");
			insertF.append(std::to_string(i->id));
			insertF.append(")");
		}
	}
	if (!delF.empty())
	{
		boost::format fmt("delete from user_friend where user_id = %1% and friend_id in (%2%)");
		fmt%user->GetUserId() % delF;
		if (!m_db->Query(fmt.str().c_str()))
		{
			std::cout << m_db->GetErrMsg() << endl;
			return;
		}
	}
	if (!insertF.empty())
	{
		boost::format fmt("insert into user_friend (user_id,friend_id) values %1%");
		fmt % insertF;
		if (!m_db->Query(fmt.str().c_str()))
		{
			std::cout << m_db->GetErrMsg() << endl;
			return;
		}
	}
	//insert into user_friend (user_id,friend_id) values (1,100), (2,200)
}

void CReadWriteUser::ReadUserQuest(CUser *user)
{
	boost::format fmt("select data from user_quest where user_id=%1%");
	fmt%user->GetUserId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		if (user->GetQuestNum() >= user->GetMaxQuestNum())
			break;
		CUserQuest *quest = new CUserQuest;
		quest->InitData(m_db->GetVal(i, 0));
		user->AddQuest(quest);
	}
}

bool CReadWriteUser::ReadEnemyTeam(std::list<CEnemyTeam*>& teams)
{
	if (!m_db->Query("select id,star,promotion,formation,member1,level1,skill1,skill_level1,member2,level2,skill2,skill_level2,"
		"member3,level3,skill3,skill_level3,member4,level4,skill4,skill_level4,member5,level5,skill5,skill_level5,award_id,cost from enemy_team"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	std::vector<string> split;
	char *stars[5];
	char *promotions[5];
	char *formation[5];
	char name[64];
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		char *strStar = m_db->GetVal(i, "star");
		char *strPro = m_db->GetVal(i, "promotion");
		char *strFor = m_db->GetVal(i, "formation");
		if (strStar == nullptr || strPro == nullptr || strFor == nullptr)
			return false;
		int num = CUtility::SplitLine(stars, 5, strStar, ',');
		if (num <= 0)
			return false;
		if (num != CUtility::SplitLine(promotions, 5, strPro, ','))
			return false;
		if (num != CUtility::SplitLine(formation, 5, strFor, ','))
			return false;
		CEnemyTeam *team = new CEnemyTeam;
		team->SetId(m_db->GetVal<uint32_t>(i, "id"));
		team->SetAwardId(m_db->GetVal<uint32_t>(i, "award_id"));
		team->SetCost(m_db->GetVal<uint32_t>(i, "cost"));
		auto enemys = team->GetEnemys();
		for (int j = 0; j < num; j++)
		{
			EnemyInfo info;
			info.star = (uint32_t)atoi(stars[j]);
			info.promotion = (uint32_t)atoi(promotions[j]);
			info.formation = (uint32_t)atoi(formation[j]);
			snprintf(name, sizeof(name), "member%d", j + 1);
			info.conditionPos = m_db->GetVal<uint32_t>(i, name);
			snprintf(name, sizeof(name), "level%d", j + 1);
			char *levels[2];
			char *strLevel = m_db->GetVal(i, name);
			if (2 != CUtility::SplitLine(levels, 2, strLevel, ','))
				return false;
			info.beginLevel = atoi(levels[0]);
			info.endLevel = atoi(levels[1]);
			char *skills[8];
			char *skillLevel[8];
			snprintf(name, sizeof(name), "skill%d", j + 1);
			char *strSkill = m_db->GetVal(i, name);
			snprintf(name, sizeof(name), "skill_level%d", j + 1);
			char *strSkillLevel = m_db->GetVal(i, name);
			int skillNum = CUtility::SplitLine(skills, 8, strSkill,',');
			if (skillNum != CUtility::SplitLine(skillLevel, 8, strSkillLevel, ','))
				return false;
			for (int i = 0; i < skillNum; i++)
			{
				info.skills.push_back((uint32_t)atoi(skills[i]));
				info.skillLevels.push_back((uint32_t)atoi(skillLevel[i]));
			}
			enemys->push_back(info);
		}
		
		teams.push_back(team);
	}
	return true;
}

bool CReadWriteUser::ReadEnemy(std::list<CEnemy*>& enemys)
{
	if (!m_db->Query("select id,force,sex,soldier,strength,leadership,wisdom,charm,luck,vs_health from enemy"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	std::vector<string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CEnemy *enemy = new CEnemy;
		enemy->SetId(m_db->GetVal<uint32_t>(i, "id"));
		enemy->SetForce(m_db->GetVal<uint32_t>(i, "force"));
		enemy->SetSex(m_db->GetVal<uint32_t>(i, "sex"));
		enemy->SetSoldierType(m_db->GetVal<uint32_t>(i, "soldier"));
		enemy->SetStrength(m_db->GetVal<uint32_t>(i, "strength"));
		enemy->SetLeadership(m_db->GetVal<uint32_t>(i, "leadership"));
		enemy->SetWisdom(m_db->GetVal<uint32_t>(i, "wisdom"));
		enemy->SetCharm(m_db->GetVal<uint32_t>(i, "charm"));
		enemy->SetLuck(m_db->GetVal<uint32_t>(i, "luck"));
		enemy->SetVsHealth(m_db->GetVal<uint32_t>(i, "vs_health"));
		enemys.push_back(enemy);
	}
	return true;
}

void CReadWriteUser::ReadUserBattle(CUser * user)
{
	boost::format fmt("select id,type,type_data,battle_data,award_id from user_battle where user_id=%1%");
	fmt%user->GetUserId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CUserBattle *battle = new CUserBattle;
		battle->Init( m_db->GetVal<uint32_t>(i, "type"), m_db->GetVal<uint32_t>(i, "type_data"),m_db->GetVal(i,"battle_data"));
		battle->SetAwardId(m_db->GetVal<uint32_t>(i, "award_id"));
		user->AddBattle(battle);
	}
}

void CReadWriteUser::WriteUserBattle(CUser * user)
{
	boost::format fmt;
	CUserBattle *battle;
	auto battles = user->GetBattles();
	for (auto i = battles->begin(); i != battles->end(); i++)
	{
		battle = i->second;
		std::string battleData;
		battle->GetDataStr(battleData);
		
		if (battle->GetDelBattle())
		{
			fmt.parse("delete from user_battle where id=%1%");
			fmt % battle->GetId();
		}
		else if (battle->GetUpdateToDb())
		{
			fmt.parse("update user_battle set battle_data='%1%' where id=%2%");
			fmt%battleData%battle->GetId();
		}
		else
		{
			fmt.parse("insert into user_battle (id,user_id,type,type_data,battle_data,award_id) values "
				"(%1%,%2%,%3%,%4%,'%5%',%6%)");
			fmt%battle->GetId() % user->GetUserId() % battle->GetType() % battle->GetTypeData() % battleData % battle->GetAwardId();
		}

		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << "write user battle error:" << m_db->GetErrMsg() << endl;
		}
	}
}

void CReadWriteUser::GetUserNick(std::string & nick)
{
	if (!m_db->Query("select id,name from name where state=0 limit 1"))
	{
		cout << m_db->GetErrMsg() << endl;
		return;
	}
	if (m_db->GetRowsNum() <= 0)
	{
		cout << "can not get nick" << endl;
		return;
	}
	int id = m_db->GetVal<int>("id");
	nick = m_db->GetVal("name");

	char sql[256];
	snprintf(sql, sizeof(sql), "update name set state=1 where id=%d", id);
	if (!m_db->Query(sql))
	{
		cout << m_db->GetErrMsg() << endl;
		return;
	}
}

void CReadWriteUser::UpdateTmpUseNick()
{
	if (!m_db->Query("update name set state=0 where state=1"))
	{
		cout << m_db->GetErrMsg() << endl;
	}
}

void CReadWriteUser::UseNick(std::string & nick)
{
	char sql[256];
	snprintf(sql, sizeof(sql), "update name set state=2 where name='%s'", nick.c_str());
	if (!m_db->Query(sql))
	{
		cout << m_db->GetErrMsg() << endl;
		return;
	}
}

void CReadWriteUser::ReadUserItem(CUser * user)
{
	boost::format fmt("select id,item_id,item_num from user_item where user_id=%1%");
	fmt%user->GetUserId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CItemInst *item = new CItemInst;
		item->SetId(m_db->GetVal<uint64_t>(i, "id"));
		if (!item->Init(m_db->GetVal<uint32_t>(i, "item_id"), m_db->GetVal<uint32_t>(i, "item_num")))
		{
			delete item;
			return;
		}
		item->SetUpdateToDb(true);
		user->AddItem(item);
	}
}

void CReadWriteUser::WriteUserHero(CUser *user)
{
	boost::format fmt;

	const std::unordered_map<uint64_t, CHeroInst*> heros = user->GetHeros();

	CHeroInst *hero;
	std::string skills;
	char strength[32];
	char leadership[32];
	char wisdom[32];
	char charm[32];
	char luck[32];
	auto fotmatAttr = [](HeroAttr attr, char *str) {
		sprintf(str, "'%d,%d'", attr.exp, attr.lv);
	};

	string inCondition;

	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		hero = i->second;
		MakeInCondition(hero->GetId(), inCondition);

		auto s = hero->GetSkills();
		skills.clear();
		for (size_t i = 0; i < s.size(); i++)
		{
			char buf[32];
			if (i == 0)
				snprintf(buf, sizeof(buf), "%d,%d,%d", s[i].GetSkillId(), s[i].GetLevel(), s[i].GetExp());
			else
				snprintf(buf, sizeof(buf), ",%d,%d,%d", s[i].GetSkillId(), s[i].GetLevel(), s[i].GetExp());
			skills.append(buf);
		}
		fotmatAttr(hero->GetStrength(), strength);
		fotmatAttr(hero->GetLeadership(), leadership);
		fotmatAttr(hero->GetWisdom(), wisdom);
		fotmatAttr(hero->GetCharm(), charm);
		fotmatAttr(hero->GetLuck(), luck);
		if (hero->GetUpdateToDb())
		{
			fmt.parse("update user_hero set hero_id=%1%,level=%2%,star_level=%3%,strength=%4%,leadership=%5%,wisdom=%6%,charm=%7%,"
				"luck=%8%,cd_time=%9%,exp=%10%,skills='%11%' where id=%12%");
			fmt%hero->GetHeroId() % hero->GetLevel() % hero->GetStarLevel() % strength % leadership
				% wisdom % charm % luck % hero->GetCDTime() % hero->GetExp() % skills
				% hero->GetId();
		}
		else
		{
			fmt.parse("insert into user_hero (hero_id,level,star_level,strength,leadership,wisdom,charm,luck ,id,cd_time,user_id,exp,skills) values "
				"(%1%,%2%,%3%,%4%,%5%,%6%,%7%,%8% ,%9%,%10%,%11%,%12%,'%13%')");
			fmt%hero->GetHeroId() % hero->GetLevel() % hero->GetStarLevel() % strength % leadership
				% wisdom % charm % luck % hero->GetId() % hero->GetCDTime()
				% user->GetUserId() % hero->GetExp() % skills;
		}

		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << "write user hero error:" << m_db->GetErrMsg() << endl;
			return;
		}
	}
	if (!heros.empty())
	{
		fmt.parse("delete from user_hero where user_id = %1% and id not in (%2%)");
		fmt % user->GetUserId() % inCondition;
		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << m_db->GetErrMsg() << endl;
		}
	}
}

void CReadWriteUser::WriteUserResourceField(CUser *user)
{
	boost::format fmt;

	const std::unordered_map<uint64_t, CResourceInst*> resFields = user->GetResourceFields();
	CResourceInst *resField;
	string inCondition;
	for (auto i = resFields.begin(); i != resFields.end(); i++)
	{
		resField = i->second;
		MakeInCondition(resField->GetId(), inCondition);


		if (resField->GetUpdateToDb())
		{
			fmt.parse("update user_resource set resource_id=%1%,create_time=%2%,resource_cd=%3%,pos=%4% ,resource_num=%5% ,left_resource_num=%6%,is_close=%7% where id=%8%");
			fmt%resField->GetResourceID() % resField->GetCreateTime() % resField->GetResCD() % resField->GetPosID() % resField->GetResourceNum() % resField->GetLeftResourceNum() % resField->GetResourceClose() % resField->GetId();
		}
		else
		{
			fmt.parse("insert into user_resource (id, resource_id, create_time, user_id, resource_cd, pos,resource_num,left_resource_num,is_close) values (%1%,%2%,%3%,%4%,%5%,%6%,%7%,%8%,%9%)");
			fmt % resField->GetId() %resField->GetResourceID() % resField->GetCreateTime() % user->GetUserId() % resField->GetResCD() % resField->GetPosID() % resField->GetResourceNum() % resField->GetLeftResourceNum() % resField->GetResourceClose();
		}

		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << "write user resource error:" << m_db->GetErrMsg() << endl;
			return;
		}
	}
	if (!resFields.empty())
	{
		fmt.parse("delete from user_resource where user_id = %1% and id not in (%2%)");
		fmt % user->GetUserId() % inCondition;
		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << m_db->GetErrMsg() << endl;
		}
	}
}
void CReadWriteUser::WriteUserItem(CUser * user)
{
	boost::format fmt;
	CItemInst *item;
	auto items = user->GetItems();
	
	string inCondition;
	for (auto i = items->begin(); i != items->end(); i++)
	{
		item = i->second;
		MakeInCondition(item->GetId(), inCondition);
		if (item->GetUpdateToDb())
		{
			fmt.parse("update user_item set item_num=%1% where id=%2%");
			fmt%item->GetNum() % item->GetId();
		}
		else
		{
			fmt.parse("insert into user_item (id,user_id,item_id,item_num) values (%1%,%2%,%3%,%4%)");
			fmt % item->GetId() % user->GetUserId() % item->GetItemTmpl()->GetId() % item->GetNum();
		}

		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << "write user item error:" << m_db->GetErrMsg() << endl;
		}
	}
	if (!items->empty())
	{
		fmt.parse("delete from user_item where user_id = %1% and id not in (%2%)");
		fmt % user->GetUserId() % inCondition;
		if (!m_db->Query(fmt.str().c_str()))
		{
			cout << m_db->GetErrMsg() << endl;
		}
	}
}

void CReadWriteUser::ReadHeroClass(std::list<CHeroClass*>& heros)
{
	if (!m_db->Query("select id,force,city,building,gift_type,interest,star,base_star,exp,property_exp,property_lv,learnable,skill_id,"
		"skill_level,skill_exp,vs_health,sex from hero_class"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CHeroClass *hero= new CHeroClass;
		hero->SetId(m_db->GetVal<uint32_t>(i, "id"));
		hero->SetForce(m_db->GetVal<uint32_t>(i, "force"));
		hero->SetCity(m_db->GetVal<uint32_t>(i, "city"));
		hero->SetBuilding(m_db->GetVal<uint32_t>(i, "building"));
		hero->SetGiftInterest(m_db->GetVal(i, "gift_type"), m_db->GetVal(i, "interest"));
		hero->SetBaseStar(m_db->GetVal<uint32_t>(i, "base_star"));
		HeroAttr attr;
		attr.exp = m_db->GetVal<uint32_t>(i, "exp");
		attr.lv = m_db->GetVal<uint32_t>(i, "star");
		hero->SetStar(attr);
		hero->SetProperty(m_db->GetVal(i, "property_exp"),m_db->GetVal(i,"property_lv"));
		hero->SetCanLearnPro(m_db->GetVal(i, "learnable"));
		hero->SetSkill(m_db->GetVal(i, "skill_id"), m_db->GetVal(i, "skill_level"), m_db->GetVal(i, "skill_exp"));
		hero->SetVsHealth(m_db->GetVal<uint32_t>(i, "vs_health"));
		hero->SetSex(m_db->GetVal<uint32_t>(i, "sex"));
		heros.push_back(hero);
	}
}

bool CReadWriteUser::ReadQuestVisit(std::list<QuestVisit*> &quests)
{
	if (!m_db->Query("select title,in_selfforce_chance,in_enemyforce_chance,in_otherforce_chance,rarity,rarity_chance, award_id, place from quest_visit"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	const int SPLITE_NUM = 20;
	char *p[SPLITE_NUM];
	char *p1[SPLITE_NUM];
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		QuestVisit *quest = new QuestVisit;
		quest->title = m_db->GetVal<uint32_t>(i, "title");
		quest->inSelfforce = m_db->GetVal<uint32_t>(i, "in_selfforce_chance");
		quest->inEnemyforce = m_db->GetVal<uint32_t>(i, "in_enemyforce_chance");
		quest->inOtherforce = m_db->GetVal<uint32_t>(i, "in_otherforce_chance");
		char *rarity = m_db->GetVal(i, "rarity");
		char *rarity_chance = m_db->GetVal(i, "rarity_chance");
		int num = CUtility::SplitLine(p, SPLITE_NUM, rarity, ',');
		if (num <= 0)
			return false;
		if (num == CUtility::SplitLine(p1, SPLITE_NUM, rarity_chance, ','))
		{
			for (int i = 0; i < num; i++)
			{
				quest->raritys.push_back(atoi(p[i]));
				quest->raritysChance.push_back(atoi(p[i]));
			}
		}
		else
		{
			return false;
		}
		quest->awardId = m_db->GetVal<uint32_t>(i, "award_id");
		quest->place = m_db->GetVal<uint32_t>(i, "place");
		quests.push_back(quest);
	}
	return true;
}

bool CReadWriteUser::ReadAdvConf(CAdvantureConf *conf)
{
	if (!m_db->Query("select award_gold,award_exp,recover_time from advanture_conf"))
	{
		cout << "get advanture_conf error" << endl;
		return false;
	}
	conf->SetAwardGold(m_db->GetVal<uint32_t>("award_gold"));
	conf->SetAwardExp(m_db->GetVal<uint32_t>("award_exp"));
	conf->SetRecoverTime(m_db->GetVal<uint32_t>("recover_time"));
	return true;
}

void CReadWriteUser::ReadAdvEvent(std::list<CAdvantureEvent*>& events)
{
	if (!m_db->Query("select id,percent,award_id,delay from advanture_event"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CAdvantureEvent *event = new CAdvantureEvent;
		event->SetId(m_db->GetVal<uint32_t>(i, "id"));
		event->SetPercent(m_db->GetVal<float>(i, "percent"));
		event->SetAwardId(m_db->GetVal<uint32_t>(i, "award_id"));
		event->SetDelay(m_db->GetVal<uint32_t>(i, "delay"));;
		events.push_back(event);
	}
}

void CReadWriteUser::ReadEvolveCost(std::list<CEvolveCost*>& costs)
{
	if (!m_db->Query("select star,awaken_level,promotion_level,materials,num,coin from evolve_cost"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CEvolveCost *cost = new CEvolveCost;
		cost->SetStar(m_db->GetVal<uint32_t>(i, "star"));
		cost->SetAwakenLevel(m_db->GetVal<uint32_t>(i, "awaken_level"));
		cost->SetPromotionLevel(m_db->GetVal<uint32_t>(i, "promotion_level"));
		cost->SetCoin(m_db->GetVal<uint32_t>(i, "coin"));

		cost->SetMaterials(m_db->GetVal(i, "materials"), m_db->GetVal(i, "num"));
		costs.push_back(cost);
	}
}

void CReadWriteUser::ReadEvolve(std::list<std::pair<uint32_t, uint32_t>>& fromTos)
{
	if (!m_db->Query("select from_id,to_id from evolve"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		std::pair<uint32_t, uint32_t> p;
		p.first = m_db->GetVal<uint32_t>(i, "from_id");
		p.second = m_db->GetVal<uint32_t>(i, "to_id");
		fromTos.push_back(p);
	}
}

bool CReadWriteUser::ReadLearnSkill(int & friendship, int &critBase, int &critTimes, int &generalSoul)
{
	if (!m_db->Query("select friendship,crit_base,crit_times, general_soul from learn_skill"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	if(m_db->GetRowsNum() > 0)
	{
		friendship = m_db->GetVal<int>("friendship");
		critBase = m_db->GetVal<int>("crit_base");
		critTimes = m_db->GetVal<int>("crit_times");
		generalSoul = m_db->GetVal<int>("general_soul");
		return true;
	}
	return false;
}

bool CReadWriteUser::ReadLearnPropeyty(std::list<LearnPropertyInfo*>& infos)
{
	if (!m_db->Query("select difference,property_exp,friendship,crit_base,crit_add,crit_times,general_soul from learn_property"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		LearnPropertyInfo *pro = new LearnPropertyInfo;
		char *p[2];
		if (CUtility::SplitLine(p, 2, m_db->GetVal(i, "difference"), ',') != 2)
			return false;
		pro->diffBegin = atoi(p[0]);
		pro->diffEnd = atoi(p[1]);
		pro->exp = m_db->GetVal<int>(i, "property_exp");
		pro->friendship = m_db->GetVal<int>(i, "friendship");
		pro->critBase = m_db->GetVal<int>(i, "crit_base");
		pro->critAdd = m_db->GetVal<int>(i, "crit_add");
		pro->critTimes = m_db->GetVal<int>(i, "crit_times");
		pro->generalSoul = m_db->GetVal<int>(i,"general_soul");
		infos.push_back(pro);
	}
	return true;
}

bool CReadWriteUser::ReadHeroPropertyExp()
{
	if (!m_db->Query("select strength_exp,leadership_exp,wisdom_exp,charm_exp,luck_exp from property_exp order by level"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	auto &strengthexp = CHeroInst::GetStrengthExp();
	auto &leadershipExp = CHeroInst::GetLeadershipExp();
	auto &wisdomExp = CHeroInst::GetWisdomExp();
	auto &charmExp = CHeroInst::GetCharmExp();
	auto &luckExp = CHeroInst::GetLuckExp();

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		strengthexp.push_back(m_db->GetVal<uint32_t>(i, "strength_exp"));
		leadershipExp.push_back(m_db->GetVal<uint32_t>(i, "leadership_exp"));
		wisdomExp.push_back(m_db->GetVal<uint32_t>(i, "wisdom_exp"));
		charmExp.push_back(m_db->GetVal<uint32_t>(i, "charm_exp"));
		luckExp.push_back(m_db->GetVal<uint32_t>(i, "luck_exp"));
	}
	return true;
}

bool CReadWriteUser::CreateUser(CUser &user)
{
	boost::format fmt("select id from user_info where nick='%1%'");
	string sql = (fmt%user.GetNick()).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "get user_info error" << endl;
		return false;
	}
	if (m_db->GetVal(0) != nullptr)
	{
		return false;
	}

	fmt.clear();
	fmt.parse("insert into user_info (nick,account_id) values ('%1%',%2%) returning id");
	sql = (fmt%user.GetNick() % user.GetAccountId()).str();

	if (!m_db->Query(sql.c_str()))
	{
		cout << "CreateUser error:" << m_db->GetErrMsg() << endl;
		return false;
	}
	if (m_db->GetVal(0) == nullptr)
	{
		return false;
	}
	user.SetUserId(m_db->GetVal<uint32_t>(0));
	return true;
}

bool CReadWriteUser::ReadBadWrods(std::list<std::string> &badWords)
{
	if (!m_db->Query("select word from bad_word"))
	{
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		badWords.push_back(m_db->GetVal(i,0));
	}
	return true;
}

void CReadWriteUser::InitDb()
{
	CDataBase *db = CGlobalVal::GetData<CDataBase>("db_connect");
	assert(db != nullptr);
	m_db = db;
}

void CReadWriteUser::ReadServerList(std::list<NetMsg::ServerInfo>& serverList)
{
	if (!m_db->Query("select id,name,ip,port,state from server_list order by order_id"))
	{
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		NetMsg::ServerInfo info;
		info.set_id(m_db->GetVal<uint32_t>(i, "id"));
		info.set_name(m_db->GetVal(i, "name"));
		info.set_ip(m_db->GetVal(i, "ip"));
		info.set_port(m_db->GetVal<int>(i, "port"));
		info.set_status((NetMsg::ServerStatus)m_db->GetVal<int>(i, "state"));
		serverList.push_back(info);
	}
}

void CReadWriteUser::ReadHero(std::list<CHeroTmpl*> &heros)
{
	if (!m_db->Query("select id,class,title,star,soldier_type,captain_point,strength,leadership,wisdom,charm,luck,vs_health,skill_id1,skill_level1,"
		"skill_condition1,skill_id2,skill_level2,skill_condition2,skill_id3,skill_level3,skill_condition3,skill_id4,"
		"skill_level4,skill_condition4 from hero order by id"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CHeroTmpl *hero = new CHeroTmpl;
		hero->SetId(m_db->GetVal<uint32_t>(i, "id"));
		hero->SetClass(m_db->GetVal<uint32_t>(i, "class"));
		//hero->SetForce(m_db->GetVal<uint32_t>(i, "force"));
		hero->SetTitle(m_db->GetVal(i, "title"));
		hero->SetStar(m_db->GetVal<uint32_t>(i, "star"));
		hero->SetSoldierType(m_db->GetVal<uint32_t>(i, "soldier_type"));
		hero->SetCaptainPoint(m_db->GetVal<uint32_t>(i, "captain_point"));
		hero->SetStrength(m_db->GetVal<uint32_t>(i, "strength"));
		hero->SetLeadership(m_db->GetVal<uint32_t>(i, "leadership"));
		hero->SetWisdom(m_db->GetVal<uint32_t>(i, "wisdom"));
		hero->SetCharm(m_db->GetVal<uint32_t>(i, "charm"));
		hero->SetLuck(m_db->GetVal<uint32_t>(i, "luck"));
		hero->SetVsHealth(m_db->GetVal<uint32_t>(i, "vs_health"));
		char buf[64];
		for (uint32_t j = 0; j < 4; j++)
		{
			snprintf(buf, sizeof(buf), "skill_id%d", j + 1);
			uint32_t skillId = m_db->GetVal<uint32_t>(i, buf);
			snprintf(buf, sizeof(buf), "skill_level%d", j + 1);
			uint32_t skillLevel = m_db->GetVal<uint32_t>(i, buf);
			snprintf(buf, sizeof(buf), "skill_condition%d", j + 1);
			std::string condition = m_db->GetVal(i, buf);
			hero->AddSkill(skillId, skillLevel, condition);
		}

		heros.push_back(hero);
	}
}

void CReadWriteUser::ReadItem(std::list<CItemTmpl*>& items)
{
	if (!m_db->Query("select id,type,star,extra_id,extra_val,extra_val2 from item"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CItemTmpl *item = new CItemTmpl;
		item->SetId(m_db->GetVal<uint32_t>(i, "id"));
		item->SetType(m_db->GetVal<uint32_t>(i, "type"));
		item->SetStar(m_db->GetVal<uint32_t>(i, "star"));
		item->SetExtId(m_db->GetVal<uint32_t>(i, "extra_id"));
		item->SetExtVal(m_db->GetVal<uint32_t>(i, "extra_val"));
		item->SetExtVal2(m_db->GetVal<uint32_t>(i, "extra_val2"));
		items.push_back(item);
	}
}

bool CReadWriteUser::ReadForce(std::list<CForce*>& forces)
{
	if (!m_db->Query("select f.id,f.leader,f.capital,f.citys,c.heros,f.can_choose from force as f,choose_hero as c where f.id=c.force"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	std::vector<std::string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CForce *force = new CForce;
		force->SetId(m_db->GetVal<uint32_t>(i, "id"));
		force->SetLeader(m_db->GetVal<uint32_t>(i, "leader"));
		force->SetCapital(m_db->GetVal<uint32_t>(i, "capital"));
		force->SetCanChoose(m_db->GetVal<bool>(i, "can_choose"));
		std::string citys = m_db->GetVal(i, "citys");
		split.clear();
		boost::split(split, citys, boost::is_any_of(","), boost::algorithm::token_compress_on);
		try
		{
			for (auto i = split.begin(); i != split.end(); i++)
			{
				CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(boost::lexical_cast<uint32_t>(*i));
				if (city == nullptr)
					return false;
				force->AddCity(city);
			}
		}
		catch (...)
		{
			return false;
		}
		std::string heros = m_db->GetVal(i, "heros");
		split.clear();
		boost::split(split, heros, boost::is_any_of(","), boost::algorithm::token_compress_on);
		for (auto i = split.begin(); i != split.end(); i++)
		{
			force->AddCooseHeros(boost::lexical_cast<uint32_t>(*i));
		}
		forces.push_back(force);
	}
	return true;
}

void CReadWriteUser::ReadSkill(std::list <CSkillTmpl*> &skills)
{
	if (!m_db->Query("select id,star,is_exclusive,is_battle_skill,soldier_type,anim_name,levelup_curveid,skill_param1,skill_param2,skill_param3,skill_param4 from skill"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CSkillTmpl *skill = new CSkillTmpl;
		skill->SetId(m_db->GetVal<uint32_t>(i, "id"));
		skill->SetStar(m_db->GetVal<uint32_t>(i, "star"));
		
		if (m_db->GetVal<uint32_t>(i, "is_exclusive") == 1)
			skill->SetIsExclusive(true);
		else
			skill->SetIsExclusive(false);

		if(m_db->GetVal<uint32_t>(i, "is_battle_skill") == 1)
			skill->SetIsBattleSkill(true);
		else
			skill->SetIsBattleSkill(false);
		skill->SetSoldierType(m_db->GetVal<uint32_t>(i, "soldier_type"));
		skill->SetAnimName(m_db->GetVal(i, "anim_name"));
		skill->SetLevelUpId(m_db->GetVal<uint32_t>(i, "levelup_curveid"));
		skill->SetSkillParam1(m_db->GetVal(i, "skill_param1"));
		skill->SetSkillParam2(m_db->GetVal(i, "skill_param2"));
		skill->SetSkillParam3(m_db->GetVal(i, "skill_param3"));
		skill->SetSkillParam4(m_db->GetVal(i, "skill_param4"));

		skills.push_back(skill);
	}
}

void CReadWriteUser::ReadUserHero(CUser * user)
{
	boost::format fmt("select id,hero_id,level,star_level,strength,leadership,wisdom,charm,luck,cd_time,exp,skills from user_hero where user_id=%1%");
	fmt%user->GetUserId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	auto toAttr = [](char *str, HeroAttr &attr) {
		char *p[2];
		if (CUtility::SplitLine(p, 2, str, ',') == 2)
		{
			attr.exp = atoi(p[0]);
			attr.lv = atoi(p[1]);
		}
	};
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CHeroInst *hero = new CHeroInst;
		hero->SetId(m_db->GetVal<uint64_t>(i, "id"));
		if (!hero->Init(m_db->GetVal<uint32_t>(i, "hero_id")))
		{
			delete hero;
			return;
		}
		hero->SetLevel(m_db->GetVal<uint32_t>(i, "level"));
		hero->SetStarLevel(m_db->GetVal<uint32_t>(i, "star_level"));
		HeroAttr &strength = hero->GetStrength();
		toAttr(m_db->GetVal(i, "strength"), strength);
		HeroAttr &leadership = hero->GetLeadership();
		toAttr(m_db->GetVal(i, "leadership"), leadership);
		HeroAttr &wisdom = hero->GetWisdom();
		toAttr(m_db->GetVal(i, "wisdom"), wisdom);
		HeroAttr &charm = hero->GetCharm();
		toAttr(m_db->GetVal(i, "charm"), charm);
		HeroAttr &luck = hero->GetLuck();
		toAttr(m_db->GetVal(i, "luck"), luck);
		hero->SetCDTime(m_db->GetVal<time_t>(i, "cd_time"));
		hero->SetExp(m_db->GetVal<uint32_t>(i, "exp"));
		char *skills = m_db->GetVal(i, "skills");
		char *p[24];
		int num = CUtility::SplitLine(p, 24,skills , ',');
		if (num % 3 == 0)
		{
			for (int i = 0; i < num / 3; i++)
			{
				CSkillInst skill;
				int pos = 3 * i;
				if (skill.Init(atoi(p[pos])))
				{
					skill.SetLevel(atoi(p[pos+1]));
					skill.SetExp(atoi(p[pos + 2]));
					hero->AddSkill(skill);
				}
			}
		}
		hero->SetUpdateToDb(true);
		user->AddHero(hero);
	}
}

//ResourceFieldFlag
void CReadWriteUser::ReadUserResourceField(CUser * user)
{
	boost::format fmt("select id,resource_id,create_time,resource_cd,pos,resource_num,left_resource_num,team_mining ,is_close from user_resource where user_id=%1%");
	fmt%user->GetUserId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CResourceInst *resField = new CResourceInst;
		resField->SetId(m_db->GetVal<uint64_t>(i, "id"));
		resField->SetResourceID(m_db->GetVal<uint32_t>(i, "resource_id"));
		resField->SetCreateTime(m_db->GetVal<uint32_t>(i, "create_time"));
		resField->SetResCD(m_db->GetVal<uint32_t>(i, "resource_cd"));
		resField->SetPosID(m_db->GetVal<uint32_t>(i, "pos"));
		resField->SetResourceNum(m_db->GetVal<uint32_t>(i, "resource_num"));
		resField->SetLeftResourceNum(m_db->GetVal<uint32_t>(i, "left_resource_num"));
		resField->SetResourceClose(m_db->GetVal<uint32_t>(i, "is_close"));
		//lolo


		resField->SetUpdateToDb(true);
		user->AddResourceFields(resField);
	}
}

void CReadWriteUser::ReadUId()
{

	if (!m_db->Query("select max(id) from user_resource"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CResourceInst::SetUID(m_db->GetVal<uint64_t>(0));

	if (!m_db->Query("select max(id) from user_hero"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CHeroInst::SetUID(m_db->GetVal<uint64_t>(0));
	if (!m_db->Query("select max(id) from hero_skill"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CSkillInst::SetUID(m_db->GetVal<uint64_t>(0));

	if (!m_db->Query("select max(id) from user_quest"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CUserQuest::SetUID(m_db->GetVal<uint64_t>(0));

	if (!m_db->Query("select max(id) from user_battle"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CUserBattle::SetUID(m_db->GetVal<uint64_t>(0));

	if (!m_db->Query("select max(id) from user_item"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}
	CItemInst::SetUID(m_db->GetVal<uint64_t>(0));
}

void CReadWriteUser::ReadWorldPath(std::list<CWorldPath*> &worldPaths)
{
	if (!m_db->Query("select id,start_point,end_point,length,weight,in_use,can_have_monster from world_path"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CWorldPath *path = new CWorldPath;
		path->SetId(m_db->GetVal<uint32_t>(i,"id"));
		path->SetStartPoint(m_db->GetVal<uint32_t>(i,"start_point"));
		path->SetEndPoint(m_db->GetVal<uint32_t>(i,"end_point"));
		path->SetLength(m_db->GetVal<uint32_t>(i,"length"));
		path->SetWeight(m_db->GetVal<uint32_t>(i,"weight"));
		path->SetInUse(m_db->GetVal<bool>(i,"in_use"));
		path->SetCanHaveMonster(m_db->GetVal<bool>(i,"can_have_monster"));

		worldPaths.push_back(path);
	}
}

void CReadWriteUser::ReadWorldCity(std::list<CWorldCity*>& citys)
{
	if (!m_db->Query("select id,way_point,battle_scene,governor,agriculture,business,military,culture_development,money,provisions,"
		"moneydelta,provisions_delta,soldiers,reserver,infantry,archer,cavalry,sword_shield,crossbow_man,elephant,"
		"innfantry_wounded,archer_wounded,cavalry_wounded,sword_shield_wounded,crossbow_man_wounded,elephant_wounded,population,"
		"population_speed,loyalty,durability,specialties,city_item,city_hero from world_city"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CWorldCity *city = new CWorldCity;
		city->SetId(m_db->GetVal<uint32_t>(i, "id"));
		city->SetWayPoint(m_db->GetVal<uint32_t>(i, "way_point"));
		city->SetBattleScene(m_db->GetVal<uint32_t>(i, "battle_scene"));
		city->SetGovernor(m_db->GetVal<uint32_t>(i, "governor"));
		city->SetAgriculture(m_db->GetVal<uint32_t>(i, "agriculture"));
		city->SetBusiness(m_db->GetVal<uint32_t>(i, "business"));
		city->SetMilitary(m_db->GetVal<uint32_t>(i, "military"));
		city->SetCultureDevelopment(m_db->GetVal<uint32_t>(i, "culture_development"));
		city->SetMoney(m_db->GetVal<uint32_t>(i, "money"));
		city->SetProvisions(m_db->GetVal<uint32_t>(i, "provisions"));
		city->SetMoneyDelta(m_db->GetVal<uint32_t>(i, "moneydelta"));
		city->SetProvisionsDelta(m_db->GetVal<uint32_t>(i, "provisions_delta"));
		city->SetSoldiers(m_db->GetVal<uint32_t>(i, "soldiers"));
		city->SetReserver(m_db->GetVal<uint32_t>(i, "reserver"));
		city->SetInfantry(m_db->GetVal<uint32_t>(i, "infantry"));
		city->SetArcher(m_db->GetVal<uint32_t>(i, "archer"));
		city->SetCavalry(m_db->GetVal<uint32_t>(i, "cavalry"));
		city->SetSwordShield(m_db->GetVal<uint32_t>(i, "sword_shield"));
		city->SetCrossbowMan(m_db->GetVal<uint32_t>(i, "crossbow_man"));
		city->SetElephant(m_db->GetVal<uint32_t>(i, "elephant"));
		city->SetInfantryWounded(m_db->GetVal<uint32_t>(i, "innfantry_wounded"));
		city->SetArcherWounded(m_db->GetVal<uint32_t>(i, "archer_wounded"));
		city->SetCavalryWounded(m_db->GetVal<uint32_t>(i, "cavalry_wounded"));
		city->SetSwordShieldWounded(m_db->GetVal<uint32_t>(i, "sword_shield_wounded"));
		city->SetCrossbowManWounded(m_db->GetVal<uint32_t>(i, "crossbow_man_wounded"));
		city->SetElephantWounded(m_db->GetVal<uint32_t>(i, "elephant_wounded"));
		city->SetPopulation(m_db->GetVal<uint32_t>(i, "population"));
		city->SetPopulationSpeed(m_db->GetVal<uint32_t>(i, "population_speed"));
		city->SetLoyalty(m_db->GetVal<uint32_t>(i, "loyalty"));
		city->SetDurability(m_db->GetVal<uint32_t>(i, "durability"));
		char *val = m_db->GetVal(i, "specialties");
		char *p[10];
		int n = CUtility::SplitLine(p, 10, val, ',');
		auto &s = city->GetSpecialties();
		for (int i = 0; i < n; i++)
		{
			s.push_back(atoi(p[i]));
		}

	
		NetMsg::CityItemAck cityAck;//lolo
		char *strCityAck = m_db->GetVal(i,"city_item");
		if (strCityAck != nullptr && strlen(strCityAck) > 1)
		{
			CUtility::StrToMsg(strCityAck, &cityAck);
			for (int j = 0; j < cityAck.cityitem_size(); j++)
			{
				NetMsg::CityItemInfo * info = cityAck.mutable_cityitem(j);
				city->SetCityItemInfo(info->itemid(),info->itemnum());
			}
		}


		NetMsg::CityHeroAck heroAck;//lolo
		char *strHeroAck = m_db->GetVal(i, "city_hero");
		if (strHeroAck != nullptr && strlen(strHeroAck) > 1)
		{
			CUtility::StrToMsg(strHeroAck, &heroAck);
			for (int k = 0; k < heroAck.cityhero_size(); k++)
			{
				NetMsg::CityHeroInfo * info = heroAck.mutable_cityhero(k);
				if(info)
				{
					uint32_t id = info->heroid();
					std::string name = info->heroname();
					uint32_t level = info->herolevel();
					city->AddCityHero(id, name, level);
				}
			}
		}

		citys.push_back(city);
	}
}

void CReadWriteUser::ReadQuest(std::list<CQuest*>& quests)
{
	if (!m_db->Query("select id,type,subtype,effect,award_type,award_num from quest"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CQuest *quest = new CQuest;
		quest->SetId(m_db->GetVal<uint32_t>(i,"id"));
		quest->SetType(m_db->GetVal<uint32_t>(i, "type"));
		quest->SetSubType(m_db->GetVal<uint32_t>(i, "subtype"));
		quest->SetEffect(m_db->GetVal<uint32_t>(i,"effect"));
		quest->SetAward(m_db->GetVal(i, "award_type"), m_db->GetVal(i, "award_num"));
		quests.push_back(quest);
	}
}

void CReadWriteUser::ReadConsAgriculture(std::list<ConsAgriculture*> &dataList)
{
	if (!m_db->Query("select title,in_selfcity_chance,target,award_id,place from quest_cons_agriculture"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	std::vector<string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		ConsAgriculture *agr = new ConsAgriculture;
		agr->title = m_db->GetVal<uint32_t>(i,"title");
		agr->selfCityPer = m_db->GetVal<float>(i,"in_selfcity_chance");
		agr->target = m_db->GetVal<uint32_t>(i,"target");
		agr->awardId = m_db->GetVal<uint32_t>(i,"award_id");

		split.clear();
		std::string var = m_db->GetVal(i,"place");
		boost::split(split, var, boost::is_any_of(","), boost::algorithm::token_compress_on);
		for (auto i = split.begin(); i != split.end(); i++)
		{
			agr->places.push_back(boost::lexical_cast<uint32_t>(*i));
		}
		dataList.push_back(agr);
	}
}

void CReadWriteUser::ReadConsBusiness(std::list<ConsBusiness*>& dataList)
{
	if (!m_db->Query("select title,in_selfcity_chance,target,award_id,place from quest_cons_business"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	std::vector<string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		ConsBusiness *bus = new ConsBusiness;
		bus->title = m_db->GetVal<uint32_t>(i,"title");
		bus->selfCityPer = m_db->GetVal<float>(i,"in_selfcity_chance");
		bus->target = m_db->GetVal<uint32_t>(i,"target");
		bus->awardId = m_db->GetVal<uint32_t>(i,"award_id");

		split.clear();
		std::string var = m_db->GetVal(i,"place");
		boost::split(split, var, boost::is_any_of(","), boost::algorithm::token_compress_on);
		for (auto i = split.begin(); i != split.end(); i++)
		{
			bus->places.push_back(boost::lexical_cast<uint32_t>(*i));
		}
		dataList.push_back(bus);
	}
}

void CReadWriteUser::ReadBattleCateran(std::list<BattleCateran*>& battls)
{
	if (!m_db->Query("select id,chance,in_selfcity_chance,pos_on_rood,duration,enemy_team,award_id from quest_battle_cateran"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	std::vector<string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		BattleCateran *battle = new BattleCateran;
		battle->id = m_db->GetVal<uint32_t>(i, "id");
		battle->chance = m_db->GetVal<uint32_t>(i, "chance");
		battle->selfCityPer = m_db->GetVal<float>(i, "in_selfcity_chance");
		std::string var = m_db->GetVal(i, "pos_on_rood");
		boost::split(split, var, boost::is_any_of(","), boost::algorithm::token_compress_on);
		if (split.size() < 2)
			continue;
		battle->onRoadBegin = (float)atof(split[0].c_str());
		battle->onRoadEnd = (float)atof(split[1].c_str());
		battle->duration = m_db->GetVal<uint32_t>(i, "duration");
		battle->enemyTeam = m_db->GetVal<uint32_t>(i, "enemy_team");
		battle->awardId = m_db->GetVal<uint32_t>(i, "award_id");
		battls.push_back(battle);
	}
}

void CReadWriteUser::ReadConsMilitary(std::list<ConsMilitary*>& dataList)
{
	if (!m_db->Query("select title,in_selfcity_chance,target,award_id,place from quest_cons_military"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	std::vector<string> split;
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		ConsMilitary *bus = new ConsMilitary;
		bus->title = m_db->GetVal<uint32_t>(i, "title");
		bus->selfCityPer = m_db->GetVal<float>(i, "in_selfcity_chance");
		bus->target = m_db->GetVal<uint32_t>(i, "target");
		bus->awardId = m_db->GetVal<uint32_t>(i, "award_id");

		split.clear();
		std::string var = m_db->GetVal(i, "place");
		boost::split(split, var, boost::is_any_of(","), boost::algorithm::token_compress_on);
		for (auto i = split.begin(); i != split.end(); i++)
		{
			bus->places.push_back(boost::lexical_cast<uint32_t>(*i));
		}
		dataList.push_back(bus);
	}
}
//add by clx

bool CReadWriteUser::ReadRankReputation(std::list<CRankType*>& rankreputations)
{
	if (!m_db->Query("select id,nick,force,level,title,reputation,icon from user_info order by reputation desc limit 20"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CRankType *rankReputation = new CRankType;
		if (!rankReputation->Init(m_db->GetVal<uint32_t>(i, "id"),m_db->GetVal(i, "nick"), m_db->GetVal<uint32_t>(i, "force"), m_db->GetVal<uint32_t>(i, "level"), m_db->GetVal<uint32_t>(i, "title"), m_db->GetVal<uint32_t>(i, "reputation"), 10, m_db->GetVal<uint32_t>(i, "icon")))
		{
			delete rankReputation;
			return false;
		}
		rankreputations.push_back(rankReputation);
	}
	return true;
}

bool CReadWriteUser::ReadRankLevel(std::list<CRankType*>& ranklevel)
{
	if (!m_db->Query("select id,nick,force,level,title,reputation,icon from user_info order by level desc limit 20"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CRankType *ranktype = new CRankType;
		if (!ranktype->Init(m_db->GetVal<uint32_t>(i, "id"),m_db->GetVal(i, "nick"), m_db->GetVal<uint32_t>(i, "force"), m_db->GetVal<uint32_t>(i, "level"), m_db->GetVal<uint32_t>(i, "title"), m_db->GetVal<uint32_t>(i, "reputation"),10, m_db->GetVal<uint32_t>(i, "icon")))
		{
			delete ranktype;
			return false;
		}
		ranklevel.push_back(ranktype);
	}
	return true;
}


bool CReadWriteUser::ReadOwnReputaion(CUserTopList * topList)
{
	//userid 
	uint32_t userID = topList->GetUserID();
	boost::format fmt("select count(*) from user_info where reputation > (select reputation from user_info where id = %1%)");
	fmt%userID;

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		topList->SetMyRanking(m_db->GetVal<uint32_t>(i, "count"));
	}
	return true;
}


bool CReadWriteUser::ReadOwnLevel(CUserTopList * topList)
{
	//userid 
	uint32_t userID = topList->GetUserID();
	boost::format fmt("select count(*) from user_info where level > (select level from user_info where id = %1%)");
	fmt%userID;

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		topList->SetMyRanking(m_db->GetVal<uint32_t>(i, "count"));
	}
	return true;
}

bool CReadWriteUser::ReadRanks(std::list<CRank*>& ranks)
{
	if (!m_db->Query("select level,exp,titles from rank order by level"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CRank *rank = new CRank;
		if (!rank->Init(m_db->GetVal<uint32_t>(i,"level"), m_db->GetVal<uint32_t>(i,"exp"), m_db->GetVal(i,"titles")))
		{
			delete rank;
			return false;
		}
		ranks.push_back(rank);
	}
	return true;
}




bool CReadWriteUser::ReadTitles(std::list<CTitle*>& titles)
{
	if (!m_db->Query("select id,num_get_quest,quests,quest_chance,num_limit,property_add,award_id,campaign_cycle,campaign_time,name from title"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CTitle *title= new CTitle;
		if (!title->Init(m_db->GetVal<uint32_t>(i,"id"), m_db->GetVal(i,"quests"), m_db->GetVal(i,"quest_chance")))
		{
			delete title;
			return false;
		}
		title->SetQuestNum(m_db->GetVal<uint32_t>(i, "num_get_quest"));
		title->SetLimitNum(m_db->GetVal<int>(i, "num_limit"));
		title->SetAwardId(m_db->GetVal<uint32_t>(i, "award_id"));
		title->SetAddProperty(m_db->GetVal(i, "property_add"));
		title->SetCampaignSpace(m_db->GetVal<uint32_t>(i, "campaign_cycle"));
		title->SetCampaignTime(m_db->GetVal(i, "campaign_time"));
		title->SetName(m_db->GetVal(i, "name"));
		titles.push_back(title);
	}
	return true;
}

void CReadWriteUser::ReadAwards(std::list<CAward*>& awards)
{
	if (!m_db->Query("select id,exp,exp_hero,coin,gold,contribution,reputation,item_type1,item_id1,item_num1,"
		"item_type2,item_id2,item_num2,item_type3,item_id3,item_num3,item_type4,item_id4,item_num4,random_per,"
		"random_group,random_times,random_times_per from award"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CAward *award = new CAward;
		award->SetId(m_db->GetVal<uint32_t>(i,"id"));
		award->SetExp(m_db->GetVal<uint32_t>(i,"exp"));
		award->SetExpHero(m_db->GetVal<uint32_t>(i, "exp_hero"));
		award->SetCoin(m_db->GetVal<uint32_t>(i, "coin"));
		award->SetGold(m_db->GetVal<uint32_t>(i,"gold"));
		award->SetContribution(m_db->GetVal<uint32_t>(i,"contribution"));
		award->SetReputation(m_db->GetVal<uint32_t>(i,"reputation"));
		char buf[64];
		for (int j = 0; j < 4; j++)
		{
			int pos = j + 1;
			int type, itemId, num;
			snprintf(buf, sizeof(buf), "item_type%d", pos);
			type = m_db->GetVal<uint32_t>(i, buf);
			snprintf(buf, sizeof(buf), "item_id%d", pos);
			itemId = m_db->GetVal<uint32_t>(i, buf);
			snprintf(buf, sizeof(buf), "item_num%d", pos);
			num = m_db->GetVal<uint32_t>(i, buf);
			award->AddAward(type, itemId, num);
		}
		award->SetRandomPer(m_db->GetVal<float>(i, "random_per"));
		award->SetRandomGroup(m_db->GetVal<uint32_t>(i, "random_group"));
		char *randTimes = m_db->GetVal(i, "random_times");
		char *randTimesPer = m_db->GetVal(i, "random_times_per");
		char *rT[20];
		char *rTP[20];
		int num = CUtility::SplitLine(rT, 20, randTimes, ',');
		if (num == CUtility::SplitLine(rTP, 20, randTimesPer, ','))
		{
			for (int j = 0; j < num; j++)
			{
				award->AddRandTimes((float)atof(rTP[j]), atoi(rT[j]));
			}
		}
		awards.push_back(award);
	}
}

void CReadWriteUser::ReadAwardGroups(std::list<AwardGroup*>& awardGroups)
{
	if (!m_db->Query("select id,percent,type,item_id,min_num,max_num from award_group"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		AwardGroup *award = new AwardGroup;
		award->id = m_db->GetVal<uint32_t>(i, "id");
		award->percent = m_db->GetVal<float>(i, "percent");
		award->type = (CAward::AwardItemType)m_db->GetVal<uint32_t>(i, "type");
		award->itemId = m_db->GetVal<uint32_t>(i, "item_id");
		award->minNum = m_db->GetVal<uint32_t>(i, "min_num");
		award->maxNum = m_db->GetVal<uint32_t>(i, "max_num");
		awardGroups.push_back(award);
	}
}

void CReadWriteUser::ReadShop(std::list<CShopTmpl*>& shops)
{
	if (!m_db->Query("select id,refresh_type,refresh_data,buy_time,can_refresh,currency,refresh_price,refresh_item,goods_number,"
		"group_id,group_chance,group_min_num from shop"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CShopTmpl *shop = new CShopTmpl;
		shop->SetId(m_db->GetVal<uint32_t>(i, "id"));
		shop->SetRefreshType(m_db->GetVal<uint32_t>(i, "refresh_type"));
		shop->SetRefreshData(m_db->GetVal(i));
		shop->SetBuyTime(m_db->GetVal<uint32_t>(i, "buy_time"));
		shop->SetCanRefresh(m_db->GetVal<bool>(i, "can_refresh"));
		shop->SetCurrency(m_db->GetVal<uint32_t>(i, "currency"));
		shop->SetRefreshPrice(m_db->GetVal<uint32_t>(i, "refresh_price"));
		shop->SetRefreshItem(m_db->GetVal<uint32_t>(i, "refresh_item"));
		shop->SetGoodsNum(m_db->GetVal<uint32_t>(i, "goods_number"));
		char *groupId = m_db->GetVal(i, "group_id");
		char *groupChance = m_db->GetVal(i, "group_chance");
		char *groupMinNum = m_db->GetVal(i, "group_min_num");
		char *gId[20];
		char *gCh[20];
		char *gMin[20];
		int num = CUtility::SplitLine(gId, 20, groupId, ',');
		if (num == CUtility::SplitLine(gCh, 20, groupChance, ',') && num == CUtility::SplitLine(gMin, 20, groupMinNum, ','))
		{
			for (int j = 0; j < num; j++)
			{
				shop->AddGroup(atoi(gId[j]), atoi(gCh[j]), atoi(gMin[j]));
			}
		}
		shops.push_back(shop);
	}
}

void CReadWriteUser::ReadShopGoods(std::list<CShopGoods*>& shopGoods)
{
	if (!m_db->Query("select id,group_id,item_id,item_num,currency,price from shop_goods"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CShopGoods *good = new CShopGoods;
		good->SetItemId(m_db->GetVal<uint32_t>(i, "id"));
		good->SetGroupId(m_db->GetVal<uint32_t>(i, "group_id"));
		good->SetItemId(m_db->GetVal<uint32_t>(i, "item_id"));
		good->SetItemNum(m_db->GetVal<uint32_t>(i, "item_num"));
		good->SetCurrency(m_db->GetVal<uint32_t>(i, "currency"));
		good->SetPrice(m_db->GetVal<uint32_t>(i, "price"));
		shopGoods.push_back(good);
	}
}

void CReadWriteUser::ReadWorldWarScene(CWorldWarScene* scene)
{
	boost::format fmt("select id,connection,building_type,force from world_war_scene_path_%1%");
	fmt%scene->GetSceneId();

	if (!m_db->Query(fmt.str().c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return;
	}

	scene->Init(m_db);
}

void CReadData::SetData(CDataBase * db,int row)
{
	m_datas.clear();
	for (int i = 0; i < db->GetColumnsNum(); i++)
	{
		m_datas.push_back(db->GetVal(row, i));
	}
}

bool CReadWriteUser::ReadData(std::list<CReadData>& datas, std::vector<const char*>& fieldNames, const char * table, const char * con)
{
	std::string sql;
	sql = "select ";
	for (auto i = fieldNames.begin(); i != fieldNames.end(); i++)
	{
		if (i != fieldNames.begin())
			sql.append(",");
		sql.append(*i);
	}
	sql.append(" from ");
	sql.append(table);
	if (con != nullptr)
	{
		sql.append(" ");
		sql.append(con);
	}
	if (!m_db->Query(sql.c_str()))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CReadData data;
		data.SetData(m_db, i);
		datas.push_back(data);
	}
	return true;
}

bool CReadWriteUser::UpdateWorldCity(CWorldCity *city)
{
	string cityItemString;
	NetMsg::CityItemAck cityItemAck;
	city->GetCityItem(&cityItemAck);
	CUtility::MsgToStr(&cityItemAck, cityItemString);
	string cityHeroString;
	NetMsg::CityHeroAck cityHeroAck;
	city->GetCityHero(&cityHeroAck);
	CUtility::MsgToStr(&cityHeroAck, cityHeroString);

	stringstream strStrem;
	strStrem << "update world_city set ";
	strStrem << "governor =";
	strStrem << city->GetGovernor();
	strStrem << ",agriculture=";
	strStrem << city->GetAgriculture();
	strStrem << ",business=";
	strStrem << city->GetBusiness();
	strStrem << ",military=";
	strStrem << city->GetMilitary();
	strStrem << ",culture_development=";
	strStrem << city->GetCultureDevelopment();
	strStrem << ",money=";
	strStrem << city->GetMoney();
	strStrem << ",provisions=";
	strStrem << city->GetProvisions();
	strStrem << ",moneydelta=";
	strStrem << city->GetMoneyDelta();
	strStrem << ",provisions_delta=";
	strStrem << city->GetProvisionsDelta();
	strStrem << ",soldiers=";
	strStrem << city->GetSoldiers();
	strStrem << ",reserver=";
	strStrem << city->GetReserver();
	strStrem << ",infantry=";
	strStrem << city->GetInfantry();
	strStrem << ",archer=";
	strStrem << city->GetArcher();
	strStrem << ",cavalry=";
	strStrem << city->GetCavalry();
	strStrem << ",sword_shield=";
	strStrem << city->GetSwordShield();
	strStrem << ",crossbow_man=";
	strStrem << city->GetCrossbowMan();
	strStrem << ",elephant=";
	strStrem << city->GetElephant();
	strStrem << ",innfantry_wounded=";
	strStrem << city->GetInfantryWounded();
	strStrem << ",archer_wounded=";
	strStrem << city->GetArcherWounded();
	strStrem << ",cavalry_wounded=";
	strStrem << city->GetCavalryWounded();
	strStrem << ",sword_shield_wounded=";
	strStrem << city->GetSwordShieldWounded();
	strStrem << ",crossbow_man_wounded=";
	strStrem << city->GetCrossbowManWounded();
	strStrem << ",elephant_wounded=";
	strStrem << city->GetElephantWounded();
	strStrem << ",population=";
	strStrem << city->GetPopulation();
	strStrem << ",population_speed=";
	strStrem << city->GetPopulationSpeed();
	strStrem << ",loyalty=";
	strStrem << city->GetLoyalty();
	strStrem << ",durability=";
	strStrem << city->GetDurability();

	strStrem << ",city_item=";
	strStrem << cityItemString.c_str();

	strStrem << ",city_hero=";
	strStrem << cityHeroString.c_str();

	/*strStrem << ",specialties";
	strStrem << city->GetSpecialties();*/
	strStrem << " where id=" << city->GetId();

	if (!m_db->Query(strStrem.str().c_str()))
	{
		cout << m_db->GetErrMsg() << endl;
		cout << "update world_city :" <<city->GetId() <<" error"<< endl;
		return false;
	}

	return true;
}

void CReadWriteUser::WriteMail(CUserMail * mail)
{
	boost::format fmt("INSERT INTO user_mail (f_id,f_name,f_icon,f_title,to_id,title,content,award,send_time,is_read,received,type) "
		"VALUES (%1%, '%2%',%3%,%4%,%5%,'%6%','%7%','%8%',%9%,0,0,%10%) RETURNING id");
	auto &name = mail->GetFromName();
	auto &title = mail->GetTitle();
	auto &content = mail->GetContent();
	uint32_t awardId = mail->GetAward().awardid();

	fmt%mail->GetFromId() % name%mail->GetFromIcon() % mail->GetFromTitle() % mail->GetToId() % title%content%awardId%mail->GetSendTime() % mail->GetType();
	string sql = fmt.str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "write mail error" << m_db->GetErrMsg()<< endl;
		return;
	}
	mail->SetId(m_db->GetVal<uint32_t>(0));
}

void CReadWriteUser::UpdateMailIsRead(uint32_t mailId)
{
	boost::format fmt("update user_mail set is_read=1 where id=%1%");
	
	string sql = (fmt%mailId).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "update mail error" << endl;
		return;
	}
}

void CReadWriteUser::DelMail(CUser * user)
{
	auto &mail = user->GetMails();
	string delIds;
	for (auto i = mail.begin(); i != mail.end(); i++)
	{
		if (i->GetDelMail())
		{
			MakeInCondition(i->GetId(), delIds);
		}
	}
	if (delIds.empty())
		return;
	
	boost::format fmt("delete from user_mail where id in (%1%)");
	string sql = (fmt%delIds).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "delete mail error" << endl;
		return;
	}
}

void CReadWriteUser::ReadMail(CUser * user)
{
	auto &mails = user->GetMails();
	boost::format fmt("select id,f_id,f_name,f_icon,f_title,title,content,award,send_time,is_read,received,type from user_mail where to_id=%1%");
	string sql = (fmt%user->GetUserId()).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "read mail error:" <<m_db->GetErrMsg()<<endl;
		return;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CUserMail mail;
		mail.SetId(m_db->GetVal<uint32_t>(i,"id"));
		mail.SetFromId(m_db->GetVal<uint32_t>(i,"f_id"));
		mail.SetFromName(m_db->GetVal(i,"f_name"));
		mail.SetFromIcon(m_db->GetVal<uint32_t>(i,"f_icon"));
		mail.SetFromTitle(m_db->GetVal<uint32_t>(i,"f_title"));
		mail.SetTitle(m_db->GetVal(i, "title"));
		mail.SetContent(m_db->GetVal(i,"content"));
		auto &award = mail.GetAward();
		uint32_t awardId = m_db->GetVal<uint32_t>(i, "award");
		if (awardId != 0)
		{
			award.set_awardid(awardId);
		}

		mail.SetSendTime(m_db->GetVal<time_t>(i,"send_time"));
		mail.SetIsRead(m_db->GetVal<bool>(i,"is_read"));
		mail.SetIsReceived(m_db->GetVal<bool>(i, "received"));
		mail.SetType(m_db->GetVal<uint32_t>(i, "type"));
		mail.SetToId(user->GetUserId());
		mails.push_back(mail);
	}
}

void CReadWriteUser::UpdateMailGiveAward(uint32_t mailId)
{
	boost::format fmt("update user_mail set received=1 where id=%1%");

	string sql = (fmt%mailId).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "UpdateMailGiveAward mail error" << endl;
		return;
	}
}

void CReadWriteUser::UpdateUserTitle(uint32_t userId, uint32_t title)
{
	boost::format fmt("update user_info set title=%1% where id=%2%");

	string sql = (fmt%title%userId).str();
	if (!m_db->Query(sql.c_str()))
	{
		cout << "update user title error" << endl;
		return;
	}
}
bool CReadWriteUser::ReadExchangeGeneralSoulCost(std::list<GeneralSoulCost*>& costs)
{
	if (!m_db->Query("select star,number from hero_general_soul"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		GeneralSoulCost *cost = new GeneralSoulCost;
		cost->star = m_db->GetVal<uint32_t>(i, "star");
		cost->cost = m_db->GetVal<uint32_t>(i, "number");
		costs.push_back(cost);
	}
	return true;
}
bool CReadWriteUser::ReadFriendshipWhenGetHero(std::list<GetFriendshipByGetHero*>& friendShips)
{
	if (!m_db->Query("select star,first_time,normal from hero_friendship_when_get_hero"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		GetFriendshipByGetHero *fri = new GetFriendshipByGetHero;
		fri->star = m_db->GetVal<uint32_t>(i, "star");
		fri->first = m_db->GetVal<uint32_t>(i, "first_time");
		fri->normal = m_db->GetVal<uint32_t>(i, "normal");
		friendShips.push_back(fri);
	}
	return true;
}
bool CReadWriteUser::ReadHeroFriendshipLevelUp(std::list<CLearnAndVsCD*>& infos)
{
	if (!m_db->Query("select level,exp,teacher_learn_cd,student_learn_cd from hero_friendship_level_up"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CLearnAndVsCD *cd = new CLearnAndVsCD;
		cd->level = m_db->GetVal<uint32_t>(i, "level");
		cd->exp = m_db->GetVal<uint32_t>(i, "exp");
		cd->teacher_learn_cd = m_db->GetVal<uint32_t>(i, "teacher_learn_cd");
		cd->student_learn_cd = m_db->GetVal<uint32_t>(i, "student_learn_cd");
		infos.push_back(cd);
	}
	return true;
}

//ResourceFieldFlag
bool CReadWriteUser::ReadResourceField(std::list<CResourceTmpl*> & infos)
{
	if (!m_db->Query("select id,star,type,duration from resource_player"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}
	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CResourceTmpl * resTmpl = new CResourceTmpl;
		resTmpl->SetID(m_db->GetVal<uint32_t>(i, "id"));
		resTmpl->SetStar(m_db->GetVal<uint32_t>(i, "star"));
		resTmpl->SetResCDTime(m_db->GetVal<uint32_t>(i, "duration"));
		resTmpl->SetResType(m_db->GetVal<uint32_t>(i, "type"));
		infos.push_back(resTmpl);
	}
	return true;
}


bool CReadWriteUser::ReadResourcePos(std::list<CResourcePos*> & infos)
{
	if (!m_db->Query("select id,force from resource_player_position"))
	{
		std::cout << m_db->GetErrMsg() << endl;
		return false;
	}

	for (int i = 0; i < m_db->GetRowsNum(); i++)
	{
		CResourcePos * pos = new CResourcePos;
		pos->SetID(m_db->GetVal<uint32_t>(i, "id"));
		pos->SetForce(m_db->GetVal<uint32_t>(i, "force"));
		infos.push_back(pos);
	}
	return true;
}