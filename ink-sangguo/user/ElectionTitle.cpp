#include "ElectionTitle.h"
#include "user.h"
#include "Rank.h"
#include "Title.h"
#include "Force.h"
#include "utility.h"
#include "ReadWriteUser.h"
#include <iostream>
#include "MailInfo.h"

bool CElectionTitle::Init()
{
	auto ranks = CRankMgr::get_const_instance().GetRanks();
	if (ranks->size() <= 0)
		return false;
	for (auto i = ranks->begin(); i != ranks->end(); i++)
	{
		CRank *rank = *i;
		auto titles = rank->GetTitles();
		for (auto t = titles->begin(); t != titles->end(); t++)
		{
			m_titleRanks.insert(std::make_pair(*t, rank));
		}
		auto tmpT = *titles;
		for (auto r = ranks->begin(); r != ranks->end(); r++)
		{
			CRank *rr = *r;
			if (rank->GetLevel() > rr->GetLevel())
			{
				tmpT.insert(tmpT.begin(), rr->GetTitles()->begin(),rr->GetTitles()->end());
			}
		}
		m_rankCanElectionTitles.insert(std::make_pair(rank->GetLevel(), tmpT));
	}
	auto titles = CTitleMgr::get_const_instance().GetTitles();

	for (auto i = titles->begin(); i != titles->end(); i++)
	{
		if (i->second->GetLimitNum() > 0)
		{
			TitleUpdateTime ut;
			time_t now = time(nullptr);
			tm *tmNow = localtime(&now);
			tmNow->tm_hour = i->second->GetCampaignHour();
			tmNow->tm_min = i->second->GetCampaignMinute();
			tmNow->tm_sec = 0;
			ut.title = i->second;
			m_updateTimes.push_back(ut);
		}
	}
	return true;
}

void CElectionTitle::Save()
{
}

bool CElectionTitle::CanElection(uint32_t rank, uint32_t title)
{
	auto iter = m_rankCanElectionTitles.find(rank);
	if (iter == m_rankCanElectionTitles.end())
		return false;
	auto titles = iter->second;
	return std::find(titles.begin(), titles.end(), title) != titles.end();
}

void CElectionTitle::AddTitleUser(UserForceTitle * user)
{
	ElectionTitleUser data;
	data.userId = user->userId;
	data.name = user->name;

	uint32_t force = user->force;
	auto &f = GetForceTitleUser(force, user->title);
	f.titleUsers.push_back(data);

	UserForceTitle forceTitle;
	forceTitle.force = force;
	forceTitle.name = user->name;
	forceTitle.title = user->title;
	forceTitle.userId = user->userId;
	m_allTitleUsers.insert(std::make_pair(user->userId, forceTitle));
}

void CElectionTitle::SortUser(std::list<ForceTitleElecData*>& forceTitles)
{
	auto comp = [](UserElectionData &u1, UserElectionData &u2) {
		return u1.elecData > u2.elecData;
	};
	auto equal = [](UserElectionData &electData, ElectionTitleUser &titleUser) {
		return electData.userId == titleUser.userId;
	};

	for (auto i = forceTitles.begin(); i != forceTitles.end(); i++)
	{
		auto f = (*i);
		f->sortUsers.clear();
		for (auto u = f->titleUsers.begin(); u != f->titleUsers.end(); u++)
		{
			UserElectionData data;
			data.type = HaveTitleUser;
			data.userId = u->userId;
			data.name = u->name;
			UserForceTitle electData;
			if (GetTitleUser(data.userId, electData))
			{
				data.elecData = electData.electionData;
				f->sortUsers.push_back(data);
			}
		}
		for (auto u = f->electionUsers.begin(); u != f->electionUsers.end(); u++)
		{
			UserElectionData data;
			data.type = ElectionUser;
			data.userId = u->userId;
			data.name = u->name;
			UserForceTitle electData;
			if (std::find_if(f->sortUsers.begin(), f->sortUsers.end(), std::bind(equal, std::placeholders::_1, *u))
				== f->sortUsers.end())
			{
				if (GetElectionUser(data.userId, electData))
				{
					data.elecData = electData.electionData;
					f->sortUsers.push_back(data);
				}
			}
		}
		std::sort(f->sortUsers.begin(), f->sortUsers.end(), comp);
	}
}

