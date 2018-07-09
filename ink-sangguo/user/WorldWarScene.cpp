#include "WorldWarScene.h"
#include "WorldWarSceneAgent.h"
#include "WorldWarSceneBuilding.h"
#include "WorldWarSceneFakeAgent.h"
#include "ReadWriteUser.h"
#include "utility.h"
#include "GlobalVal.h"
#include "user.h"
#include "HeroInst.h"

std::unordered_map<uint32_t, CWorldWarSceneBlock*>* CWorldWarSceneBlock::GetConnectionBlocks()
{
	return &(m_connectionBlocks);
}

std::unordered_map<uint32_t, time_t>* CWorldWarSceneBlock::GetAttackerTimers()
{
	return &(m_attackerTimers);
}

std::vector<CWorldWarSceneAgent*>* CWorldWarSceneBlock::GetAgents()
{
	return &(m_agents);
}

CWorldWarSceneBuilding* CWorldWarSceneBlock::GetBuilding()
{
	return m_building;
}

void CWorldWarSceneBlock::SetBuilding(CWorldWarSceneBuilding* building)
{
	m_building = building;
}

void CWorldWarSceneBlock::AddAgent(CWorldWarSceneAgent* agent)
{
	bool isNew = true;
	for (uint32_t i = 0; i < m_agents.size(); ++i)
	{
		if (m_agents[i]->GetUid() == agent->GetUid())
		{
			isNew = false;
			break;
		}
	}

	if (isNew)
	{
		m_agents.push_back(agent);
		m_occupied = agent->GetForceType();
		m_isDirty = true;
	}
}

void CWorldWarSceneBlock::RemoveAgent(CWorldWarSceneAgent* agent)
{
	for (uint32_t i = 0; i < m_agents.size(); ++i)
	{
		if (m_agents[i]->GetUid() == agent->GetUid())
		{
			m_agents.erase(m_agents.begin() + i);
			if (m_agents.size() == 0)
				m_occupied = FORCE_NONE;
			m_isDirty = true;
			break;
		}
	}
}

void CWorldWarSceneBlock::FillStatus(NetMsg::WorldWarStatusAck* ack)
{
	NetMsg::WorldWarSceneBlockState* state = ack->add_blocks();
	state->set_blockid(m_blockId);
	state->set_occupied((NetMsg::WorldWarSceneForceType)m_occupied);
	state->set_number(m_agents.size());
}


CWorldWarSceneBuilding* CWorldWarScene::GetBase(WorldWarSceneForceType type)
{
	auto iter = m_bases.find(type);
	if (iter != m_bases.end())
		return iter->second;

	return nullptr;
}

std::vector<CWorldWarSceneBuilding*>* CWorldWarScene::GetSpawnPoints(WorldWarSceneForceType type)
{
	auto iter = m_spawnPoints.find(type);
	if (iter != m_spawnPoints.end())
		return &(iter->second);

	return nullptr;
}

void CWorldWarScene::Init(uint32_t sceneId)
{
	m_sceneId = sceneId;
	m_agentUid = 0;
	CReadWriteUser::ReadWorldWarScene(this);
	m_fakeAgent = new CWorldWarSceneFakeAgent();
	m_fakeAgent->Init(this);
}

