#include "WorldWar.h"
#include <algorithm>
#include "user.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include "MailInfo.h"
#include "WorldWarScene.h"

bool CWorldWar::Init()
{
	struct WarConf
	{
		uint32_t id;
		uint32_t f1;
		uint32_t f2;
		uint32_t type;
		uint32_t path;
		float percent;
	};

	if (!readWorldWarAward())
		return false;
	if (!readWordldWarTable())
		return false;

	WarConf confs[] = {
		{1	, 12	,3	,1	,36	,0.95f},
		{2	,5,	11,	2,	133,	0.5f},
		{3	,11,	8,	2,	40,	0.6},
	};

	for (int i = 0; i < sizeof(confs) / sizeof(confs[0]); i++)
	{
		StartWar(confs[i].type, i + 1,confs[i].f1,confs[i].f2,confs[i].path,confs[i].percent);
	}
	return true;
}

bool CWorldWar::GiveAward(WarInfo * warInfo, int hour)
{

	uint32_t warListID = warInfo->warId;
	for (auto i = m_warTable.begin(); i != m_warTable.end(); i++)
	{
		if(warListID == i->id)
		{
			std::string strSortTimes = i->period_award_time;
			char *timeId[256];
			std::string temp = strSortTimes;
			int num = CUtility::SplitLine(timeId, 10, (char*)temp.c_str(), ',');
			if (num <= 0)
				return false;
			for (int i = 0; i < num; i++)
			{
				//m_titles.push_back((uint32_t)atoi(timeId[i]));
				if(hour == (uint32_t)atoi(timeId[i]))
				{
					return true;
				}
			}
		}
	}
	return false;
}

uint32_t CWorldWar::GetPeriodAwardByWarID(uint32_t warID)
{
	for (auto i = m_warTable.begin(); i != m_warTable.end(); i++)
	{
		if (warID == i->id)
		{
			return i->period_award_group;
		}
	}
	return 0;
}

uint32_t CWorldWar::GetTolAwardByWarID(uint32_t warID)
{
	for (auto i = m_warTable.begin(); i != m_warTable.end(); i++)
	{
		if (warID == i->id)
		{
			return i->final_award_group;
		}
	}
	return 0;
}

//是否是整点
bool OnTheHour(time_t now, int &hour)
{
	tm *tmNow = localtime(&now);
	static int oldHour = -1;
	if (oldHour == tmNow->tm_hour)
		return false;
	oldHour = tmNow->tm_hour;
	hour = oldHour;
	return true;
}

void CWorldWar::Update(time_t now)
{
	for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	{
		auto endTime = i->second.endTime;
		if (endTime <= now)
		{
			auto &tol = i->second.tolExploit;
			int index2 = 0;
			for (auto j = tol.begin(); j != tol.end(); j++)
			{
				index2++;
				uint32_t tolAwardId = GetTolAwardByWarID(i->second.warId);
				uint32_t awardID = GetWorldWarAward(tolAwardId, index2);


				auto mailInfo = CMailInfo::get_const_instance().GetInfo(3);
				if (mailInfo == nullptr)
					return;

				char mailContent[2048];

				snprintf(mailContent, sizeof(mailContent), mailInfo->content.c_str(),index2);

				CUser *user = CUtility::GetOnlineUser(j->userId);
				if (user == nullptr)
				{
					CUtility::SendOflineSysMail(j->userId, mailInfo->sender.c_str(), mailInfo->title.c_str(), mailContent, awardID, NetMsg::MailType::SystemMail);
				}
				else
				{
					CUtility::SendSysMail(user, mailInfo->sender.c_str(), mailInfo->title.c_str(), mailContent, awardID, NetMsg::MailType::SystemMail);
				}
			}

			i->second.periodExploit.clear();
			i->second.tolExploit.clear();
			i->second.startTime = now;
			i->second.endTime = now + 3600 * 24;
		}
	}
	int hour = 0;
	if (!OnTheHour(now, hour))
		return;

	for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	{
		if(!GiveAward(&i->second, hour))
			continue;

		auto & periodExploit = i->second.periodExploit;
		int index = 0;
		for(auto j = periodExploit.begin(); j != periodExploit.end(); j++)
		{
			index++;
			uint32_t periodAwardId = GetPeriodAwardByWarID(i->second.warId);
			uint32_t awardID = GetWorldWarAward(periodAwardId, index);

			auto mailInfo = CMailInfo::get_const_instance().GetInfo(4);
			if (mailInfo == nullptr)
				return;
			char mailContent[2048];

			snprintf(mailContent, sizeof(mailContent), mailInfo->content.c_str(), hour, index);
			CUser *user = CUtility::GetOnlineUser(j->userId);
			if (user == nullptr)
			{
				
				CUtility::SendOflineSysMail(j->userId,mailInfo->sender.c_str(), mailInfo->title.c_str(), mailContent, awardID, NetMsg::MailType::SystemMail);
			}
			else
			{
				CUtility::SendSysMail(user, mailInfo->sender.c_str(), mailInfo->title.c_str(),mailContent, awardID, NetMsg::MailType::SystemMail);
			}
		}
		//give award
		periodExploit.clear();



	}
}

