#include "TeamCluster.h"
#include "user.h"
#include "HeroInst.h"
#include "utility.h"

bool CTeamCluster::Init(CUser * creater)
{
	if (creater == nullptr)
		return false;

	auto &addData = creater->GetAddData();
	auto teams = addData.mutable_teams();
	if (teams->size() <= 0)
		return false;
	
	uint32_t userId = creater->GetUserId();
	m_id = GenerateUID();

	bool addMember = false;
	for (int i = 0; i < teams->size(); i++)
	{
		auto team = teams->Mutable(i);
		if (team->team().clusterid() == 0)
		{
			addMember = true;
			auto info = m_clusterInfo.add_info();
			auto detail = m_clusterInfo.add_detail();
			info->set_teampos(i);
			info->set_userid(userId);
			detail->set_userid(userId);
			detail->set_username(creater->GetNick());
			auto teamInfo = team->mutable_team();
			teamInfo->set_clusterid(m_id);
			for (int i = 0; i < teamInfo->heroid_size(); i++)
			{
				auto hero = creater->GetHero(teamInfo->heroid(i));
				if (hero != nullptr)
				{
					auto heroInfo = detail->add_heros();
					heroInfo->set_heroid(hero->GetHeroTmpl()->GetId());
					heroInfo->set_level(hero->GetLevel());
				}
			}
		}
	}
	auto createrInfo = m_clusterInfo.mutable_creater();
	createrInfo->set_createrid(userId);
	createrInfo->set_name(creater->GetNick());
	m_clusterInfo.set_pub(true);
	return addMember;
}

bool CTeamCluster::Init(uint64_t id, const char * info)
{
	m_id = id;
	return CUtility::StrToMsg(info, &m_clusterInfo);
}

bool CTeamCluster::Join(CUser * user, uint32_t teamPos)
{
	if(m_clusterInfo.info().size() >= MAX_TEAM_NUM)
		return false;

	auto &addData = user->GetAddData();
	auto teams = addData.mutable_teams();
	if (teams->size() <= 0)
		return false;

	uint32_t userId = user->GetUserId();
	
	if (teams->size() >= teamPos)
		return false;

	auto team = teams->Mutable(teamPos);
	if (team->team().clusterid() != 0)
		return false;

	auto info = m_clusterInfo.add_info();
	auto detail = m_clusterInfo.add_detail();
	info->set_teampos(teamPos);
	info->set_userid(userId);
	detail->set_userid(userId);
	detail->set_username(user->GetNick());
	auto teamInfo = team->mutable_team();
	teamInfo->set_clusterid(m_id);
	for (int i = 0; i < teamInfo->heroid_size(); i++)
	{
		auto hero = user->GetHero(teamInfo->heroid(i));
		if (hero != nullptr)
		{
			auto heroInfo = detail->add_heros();
			heroInfo->set_heroid(hero->GetHeroTmpl()->GetId());
			heroInfo->set_level(hero->GetLevel());
		}
	}
	return true;
}

void CTeamCluster::GetClusterInfoAck(NetMsg::ClusterInfo * info)
{
	info->mutable_creater()->CopyFrom(m_clusterInfo.creater());
	info->set_uid(m_id);
	info->mutable_members()->CopyFrom(m_clusterInfo.info());
}

bool CTeamCluster::IsPub()
{
	return m_clusterInfo.pub();
}

int CTeamCluster::GetTeamMemNum()
{
	return m_clusterInfo.info().size();
}

bool CTeamClusterMgr::Init()
{
	return false;
}

void CTeamClusterMgr::Save()
{
}

void CTeamClusterMgr::AddCluster(CTeamCluster *c)
{
	m_teamClusters.insert(std::make_pair(c->GetId(), c));
}

CTeamCluster * CTeamClusterMgr::GetCluster(uint64_t id)
{
	auto iter = m_teamClusters.find(id);
	if(iter == m_teamClusters.end())
		return nullptr;
	return iter->second;
}

void CTeamClusterMgr::DelCluster(uint64_t id)
{
	m_teamClusters.erase(id);
}
