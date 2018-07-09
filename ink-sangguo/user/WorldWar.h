#pragma once
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <stdint.h>
#include <vector>
#include <time.h>
#include <list>
#include "NetMsg.pb.h"
class CUser;
class CWorldWarScene;

class CWorldWar :public boost::serialization::singleton<CWorldWar>
{
public:
	struct UserExploit
	{//军功
		uint32_t userId;
		uint32_t exploit;
		std::string name;
		uint32_t title;
		uint32_t force;
		uint32_t level;
	};

	struct WorldWarAward
	{
		uint32_t		groupID;
		std::string		rankString;
		uint32_t		awardID;
	};

	struct WorldWarTable
	{
		uint32_t		id;
		uint32_t		time;
		uint32_t		provisions_cost;
		std::string		enemy_team;
		std::string     period_award_time;
		uint32_t		period_award_group;
		uint32_t		final_award_group;
	};

	struct WarInfo
	{
		uint32_t id;//uId
		uint32_t warId;
		uint32_t force1;
		uint32_t force2;
		uint32_t type;
		uint32_t path;
		float percent;
		time_t startTime;
		time_t endTime;
		std::vector<uint32_t> enemyTeam;
		std::vector<UserExploit> periodExploit;//阶段性军功
		std::vector<UserExploit> tolExploit;//总军功
	};

	bool Init();
	uint32_t StartWar(uint32_t type, uint32_t warId, uint32_t Force1, uint32_t Force2, uint32_t Path, float Percent);
	void Update(time_t now);
	void UpdateScene(boost::posix_time::ptime now, time_t nowT);
	void GetWorldWarTopList(CUser *user,uint32_t id,NetMsg::WorldWarTopListAck *ack);
	bool readWorldWarAward();
	bool readWordldWarTable();
	void GetEnemyByWarID(uint32_t warID, std::vector<uint32_t> & enemyTeam);
	uint32_t GetWorldWarAward(uint32_t groupId, uint32_t rank);
	WarInfo *GetWar(uint32_t id);
	CWorldWarScene *GetWorldWarScene(uint32_t id);
	void GetAllWars(std::list<WarInfo*> &wars);
	bool SubmitExploit(uint32_t warId,CUser *user, int exploit);
	bool GiveAward(WarInfo * warInfo, int hour);
	uint32_t GetPeriodAwardByWarID(uint32_t warID);
	uint32_t GetTolAwardByWarID(uint32_t warID);
	uint32_t GetSeperateTimeByNextTime(int nextTime);
	void GetNextTime(uint32_t nowIndex,std::vector<uint32_t> & timeList, int & next);
	uint32_t GetPeriodLeftTime(int warId);
	uint32_t GetTolLeftTime(int warId);
	uint32_t GetNearHour(uint32_t hour, const std::vector<uint32_t> & table);
private:
	std::unordered_map<uint32_t, WarInfo> m_warInfos;
	std::unordered_map<uint32_t, CWorldWarScene*> m_scenes;
	std::vector<WorldWarAward> m_warAward;
	std::vector<WorldWarTable> m_warTable;
	std::vector<uint32_t> timeTempList;
	uint32_t m_id = 0;
};