void CElectionTitle::ResetTitleUser(std::list<ForceTitleElecData*>& forceTitles)
{
	auto equal = [](UserElectionData &electData, ElectionTitleUser &titleUser) {
		return electData.userId == titleUser.userId;
	};
	
	const CTitleMgr &titleMgr = CTitleMgr::get_const_instance();
	for (auto i = forceTitles.begin(); i != forceTitles.end(); i++)
	{
		ForceTitleElecData *data = *i;
		CTitle *title = CTitleMgr::get_const_instance().GetTitle(data->title);
		if (title == nullptr)
			continue;
		int num = title->GetLimitNum();
		if (num <= 0)
			continue;
		
		//remove electionUsers
		for (auto u = data->electionUsers.begin(); u != data->electionUsers.end(); u++)
		{
			m_allElectionUsers.erase(u->userId);
		}

		//remove sortUsers max than limit
		if (data->sortUsers.size() > num)
			data->sortUsers.erase(data->sortUsers.begin() + num, data->sortUsers.end());

		//reset titleUsers
		for (auto u = data->titleUsers.begin(); u != data->titleUsers.end(); u++)
		{
			if (std::find_if(data->sortUsers.begin(), data->sortUsers.end(), std::bind(equal, std::placeholders::_1, *u)) 
				== data->sortUsers.end())
			{
				UserForceTitle ft;
				if(GetTitleUser(u->userId,ft))
					SetDefaultTitle(u->userId,ft.title);
			}
			m_allTitleUsers.erase(u->userId);
		}

		data->titleUsers.clear();
		data->electionUsers.clear();

		//update election user title
		for (auto u = data->sortUsers.begin(); u != data->sortUsers.end(); u++)
		{
			if (u->type == ElectionUser)
				SetUserTitle(u->userId, data->title);
			ElectionTitleUser et;
			et.userId = u->userId;
			et.name = u->name;
			//data->titleUsers.push_back(et);
			UserForceTitle ft;
			ft.electionData = 0;
			ft.force = data->force;
			ft.name = u->name;
			ft.title = data->title;
			ft.userId = u->userId;
			AddTitleUser(&ft);
		}
		
	}
}

void CElectionTitle::SetDefaultTitle(uint32_t userId, uint32_t curTitle)
{
	auto iter = m_titleRanks.find(curTitle);
	if (iter == m_titleRanks.end())
		SetUserTitle(userId, 0);
	else
		SetUserTitle(userId, iter->second->GetDefaultTitle());
}

void CElectionTitle::SetUserTitle(uint32_t userId, uint32_t title)
{
	const CTitleMgr &titleMgr = CTitleMgr::get_const_instance();
	auto t = titleMgr.GetTitle(title);
	uint32_t awardId = 0;
	if (t != nullptr)
		awardId = t->GetAwardId();
	CUser *user = CUtility::GetOnlineUser(userId);
	if (user == nullptr)
	{
		CReadWriteUser::UpdateUserTitle(userId, title);
		//CUtility::SendOflineSysMail(userId, "Title award.", "Title award.", awardId, NetMsg::MailType::TitleMail);
	}
	else
	{
		user->SetTitle(title);
		CUtility::UpdateUserInfo(user, 1<<NetMsg::UpdatePlayerAck::UpdateTitle);
		//CUtility::SendSysMail(user, "Title award.", "Title award.", awardId, NetMsg::MailType::TitleMail);
	}
}

void CElectionTitle::DelElectionTitleUser(std::list<ForceTitleElecData*>& forceTitles)
{
	bool finish;
	const CTitleMgr &titleMgr = CTitleMgr::get_const_instance();

	int num;
	do
	{
		finish = true;
		for (auto i = forceTitles.begin(); i != forceTitles.end(); i++)
		{
			auto f = *i;
			num = 0;
			CTitle *title = titleMgr.GetTitle(f->title);
			if (title == nullptr)
				continue;
			int maxNum = title->GetLimitNum();
			if (maxNum < 0)
				continue;

			for (auto j = f->sortUsers.begin(); j != f->sortUsers.end(); j++)
			{
				if (DelElTitleUser(forceTitles,*j))
					finish = false;
				num++;
				if (num >= maxNum)
					break;
			}
		}
	} while (!finish);
}