void CWorldWarScene::Init(CDataBase* db)
{
	// Read data from database
	for (int i = 0; i < db->GetRowsNum(); i++)
	{
		CWorldWarSceneBlock* block = new CWorldWarSceneBlock;
		block->SetBlockId(db->GetVal<uint32_t>(i, "id"));
		block->SetIsDirty(false);
		block->SetOccupied(FORCE_NONE);
		char* connection = db->GetVal(i, "connection");
		char* connections[6];
		int num = CUtility::SplitLine(connections, 6, connection, ',');
		for (int j = 0; j < num; j++)
			block->GetConnections().push_back(atoi(connections[j]));
		uint32_t temp = db->GetVal<uint32_t>(i, "building_type");
		WorldWarSceneBuildingType type = (WorldWarSceneBuildingType)temp;
		if (type != BUILDING_NONE)
		{
			CWorldWarSceneBuilding* building = CWorldWarSceneBuildingFactory::CreateBuilding(type);
			if (building)
			{
				building->SetBuildingType(type);
				temp = db->GetVal<uint32_t>(i, "force");
				building->SetForceType((WorldWarSceneForceType)temp);
				building->SetIsDirty(false);
				building->SetBlock(block);
				block->SetBuilding(building);
				m_buildings.insert(std::make_pair(block->GetBlockId(), building));
			}
		}
		m_blocks.insert(std::make_pair(block->GetBlockId(), block));
	}

	// Set connection of blocks
	for (auto i = m_blocks.begin(); i != m_blocks.end(); i++)
	{
		std::vector<uint32_t> connection = i->second->GetConnections();
		std::unordered_map<uint32_t, CWorldWarSceneBlock*>* connectionBlocks = i->second->GetConnectionBlocks();
		for (uint32_t j = 0; j < connection.size(); j++)
		{
			auto iter = m_blocks.find(connection[j]);
			if (iter != m_blocks.end())
				connectionBlocks->insert(std::make_pair(connection[j], iter->second));
		}
		i->second->GetAttackerTimers()->clear();
		i->second->GetAgents()->clear();
	}

	// Init buildings
	m_bases.clear();
	m_spawnPoints.clear();
	std::vector<CWorldWarSceneBuilding*> force1SpawnPoint;
	std::vector<CWorldWarSceneBuilding*> force2SpawnPoint;
	for (auto i = m_buildings.begin(); i != m_buildings.end(); i++)
	{
		i->second->Init(this);
		if (i->second->GetBuildingType() == BUILDING_BASE)
		{
			m_bases.insert(std::make_pair(i->second->GetForceType(), i->second));
		}
		else if (i->second->GetBuildingType() == BUILDING_SPAWN_POINT)
		{
			if (i->second->GetForceType() == FORCE_1)
				force1SpawnPoint.push_back(i->second);
			else if (i->second->GetForceType() == FORCE_2)
				force2SpawnPoint.push_back(i->second);
		}
	}
	m_spawnPoints.insert(std::make_pair(FORCE_1, force1SpawnPoint));
	m_spawnPoints.insert(std::make_pair(FORCE_2, force2SpawnPoint));

	m_agents.clear();
	m_userAgents.clear();
	m_importantAgents.clear();
}

void CWorldWarScene::Update(boost::posix_time::ptime now, time_t nowT)
{
	boost::posix_time::time_duration msdiff = now - m_timer;
	if (msdiff.total_milliseconds() >= 500)
	{
		int64_t delta = sanguo_min(msdiff.total_milliseconds() - 500, 1000);
		m_timer = now - boost::posix_time::milliseconds(delta);

		// Update agents
		for (auto i = m_agents.begin(); i != m_agents.end(); i++)
			i->second->Update(nowT);

		// Update buildings
		for (auto i = m_buildings.begin(); i != m_buildings.end(); i++)
			i->second->Update(nowT);

		bool sendMessage = false;
		NetMsg::WorldWarStatusAck *ack = new NetMsg::WorldWarStatusAck;
		SharedMsg ackMsg(ack);
		ack->set_error(0);

		// Gather changed blocks
		for (auto i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			if (i->second->GetIsDirty())
			{
				sendMessage = true;
				i->second->FillStatus(ack);
			}
		}

		// Gather changed buildings
		for (auto i = m_buildings.begin(); i != m_buildings.end(); i++)
		{
			if (i->second->GetIsDirty())
			{
				sendMessage = true;
				i->second->FillStatus(ack);
			}
		}

		// Gather changed important agents
		for (auto i = m_importantAgents.begin(); i != m_importantAgents.end(); i++)
		{
			std::vector<CWorldWarSceneAgent*>& agents = i->second;
			for (uint32_t j = 0; j < agents.size(); ++j)
			{
				if (agents[j]->GetIsDirty())
				{
					sendMessage = true;
					agents[j]->FillStatus(ack, true, nowT);
				}
			}
		}

		// Send message to all online users
		for (auto i = m_userAgents.begin(); i != m_userAgents.end(); i++)
		{
			CUser* user = CUtility::GetOnlineUser(i->first);
			if (user && user->GetIsInWorldWarScene())
			{
				ack->clear_agents();
				std::vector<CWorldWarSceneAgent*>& agents = i->second;
				for (uint32_t j = 0; j < agents.size(); ++j)
				{
					if (agents[j]->GetIsDirty())
					{
						sendMessage = true;
						agents[j]->FillStatus(ack, false, nowT);
					}
				}

				if (sendMessage)
				{
					// Send message
					CUtility::SendWorldWarStatus(user, ackMsg);
				}
			}
		}

		for (auto i = m_blocks.begin(); i != m_blocks.end(); i++)
			i->second->SetIsDirty(false);

		for (auto i = m_agents.begin(); i != m_agents.end(); i++)
			i->second->SetIsDirty(false);

		for (auto i = m_buildings.begin(); i != m_buildings.end(); i++)
			i->second->SetIsDirty(false);

		m_fakeAgent->UpdateFakeAgents(nowT);
	}
}