void CWorldWar::UpdateScene(boost::posix_time::ptime now, time_t nowT)
{
	for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	{
		auto endTime = i->second.endTime;
		if (endTime > nowT)
		{
			auto scene = m_scenes.find(i->second.id);
			if (scene != m_scenes.end())
				scene->second->Update(now, nowT);
		}
	}
}

uint32_t CWorldWar::GetWorldWarAward(uint32_t groupId, uint32_t rank)
{
	uint32_t everyAwardID = 0;
	for (auto i = m_warAward.begin(); i != m_warAward.end(); i++)
	{
		WorldWarAward award;
		award.groupID = i->groupID;
		std::string strRank = i->rankString;
		char buff[32];
		snprintf(buff, sizeof(buff), "%s", strRank.c_str());
		award.awardID = i->awardID;
		if (groupId == award.groupID)
		{
			char *ranks[2];
			if (2 != CUtility::SplitLine(ranks, 2, buff, ','))
				return false;
			uint32_t rankbegin = atoi(ranks[0]);
			uint32_t rankend = atoi(ranks[1]);
			if ((rank >= rankbegin) && (rank < rankend))
			{
				everyAwardID = award.awardID;
				break;
			}
		}
	}
	return everyAwardID;
}
bool CWorldWar::readWorldWarAward()
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;
	fieldNames = { "group_id","rank","award_id"};
	if (!CReadWriteUser::ReadData(datas, fieldNames, "world_war_award"))
		return false;

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		WorldWarAward award;
		award.groupID = i->GetVal<uint32_t>(0);
		award.rankString = i->GetVal(1);
		award.awardID = i->GetVal<uint32_t>(2);
		m_warAward.push_back(award);
	}
	datas.clear();
	return true;
}
bool CWorldWar::readWordldWarTable()
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;
	fieldNames = { "id","time","provisions_cost","enemy_team","period_award_time","period_award_group","final_award_group"};
	if (!CReadWriteUser::ReadData(datas, fieldNames, "world_war"))
		return false;

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		WorldWarTable worldWarTable;
		worldWarTable.id = i->GetVal<uint32_t>(0);
		worldWarTable.time = i->GetVal<uint32_t>(1);
		worldWarTable.provisions_cost = i->GetVal<uint32_t>(2);
		worldWarTable.enemy_team = i->GetVal(3);
		worldWarTable.period_award_time = i->GetVal(4);
		worldWarTable.period_award_group = i->GetVal<uint32_t>(5);
		worldWarTable.final_award_group = i->GetVal<uint32_t>(6);
		m_warTable.push_back(worldWarTable);
	}
	datas.clear();
	return true;
}

uint32_t CWorldWar::StartWar(uint32_t type, uint32_t warId, uint32_t Force1, uint32_t Force2, uint32_t Path, float Percent)
{
	m_id++;
	WarInfo info;
	info.id = m_id;
	info.warId = warId;
	info.type = type;
	info.force1 = Force1;
	info.force2 = Force2;
	info.path = Path;
	info.percent = Percent;
	info.startTime = time(nullptr);
	info.endTime = info.startTime + 3600 * 24;

	GetEnemyByWarID(warId,info.enemyTeam);
	m_warInfos.insert(std::make_pair(info.id, info));

	CWorldWarScene* scene = new CWorldWarScene();
	scene->SetWorldWarId(m_id);
	scene->SetForce1Force(Force1);
	scene->SetForce2Force(Force2);
	scene->Init(1);
	m_scenes.insert(std::make_pair(m_id, scene));

	return info.id;
}

