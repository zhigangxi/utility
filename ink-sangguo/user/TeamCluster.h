#pragma once

#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <string>
#include <list>
#include "macros.h"
#include "ServerPb.pb.h"
#include <unordered_map>

class CUser;

class CTeamCluster
{
public:
	bool Init(CUser *creater);
	bool Init(uint64_t id, const char *info);
	bool Join(CUser *user, uint32_t teamPos);
	void GetClusterInfoAck(NetMsg::ClusterInfo *info);
	bool IsPub();
	bool Exit(CUser *user, uint32_t teamPos);

	int GetTeamMemNum();
	const int MAX_TEAM_NUM = 5;
private:
	DEFINE_PROPERTY(uint64_t, m_id, Id);
	DEFINE_STATIC_UID(uint64_t, m_curUId);

	DEFINE_PROPERTY_REF(ServerPB::UserCluster, m_clusterInfo, ClusterInfo);
};

class CTeamClusterMgr :public boost::serialization::singleton<CTeamClusterMgr>
{
public:
	bool Init();
	void Save();
	void AddCluster(CTeamCluster *);
	CTeamCluster *GetCluster(uint64_t id);
	std::unordered_map<uint64_t, CTeamCluster*> *GetAllCluster()
	{
		return &m_teamClusters;
	}
	void DelCluster(uint64_t id);
private:
	std::unordered_map<uint64_t,CTeamCluster*> m_teamClusters;
};