void CWorldWarScene::AddUser(CUser* user, NetMsg::EnterWorldWarAck* ack)
{
	auto iter = m_userAgents.find(user->GetUserId());
	if (iter != m_userAgents.end())
	{
		UpdateUserAgents(user, ack, &iter->second);
	}
	else
	{
		std::vector<CWorldWarSceneAgent*> agents;
		UpdateUserAgents(user, ack, &agents);
		m_userAgents.insert(std::make_pair(user->GetUserId(), agents));
	}
	ack->set_sceneid(m_sceneId);
	if (user->GetForce() == m_force1Force)
		ack->set_force((NetMsg::WorldWarSceneForceType)FORCE_1);
	else
		ack->set_force((NetMsg::WorldWarSceneForceType)FORCE_2);
	user->SetIsInWorldWarScene(true);
}

void CWorldWarScene::UpdateUserAgents(CUser* user, NetMsg::EnterWorldWarAck* ack, std::vector<CWorldWarSceneAgent*>* agents)
{
	if (user->GetForce() == m_force1Force || user->GetForce() == m_force2Force)
	{
		auto &addData = user->GetAddData();

		// Check teams
		auto teams = addData.mutable_teams();
		int index = 0;
		for (auto i = teams->begin(); i != teams->end(); ++i)
		{
			auto path = i->mutable_path();
			// Team has reach world war position
			if (path->state() == NetMsg::ArrivedTarget &&
				path->mutable_path()->runtarget() == NetMsg::RunToWorldWar &&
				(uint32_t)path->mutable_path()->targetid() == m_worldWarId)
			{
				NetMsg::TeamInfo* teamInfo = i->mutable_team();

				if (teamInfo)
				{
					bool isNew = true;
					for (uint32_t j = 0; j < agents->size(); ++j)
					{
						CWorldWarSceneAgent* agent = (*agents)[j];
						if (!agent->GetIsCluster() && agent->GetTeams().size() > 0 &&
							(agent->GetTeams())[0]->GetPosition() == index)
						{
							isNew = false;
							break;
						}
					}

					if (isNew)
					{
						CWorldWarSceneTeam* team = new CWorldWarSceneTeam();
						team->SetUserId(user->GetUserId());
						team->SetPosition(index);
						uint32_t health = 0;
						uint32_t attack = 0;
						for (int j = 0; j < teamInfo->heroid_size(); ++j)
						{
							CHeroInst* hero = user->GetHero(teamInfo->heroid(j));
							if (hero)
							{
								health += hero->GetTolLeadership();
								attack += hero->GetTolStrength();
							}
						}
						health *= 1000;
						team->SetHealth(health);
						team->SetHealthMax(health);
						team->SetAttack(attack);

						CWorldWarSceneAgent* agent = new CWorldWarSceneAgent();
						agent->GetTeams().push_back(team);
						agent->Init(this);
						agent->SetUid(++m_agentUid);
						agent->SetUserId(user->GetUserId());
						agent->SetIsCluster(false);
						if (user->GetForce() == m_force1Force)
							agent->SetForceType(FORCE_1);
						else if(user->GetForce() == m_force2Force)
							agent->SetForceType(FORCE_2);
						agent->Birth(time(nullptr));
						m_agents.insert(std::make_pair(agent->GetUid(), agent));
						agents->push_back(agent);
					}
				}
			}
			++index;
		}

		// Check cluster
		// To do
	}
}