void CWorldWar::GetEnemyByWarID(uint32_t warID ,std::vector<uint32_t> & enemyTeam)
{
	for (auto i = m_warTable.begin(); i != m_warTable.end(); i++)
	{
		if (warID == i->id)
		{
			std::string enemy = i->enemy_team;
			char *timeId[256];

			int num = CUtility::SplitLine(timeId, 20, (char*)enemy.c_str(), ',');
			if (num <= 0)
				return ;
			for (int i = 0; i < num; i++)
			{
				enemyTeam.push_back((uint32_t)atoi(timeId[i]));
			}
		}
	}
}

CWorldWar::WarInfo *CWorldWar::GetWar(uint32_t id)
{
	auto iter = m_warInfos.find(id);
	if (iter == m_warInfos.end())
		return nullptr;
	return &(iter->second);
}

CWorldWarScene *CWorldWar::GetWorldWarScene(uint32_t id)
{
	auto iter = m_scenes.find(id);
	if (iter == m_scenes.end())
		return nullptr;
	return iter->second;
}

void CWorldWar::GetAllWars(std::list<CWorldWar::WarInfo*> &wars)
{
	for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	{
		wars.push_back(&(i->second));
	}
}

bool CWorldWar::SubmitExploit(uint32_t warId,CUser *user, int exploit)
{
	if (user == nullptr)
		return false;

	auto iter = m_warInfos.find(warId);
	if (iter == m_warInfos.end())
		return false;

	auto &periodExploit = iter->second.periodExploit;
	auto &tolExploit = iter->second.tolExploit;

	CUtility::AddExploit(user, exploit);

	auto cmp = [](const UserExploit &e1, const UserExploit &e2) {
		return e1.exploit > e2.exploit;
	};

	auto addExpolit = [=](std::vector<UserExploit> &userExps){
		bool find = false;
		for (auto i = userExps.begin(); i != userExps.end(); i++)
		{
			if (i->userId == user->GetUserId())
			{
				i->exploit += exploit;
				find = true;
				break;
			}
		}
		if (!find)
		{
			UserExploit u;
			u.userId = user->GetUserId();
			u.exploit = exploit;
			u.force = user->GetForce();
			u.level = user->GetLevel();
			std::string nick = user->GetNick();
			u.name = CUtility::GetNick(nick.c_str());
			u.title = user->GetTitle();
			userExps.push_back(u);
		}
		std::sort(userExps.begin(), userExps.end(), cmp);
	};

	addExpolit(periodExploit);
	addExpolit(tolExploit);
}

void CWorldWar::GetWorldWarTopList(CUser *user,uint32_t id, NetMsg::WorldWarTopListAck *ack)
{
	auto warInfo = GetWar(id);
	if (warInfo == nullptr || user == nullptr)
		return;

	auto setExpolit = [=](std::vector<UserExploit> &userExps, ::google::protobuf::RepeatedPtrField< ::NetMsg::WorldWarUserInfo > *netExps,uint32_t &rank,uint32_t &exploit){
		const int maxNum = 20;
		int num = 0;
		bool find = false;
		for (auto i = userExps.begin(); i != userExps.end(); i++)
		{
			if (num < maxNum)
			{
				auto userInfo = netExps->Add();
				userInfo->set_exploit(i->exploit);
				userInfo->set_force(i->force);
				userInfo->set_level(i->level);
				userInfo->set_name(i->name);
				userInfo->set_title(i->title);
				userInfo->set_userid(i->userId);
			}
			num++;
			if (i->userId == user->GetUserId())
			{
				find = true;
				rank = num;
				exploit = i->exploit;
			}
			if (find && num >= maxNum)
				break;
		}
	};
	uint32_t rank = 0;
	uint32_t exploit = 0;
	setExpolit(warInfo->periodExploit, ack->mutable_lastlist(), rank, exploit);
	ack->set_lastrank(rank);
	ack->set_lastexploit(exploit);
	ack->set_periodlefttime(GetPeriodLeftTime(warInfo->warId));
	rank = 0;
	exploit = 0;
	setExpolit(warInfo->tolExploit, ack->mutable_tollist(), rank, exploit);
	ack->set_tolrank(rank);
	ack->set_tollefttime(GetTolLeftTime(warInfo->warId));
	ack->set_tolexploit(exploit);
}

