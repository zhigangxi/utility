#pragma once
#include <time.h>
#include <vector>
#include <unordered_map>
#include <boost/date_time/posix_time/ptime.hpp>
#include "macros.h"
#include "WorldWarSceneConst.h"
#include "NetMsg.pb.h"

class CWorldWarSceneBuilding;
class CDataBase;
class CWorldWarSceneAgent;
class CUser;
class CWorldWarSceneFakeAgent;

class CWorldWarSceneBlock
{
public:
	DEFINE_PROPERTY(uint32_t, m_blockId, BlockId);
	DEFINE_PROPERTY(bool, m_isDirty, IsDirty);
	DEFINE_PROPERTY(WorldWarSceneForceType, m_occupied, Occupied);
	DEFINE_PROPERTY_REF(std::vector<uint32_t>, m_connections, Connections);
	std::unordered_map<uint32_t, CWorldWarSceneBlock*>* GetConnectionBlocks();
	std::unordered_map<uint32_t, time_t>* GetAttackerTimers();
	std::vector<CWorldWarSceneAgent*>* GetAgents();
	CWorldWarSceneBuilding* GetBuilding();
	void SetBuilding(CWorldWarSceneBuilding* building);
	void AddAgent(CWorldWarSceneAgent* agent);
	void RemoveAgent(CWorldWarSceneAgent* agent);
	void FillStatus(NetMsg::WorldWarStatusAck* ack);
private:
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> m_connectionBlocks;
	std::unordered_map<uint32_t, time_t> m_attackerTimers;
	CWorldWarSceneBuilding* m_building;
	std::vector<CWorldWarSceneAgent*> m_agents;
};

class CWorldWarScene
{
public:
	DEFINE_PROPERTY(uint32_t, m_worldWarId, WorldWarId);
	DEFINE_PROPERTY(uint32_t, m_sceneId, SceneId);
	DEFINE_PROPERTY(uint32_t, m_force1Force, Force1Force);
	DEFINE_PROPERTY(uint32_t, m_force2Force, Force2Force);
	CWorldWarSceneBuilding* GetBase(WorldWarSceneForceType type);
	std::vector<CWorldWarSceneBuilding*>* GetSpawnPoints(WorldWarSceneForceType type);
	void Init(uint32_t sceneId);
	void Init(CDataBase* db);
	void Update(boost::posix_time::ptime now, time_t nowT);
	void AddUser(CUser* user, NetMsg::EnterWorldWarAck* ack);
	void FillStatus(CUser* user, NetMsg::WorldWarStatusAck* ack);
	void AgentMove(CUser* user, NetMsg::ReqWorldWarMove* req);
	void AgentDefence(CUser* user, NetMsg::ReqWorldWarDefence* req);
	void AgentSetTarget(CUser* user, NetMsg::ReqWorldWarDefenceTarget* req);
	CWorldWarSceneAgent* AddFakeAgent(WorldWarSceneForceType type);
private:
	DEFINE_PROPERTY(boost::posix_time::ptime, m_timer, Timer);
	void UpdateUserAgents(CUser* user, NetMsg::EnterWorldWarAck* ack, std::vector<CWorldWarSceneAgent*>* agents);

	uint32_t m_agentUid;
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> m_blocks;
	std::unordered_map<uint32_t, CWorldWarSceneBuilding*> m_buildings;
	std::unordered_map<WorldWarSceneForceType, CWorldWarSceneBuilding*> m_bases;
	std::unordered_map<WorldWarSceneForceType, std::vector<CWorldWarSceneBuilding*>> m_spawnPoints;
	std::unordered_map<uint32_t, CWorldWarSceneAgent*> m_agents;
	std::unordered_map<uint32_t, std::vector<CWorldWarSceneAgent*>> m_userAgents;
	std::unordered_map<WorldWarSceneForceType, std::vector<CWorldWarSceneAgent*>> m_importantAgents;
	CWorldWarSceneFakeAgent* m_fakeAgent;
};