void CWorldWarScene::FillStatus(CUser* user, NetMsg::WorldWarStatusAck* ack)
{
	for (auto i = m_blocks.begin(); i != m_blocks.end(); i++)
		i->second->FillStatus(ack);

	for (auto i = m_buildings.begin(); i != m_buildings.end(); i++)
		i->second->FillStatus(ack);

	time_t now = time(nullptr);
	for (auto i = m_importantAgents.begin(); i != m_importantAgents.end(); i++)
	{
		std::vector<CWorldWarSceneAgent*>& agents = i->second;
		for (uint32_t j = 0; j < agents.size(); ++j)
			agents[j]->FillStatus(ack, true, now);
	}

	auto iter = m_userAgents.find(user->GetUserId());
	if (iter != m_userAgents.end())
	{
		for (uint32_t i = 0; i < iter->second.size(); ++i)
		{
			CWorldWarSceneAgent* agent = (iter->second)[i];
			agent->FillStatus(ack, false, now);
		}
	}
}

void CWorldWarScene::AgentMove(CUser* user, NetMsg::ReqWorldWarMove* req)
{
	auto iter = m_agents.find(req->agentid());
	if (iter != m_agents.end())
	{
		if (iter->second->GetUserId() == user->GetUserId() && iter->second->GetForceState() == STATE_DEFENCE)
		{
			iter->second->SetForceState(STATE_MOVE);
			iter->second->GetPath().clear();
			CWorldWarSceneBlock* checkBlock = iter->second->GetCurrentBlock();
			for (int i = 0; i < req->path_size(); ++i)
			{
				auto iterr = checkBlock->GetConnectionBlocks()->find(req->path(i));
				if (iterr != checkBlock->GetConnectionBlocks()->end())
				{
					iter->second->GetPath().push_back(iterr->second);
					checkBlock = iterr->second;
					iter->second->SetIsDirty(true);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void CWorldWarScene::AgentDefence(CUser* user, NetMsg::ReqWorldWarDefence* req)
{
	auto iter = m_agents.find(req->agentid());
	if (iter != m_agents.end())
	{
		if (iter->second->GetUserId() == user->GetUserId() && iter->second->GetForceState() == STATE_MOVE)
		{
			iter->second->SetForceState(STATE_DEFENCE);
			iter->second->GetPath().clear();
			iter->second->SetDefenceTarget(nullptr);
			iter->second->SetIsDirty(true);
		}
	}
}

void CWorldWarScene::AgentSetTarget(CUser* user, NetMsg::ReqWorldWarDefenceTarget* req)
{
	auto iter = m_agents.find(req->agentid());
	if (iter != m_agents.end())
	{
		if (iter->second->GetUserId() == user->GetUserId())
		{
			if (iter->second->GetForceState() == STATE_MOVE)
			{
				iter->second->SetForceState(STATE_DEFENCE);
				iter->second->GetPath().clear();
			}

			CWorldWarSceneBlock* checkBlock = iter->second->GetCurrentBlock();
			auto iterr = checkBlock->GetConnectionBlocks()->find(req->blockid());
			if (iterr != checkBlock->GetConnectionBlocks()->end())
				iter->second->SetDefenceTarget(iterr->second);
			iter->second->SetIsDirty(true);
		}
	}
}

CWorldWarSceneAgent* CWorldWarScene::AddFakeAgent(WorldWarSceneForceType type)
{
	CWorldWarSceneTeam* team = new CWorldWarSceneTeam();
	team->SetUserId(0);
	team->SetPosition(0);
	uint32_t health = CUtility::RandomInt(1000, 4000);
	uint32_t attack = CUtility::RandomInt(100, 400);
	team->SetHealth(health);
	team->SetHealthMax(health);
	team->SetAttack(attack);

	CWorldWarSceneAgent* agent = new CWorldWarSceneAgent();
	agent->GetTeams().push_back(team);
	agent->Init(this);
	agent->SetUid(++m_agentUid);
	agent->SetUserId(0);
	agent->SetIsCluster(false);
	agent->SetForceType(type);
	agent->Birth(time(nullptr));
	m_agents.insert(std::make_pair(agent->GetUid(), agent));
	return agent;
}