bool CElectionTitle::DelElTitleUser(std::list<ForceTitleElecData*>& forceTitles, UserElectionData & data)
{
	const CTitleMgr &titleMgr = CTitleMgr::get_const_instance();

	auto findForce = [&](uint32_t force, uint32_t title) {
		for (auto i = forceTitles.begin(); i != forceTitles.end(); i++)
		{
			auto f = *i;
			if (f->force == force && f->title == title)
				return f;
		}
		return (ForceTitleElecData*)nullptr;
	};

	UserForceTitle forceTitle;
	if (!GetTitleUser(data.userId, forceTitle))
		return false;
	auto f = findForce(forceTitle.force, forceTitle.title);
	if (f == nullptr)
		return false;
	int n = 0;
	CTitle *title = titleMgr.GetTitle(forceTitle.title);
	if (title == nullptr)
		return false;
	int mn = title->GetLimitNum();
	if (mn < 0)
		return false;
	bool ret = false;
	for (auto i = f->titleUsers.begin(); i != f->titleUsers.end(); i++)
	{
		if (data.userId == i->userId)
		{
			f->titleUsers.erase(i);
			ret = true;
			break;
		}
	}
	for (auto i = f->electionUsers.begin(); i != f->electionUsers.end(); i++)
	{
		if (data.userId == i->userId)
		{
			f->electionUsers.erase(i);
			ret = true;
			break;
		}
		n++;
		if (n >= mn)
			break;
	}
	return ret;
}

CElectionTitle::ForceTitleElecData &CElectionTitle::GetForceTitleUser(uint32_t force,uint32_t title)
{
	uint64_t mask = force;
	mask = (mask << 32) | title;
	auto iter = m_forceTitleUsers.find(mask);
	if (iter == m_forceTitleUsers.end())
	{
		ForceTitleElecData data;
		data.force = force;
		data.title = title;
		auto add = m_forceTitleUsers.insert(std::make_pair(mask, data));
		return add.first->second;
	}
	return iter->second;
}

void CElectionTitle::DelFromElectionUser(uint32_t userId)
{
	auto iter = m_allElectionUsers.find(userId);
	if (iter == m_allElectionUsers.end())
		return;
	uint32_t force = iter->second.force;
	uint32_t title = iter->second.title;
	m_allElectionUsers.erase(userId);

	auto delUser = [=](std::vector<ElectionTitleUser> *users) {
		if (users == nullptr)
			return;
		for (auto i = users->begin(); i != users->end(); i++)
		{
			if (i->userId == userId)
			{
				users->erase(i);
				break;
			}
		}
	};
	auto users = GetTitleUsers(force, title);
	delUser(users);
	users = GetElectionUsers(force, title);
	delUser(users);
}

void CElectionTitle::DelFromTitleUser(uint32_t userId)
{
	auto iter = m_allElectionUsers.find(userId);
	if (iter == m_allElectionUsers.end())
		return;
	uint32_t force = iter->second.force;
	uint32_t title = iter->second.title;
	auto users = GetElectionUsers(force, title);
	if (users == nullptr)
		return;
	for (auto i = users->begin(); i != users->end(); i++)
	{
		if (i->userId == userId)
		{
			users->erase(i);
			break;
		}
	}
}

void CElectionTitle::AddElectionUser(CUser * user, uint32_t title)
{
	DelFromElectionUser(user->GetUserId());

	ElectionTitleUser data;
	data.userId = user->GetUserId();
	data.name = user->GetNick();

	uint32_t force = user->GetForce();
	ForceTitleElecData &f = GetForceTitleUser(force,title);
	f.electionUsers.push_back(data);

	UserForceTitle forceTitle;
	forceTitle.force = force;
	forceTitle.name = user->GetNick();
	forceTitle.title = title;
	forceTitle.userId = user->GetUserId();
	m_allElectionUsers.insert(std::make_pair(user->GetUserId(),forceTitle));
}

void CElectionTitle::AddElectionData(uint32_t userId,uint32_t data)
{
	auto iter = m_allTitleUsers.find(userId);
	if (iter != m_allTitleUsers.end())
	{
		iter->second.electionData += data;
	}
	iter = m_allElectionUsers.find(userId);
	if (iter != m_allElectionUsers.end())
	{
		iter->second.electionData += data;
	}
}

std::vector<ElectionTitleUser> *CElectionTitle::GetTitleUsers(uint32_t force, uint32_t title) 
{
	ForceTitleElecData &data = GetForceTitleUser(force, title);
	return &(data.titleUsers);
}

std::vector<ElectionTitleUser> *CElectionTitle::GetElectionUsers(uint32_t force, uint32_t title)
{
	ForceTitleElecData &data = GetForceTitleUser(force, title);
	return &(data.electionUsers);
}