void CWorldWar::GetNextTime(uint32_t nowIndex, std::vector<uint32_t> & timeList,int & next)
{
	if (nowIndex < timeList.size())
	{
		next = timeList[nowIndex];
	}
	else
	{
		next = timeList[0];
	}
}

uint32_t CWorldWar::GetSeperateTimeByNextTime(int nextTime)
{
	uint32_t leftTime = 0;
	time_t nowTime = time(nullptr);
	if (nextTime == 0)
	{
		time_t nextDayTime = nowTime + 3600 * 24;
		tm *tmNext = localtime(&nextDayTime);
		tmNext->tm_hour = 0;
		tmNext->tm_min = 0;
		tmNext->tm_sec = 0;
		auto tmNextTime = mktime(tmNext);
		leftTime = tmNextTime - nowTime;
		return leftTime;
	}
	else
	{
		time_t nextDayTime = nowTime;
		tm *tmNext = localtime(&nextDayTime);
		tmNext->tm_hour = nextTime;
		tmNext->tm_min = 0;
		tmNext->tm_sec = 0;
		auto tmNextTime = mktime(tmNext);
		leftTime = tmNextTime - nowTime;
		return leftTime;
	}
	return 0;
}

uint32_t CWorldWar::GetPeriodLeftTime(int warId)
{
	//lolo
	uint32_t periodleftTime = 0;
	time_t nowTimeT = time(nullptr);
	tm *tmNow = localtime(&nowTimeT);
	int nowHour = tmNow->tm_hour;

	std::vector<uint32_t> timeList;

	//for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	for (auto i = m_warTable.begin(); i != m_warTable.end(); i++)
	{
		if (warId == i->id)
		{
			std::string strSortTimes = i->period_award_time;

			char *timeId[256];

			std::string temp = strSortTimes;
			int num = CUtility::SplitLine(timeId, 10, (char*)temp.c_str(), ',');
			if (num <= 0)
				return false;
			for (int i = 0; i < num; i++)
			{
				timeList.push_back((uint32_t)atoi(timeId[i]));
			}
			int index = 0;
			int nextTime = 0;
			for (auto i = timeList.begin(); i != timeList.end(); i++)
			{
				index++;
				int nowHourFlag = GetNearHour(nowHour,timeList);
				if (nowHourFlag == *i)
				{
					GetNextTime(index, timeList, nextTime);
					periodleftTime = GetSeperateTimeByNextTime(nextTime);
					return periodleftTime;
				}
			}
		}
	}
	return 0;
}

uint32_t CWorldWar::GetTolLeftTime(int warId)
{
	uint32_t tolleftTime = 0;
	for (auto i = m_warInfos.begin(); i != m_warInfos.end(); i++)
	{
		if (warId == i->second.warId)
		{
			time_t nowTime = time(nullptr);
			time_t nextTime = i->second.endTime;
			tolleftTime = nextTime - nowTime;
			return tolleftTime;
			
		}
	}
	return 0;
}
uint32_t CWorldWar::GetNearHour(uint32_t hour, const std::vector<uint32_t> & table)
{
	timeTempList.clear();
	auto fun = [=](uint32_t hour) {
		timeTempList.push_back(hour);
	};
	std::for_each(table.begin(), table.end(), fun);

	for(int i = 0;i<timeTempList.size();i++)
	{
		if(hour < timeTempList[i] && i>0)
		{
			return timeTempList[i-1];
		}
		else if(hour < timeTempList[i])
		{
			return timeTempList[i];
		}
	}
	return 0;
}