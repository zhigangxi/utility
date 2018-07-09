#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <string>
#include <time.h>
#include <vector>
#include <list>

class CUser;
class CRank;
class CTitle;

struct ElectionTitleUser
{
	uint32_t userId;
	std::string name;
};

class CElectionTitle :public boost::serialization::singleton<CElectionTitle>
{
public:
	bool Init();
	void Save();
	void AddElectionUser(CUser *user,uint32_t title);//添加候选人
	void AddElectionData(uint32_t userId,uint32_t data);
	bool CanElection(uint32_t rank, uint32_t title);

	std::vector<ElectionTitleUser> *GetTitleUsers(uint32_t force, uint32_t title);
	uint32_t GetTitleUserNum(uint32_t force, uint32_t title)
	{
		auto users = GetTitleUsers(force, title);
		if (users == nullptr)
			return 0;
		return users->size();
	}
	std::vector<ElectionTitleUser> *GetElectionUsers(uint32_t force, uint32_t title);
	uint32_t GetElectionUserNum(uint32_t force,uint32_t title)
	{
		auto users = GetElectionUsers(force,title);
		if (users == nullptr)
			return 0;
		return users->size();
	}
	void Update(time_t now);
	bool GetElectionUser(uint32_t userId, uint32_t &title, uint32_t &data);
	int GetLeftTime(uint32_t title);
private:
	void DelFromElectionUser(uint32_t userId);
	void DelFromTitleUser(uint32_t userId);

	struct UserForceTitle
	{
		uint32_t userId;
		uint32_t force;
		uint32_t title;
		std::string name;
		uint32_t electionData = 0;
	};
	bool GetTitleUser(uint32_t userId, UserForceTitle &data);
	bool GetElectionUser(uint32_t userId, UserForceTitle &data);

	void AddTitleUser(UserForceTitle *user);//添加是某个title的人

	enum ElecUserType
	{
		HaveTitleUser,
		ElectionUser,
	};
	struct UserElectionData
	{
		uint32_t userId;
		uint32_t elecData;
		std::string name;
		ElecUserType type;
	};
	struct ForceTitleElecData
	{
		uint32_t force;
		uint32_t title;
		std::vector<ElectionTitleUser> titleUsers;
		std::vector<ElectionTitleUser> electionUsers;
		std::vector<UserElectionData> sortUsers;
	};
	void SortUser(std::list<ForceTitleElecData*> &forceTitles);
	void DelElectionTitleUser(std::list<ForceTitleElecData*> &forceTitles);
	void ResetTitleUser(std::list<ForceTitleElecData*> &forceTitles);

	void SetDefaultTitle(uint32_t userId, uint32_t curTitle);

	void SetUserTitle(uint32_t userId, uint32_t title);

	bool DelElTitleUser(std::list<ForceTitleElecData*> &forceTitles, UserElectionData &user);
	void SendTitleAward();

	ForceTitleElecData &GetForceTitleUser(uint32_t force,uint32_t title);

	std::unordered_map<uint64_t, ForceTitleElecData> m_forceTitleUsers;

	std::unordered_map<uint32_t, UserForceTitle> m_allTitleUsers;
	std::unordered_map<uint32_t, UserForceTitle> m_allElectionUsers;

	//std::unordered_map<uint32_t, uint32_t> m_updateUserTitle;//userid,title

	std::unordered_map<uint32_t, CRank*> m_titleRanks;//title,rank

	std::unordered_map<uint32_t, std::list<uint32_t>> m_rankCanElectionTitles;//rank,titles

	struct TitleUpdateTime
	{
		CTitle *title;
		time_t updateTime = 0;
		bool timeIsUp;
	};

	std::list<TitleUpdateTime> m_updateTimes;
	const static int TIME_SPACE_UNIT = 60*60*24;
};