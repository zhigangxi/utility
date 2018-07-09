#pragma once
#include "macros.h"
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include "NetMsg.pb.h"
#include "ServerPb.pb.h"
#include "OnlineUser.h"
class CUser;
class CResourceTmpl
{
public:

	enum ResourceType {
		ResourceNone = 0,
		ResourceProvisions = 1,
		ResourceWood = 2,
		ResourceStone = 3,
		ResourceIron = 4
	};

	DEFINE_PROPERTY(uint32_t, m_Id, ID);
	DEFINE_PROPERTY(uint32_t, m_star, Star);
	DEFINE_PROPERTY(uint32_t, m_resType, ResType);
	DEFINE_PROPERTY(uint32_t, m_resCDTime, ResCDTime);
};

class CResourcePos
{
	DEFINE_PROPERTY(uint32_t, m_id, ID);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
};

struct STeamInMining
{
	time_t  startMining;
	std::vector<uint64_t> heros;
};




class CResourceMgr:public boost::serialization::singleton<CResourceMgr>
{
	public:
		bool Init();
		CResourceTmpl* GetResourceInfoByIndex(uint32_t index)const;
		bool GetResourcePosId(uint32_t force,uint32_t & posId, std::unordered_map<uint32_t, uint32_t> & hasPos)const;
		bool GetDetailResource(CUser* user, uint64_t resourceUid, NetMsg::ResourceDetailInfoAck *ack)const;
		bool TeamStartMining(uint32_t heroPos, uint32_t resourceUid, CUser* user)const;
		bool TeamStopMining(uint32_t heroPos, uint32_t resourceUid, CUser* user)const;
		void AddResourceInUser(CUser* user, uint64_t resId, uint32_t teamPos)const;
		uint32_t GetResourceLeftTime(CUser* user, uint64_t uid)const;
		bool CalEveryTimeResourceNum(CUser* user, uint64_t resourceUid, uint32_t teamPos);
		uint32_t CalTeamHerosVal(CUser* user, std::vector<uint64_t> heros)const;
		std::tuple<uint32_t, uint32_t> CalTeamCarryNum(CUser* user, NetMsg::TeamInfo * teamInfo)const;

		std::tuple<uint32_t, uint32_t> GetSoldierCarryInfo(uint32_t soldier)const;
		void Update();
		void GetMiningTeamInfoForCalculate(CUser* user);
		//void CalResourceFiedIsClose(CUser * user)const;
		void DelResourceField(CUser * user)const;
		void InformClientToBackCity(CUser* user, uint64_t resId);
		void ProcessOfflineResource(CUser* user);
		void ProcessOfflineResourceOneTeam(CUser* user, uint32_t teamPos);

		ServerPB::MiningInfo* GetTeamMining(CUser* user, uint32_t teamPos)const;
		void AddMiningInfo(CUser* user, uint32_t teamPos, time_t startMining, uint32_t soldierCarryNum, uint32_t carryMax)const;
		void RemoveMiningInfo(CUser *user, uint32_t teamPos, time_t startMining, uint32_t carrynum)const;
		void SetMiningCarryNum(CUser* user, uint32_t teamPos, uint32_t carryNum);
		//ServerPB::MiningInfo* GetOneMiningTeam(CUser* user, uint32_t teamPos);

	private:
		std::unordered_map<uint32_t, CResourceTmpl*> m_ResFields;
		std::unordered_map<uint32_t, CResourcePos *> m_ResPos;
		
};