void CElectionTitle::Update(time_t now)
{
	for (auto i = m_updateTimes.begin(); i != m_updateTimes.end(); i++)
	{
		if (now - i->updateTime >= i->title->GetCampaignSpace() * TIME_SPACE_UNIT)
		{
			i->updateTime = now;
			i->timeIsUp = true;
		}
		else
		{
			i->timeIsUp = false;
		}
	}

	std::list<ForceTitleElecData*> forceTitles;
	auto forces = CForceMgr::get_const_instance().GetForces();
	for (auto force = forces->begin(); force != forces->end(); force++)
	{
		for (auto i = m_updateTimes.begin(); i != m_updateTimes.end(); i++)
		{
			if(i->timeIsUp)
			{
				auto &f = GetForceTitleUser(force->first, i->title->GetId());
				if(i->title->GetLimitNum() > 0 && !f.electionUsers.empty())
					forceTitles.push_back(&f);
			}
		}
	}
	if (forceTitles.size() > 0)
	{
		SortUser(forceTitles);
		DelElectionTitleUser(forceTitles);
		ResetTitleUser(forceTitles);
	}

	static int oldHour = -1;
	tm *tmNow = localtime(&now);
	
	if (oldHour != tmNow->tm_hour && tmNow->tm_hour == 3)
	{
		oldHour = tmNow->tm_hour;
		SendTitleAward();
	}
	else
	{
		oldHour = tmNow->tm_hour;
	}
}

void CElectionTitle::SendTitleAward()
{
	std::list<CReadData> datas;
	std::vector<const char*> fieldNames = { "id","title","nick" };

	std::stringstream stream;
	stream << "where last_login > to_timestamp(" << time(nullptr)-3600*24*15 << ")";
	if (!CReadWriteUser::ReadData(datas, fieldNames, "user_info", stream.str().c_str()))
	{
		std::cout << "read user title error" << std::endl;
		return;
	}
	uint32_t userId;
	auto &titleMgr = CTitleMgr::get_const_instance();
	
	auto mailInfo = CMailInfo::get_const_instance().GetInfo(1);
	if (mailInfo == nullptr)
		return;
	char mailTitle[256];
	char mailContent[2048];
	std::string nick;

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		userId = i->GetVal<uint32_t>(0);
		CUser *user = CUtility::GetOnlineUser(userId);
		if (user == nullptr)
		{
			auto title = titleMgr.GetTitle(i->GetVal<uint32_t>(1));
			if (title != nullptr)
			{
				nick = CUtility::GetNick(i->GetVal(2));
				snprintf(mailTitle, sizeof(mailTitle), mailInfo->title.c_str(), title->GetName().c_str());
				snprintf(mailContent, sizeof(mailContent), mailInfo->content.c_str(), nick.c_str());
				CUtility::SendOflineSysMail(userId, mailInfo->sender.c_str(), mailTitle,mailContent, title->GetAwardId(), NetMsg::MailType::TitleMail);
			}
		}
		else
		{
			auto title = titleMgr.GetTitle(user->GetTitle());
			if (title != nullptr)
			{
				nick = CUtility::GetNick(user->GetNick().c_str());
				snprintf(mailTitle, sizeof(mailTitle), mailInfo->title.c_str(), title->GetName().c_str());
				snprintf(mailContent, sizeof(mailContent), mailInfo->content.c_str(), nick.c_str());
				CUtility::SendSysMail(user, mailInfo->sender.c_str(), mailTitle,mailContent, title->GetAwardId(), NetMsg::MailType::TitleMail);
			}
		}
	}
}

bool CElectionTitle::GetElectionUser(uint32_t userId, uint32_t & title, uint32_t & data)
{
	auto iter = m_allElectionUsers.find(userId);
	if (iter == m_allElectionUsers.end())
		return false;

	title = iter->second.title;
	data = iter->second.electionData;
	return true;
}

int CElectionTitle::GetLeftTime(uint32_t title)
{
	int left = 0;
	for (auto i = m_updateTimes.begin(); i != m_updateTimes.end(); i++)
	{
		if (i->title->GetId() == title)
		{
			left = (int)(i->title->GetCampaignSpace() * TIME_SPACE_UNIT - (time(nullptr) - i->updateTime));
			if (left < 0)
				left = 0;
			return left;
		}
	}
	return 0;
}

bool CElectionTitle::GetTitleUser(uint32_t userId, UserForceTitle &data)
{
	auto iter = m_allTitleUsers.find(userId);
	if (iter == m_allTitleUsers.end())
		return false;
	data = iter->second;
	return true;
}

bool CElectionTitle::GetElectionUser(uint32_t userId, UserForceTitle &data)
{
	auto iter = m_allElectionUsers.find(userId);
	if (iter == m_allElectionUsers.end())
		return false;
	data = iter->second;
	return true;
}