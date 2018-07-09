#include "WorldWarScene.h"
#include "WorldWarSceneAgent.h"
#include "WorldWarSceneBuilding.h"

CWorldWarSceneBlock* CWorldWarSceneBuilding::GetBlock()
{
	return m_block;
}

void CWorldWarSceneBuilding::SetBlock(CWorldWarSceneBlock* block)
{
	m_block = block;
}

void CWorldWarSceneBuilding::FillStatus(NetMsg::WorldWarStatusAck* ack)
{
	NetMsg::WorldWarSceneBuildingState* state = ack->add_buildings();
	state->set_blockid(m_block->GetBlockId());
	state->set_type((NetMsg::WorldWarSceneBuildingType)m_buildingType);
	state->set_occupied((NetMsg::WorldWarSceneForceType)m_forceType);
}

void CWorldWarSceneBuildingBarn::Init(CWorldWarScene* scene)
{
	CWorldWarSceneBuilding::Init(scene);

	m_feed = 1;
	m_feedRadius = 2;
	m_feedCd = 2;
	m_feedTimer = time(nullptr);
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> checkBlocks;
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> newBlocks;
	m_feedBlocks.clear();
	m_feedBlocks.insert(std::make_pair(m_block->GetBlockId(), m_block));
	checkBlocks.insert(std::make_pair(m_block->GetBlockId(), m_block));
	for (uint32_t i = 0; i < m_feedRadius; ++i)
	{
		for (auto j = checkBlocks.begin(); j != checkBlocks.end(); ++j)
		{
			std::unordered_map<uint32_t, CWorldWarSceneBlock*>* connectionBlocks = j->second->GetConnectionBlocks();
			for (auto k = connectionBlocks->begin(); k != connectionBlocks->end(); ++k)
			{
				auto iter = m_feedBlocks.find(k->first);
				if (iter == m_feedBlocks.end())
				{
					m_feedBlocks.insert(std::make_pair(k->first, k->second));
					newBlocks.insert(std::make_pair(k->first, k->second));
				}
			}
		}

		checkBlocks.clear();
		for (auto j = newBlocks.begin(); j != newBlocks.end(); ++j)
			checkBlocks.insert(std::make_pair(j->first, j->second));
		newBlocks.clear();
	}
}

void CWorldWarSceneBuildingBarn::Update(time_t now)
{
	CWorldWarSceneBuilding::Update(now);

	if (GetForceType() != FORCE_NONE && GetFeedTimer() >= now)
	{
		SetFeedTimer(now + GetFeedCd());
		for (auto i = m_feedBlocks.begin(); i != m_feedBlocks.end(); i++)
		{
			if (i->second->GetOccupied() == GetForceType())
			{
				std::vector<CWorldWarSceneAgent*>* agents = i->second->GetAgents();
				for (uint32_t j = 0; j < agents->size(); ++j)
				{
					for (uint32_t k = 0; k < (*agents)[j]->GetTeams().size(); ++k)
					{
						CWorldWarSceneTeam* team = ((*agents)[j]->GetTeams())[k];
						if (team->GetHealth() > 0)
						{
							// Feed here
						}
					}
				}
			}
		}
	}
}

void CWorldWarSceneBuildingWithHealth::FillStatus(NetMsg::WorldWarStatusAck* ack)
{
	NetMsg::WorldWarSceneBuildingState* state = ack->add_buildings();
	state->set_blockid(m_block->GetBlockId());
	state->set_type((NetMsg::WorldWarSceneBuildingType)m_buildingType);
	state->set_occupied((NetMsg::WorldWarSceneForceType)m_forceType);
	state->set_currenthealth(m_health);
	state->set_totalhealth(m_healthMax);
}

void CWorldWarSceneBuildingTower::Init(CWorldWarScene* scene)
{
	CWorldWarSceneBuilding::Init(scene);

	m_attack = 1;
	m_attackRadius = 2;
	m_attackCd = 2;
	m_attackTimer = time(nullptr);
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> checkBlocks;
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> newBlocks;
	m_attackBlocks.clear();
	m_attackBlocks.insert(std::make_pair(m_block->GetBlockId(), m_block));
	checkBlocks.insert(std::make_pair(m_block->GetBlockId(), m_block));
	for (uint32_t i = 0; i < m_attackRadius; ++i)
	{
		for (auto j = checkBlocks.begin(); j != checkBlocks.end(); ++j)
		{
			std::unordered_map<uint32_t, CWorldWarSceneBlock*>* connectionBlocks = j->second->GetConnectionBlocks();
			for (auto k = connectionBlocks->begin(); k != connectionBlocks->end(); ++k)
			{
				auto iter = m_attackBlocks.find(k->first);
				if (iter == m_attackBlocks.end())
				{
					m_attackBlocks.insert(std::make_pair(k->first, k->second));
					newBlocks.insert(std::make_pair(k->first, k->second));
				}
			}
		}

		checkBlocks.clear();
		for (auto j = newBlocks.begin(); j != newBlocks.end(); ++j)
			checkBlocks.insert(std::make_pair(j->first, j->second));
		newBlocks.clear();
	}
}

void CWorldWarSceneBuildingTower::Update(time_t now)
{
	CWorldWarSceneBuilding::Update(now);

	if (GetHealth() > 0 && GetAttackTimer() >= now)
	{
		SetAttackTimer(now + GetAttackCd());
		for (auto i = m_attackBlocks.begin(); i != m_attackBlocks.end(); i++)
		{
			if (i->second->GetOccupied() != FORCE_NONE && i->second->GetOccupied() != GetForceType())
			{
				std::vector<CWorldWarSceneAgent*>* agents = i->second->GetAgents();
				for (uint32_t j = 0; j < agents->size(); ++j)
				{
					for (uint32_t k = 0; k < (*agents)[j]->GetTeams().size(); ++k)
					{
						CWorldWarSceneTeam* team = ((*agents)[j]->GetTeams())[k];
						if (team->GetHealth() > 0)
						{
							// Attack here
						}
					}
				}
			}
		}
	}
}

CWorldWarSceneBuilding* CWorldWarSceneBuildingFactory::CreateBuilding(WorldWarSceneBuildingType type)
{
	CWorldWarSceneBuilding* result = nullptr;
	switch (type)
	{
	case BUILDING_BASE:
	case BUILDING_STRONGHOLD:
	case BUILDING_BARRACK:
	case BUILDING_SPAWN_POINT:
	case BUILDING_CATAPULT:
		result = new CWorldWarSceneBuilding();
		break;
	case BUILDING_BARN:
		result = new CWorldWarSceneBuildingBarn();
		break;
	case BUILDING_TOWER:
		result = new CWorldWarSceneBuildingTower();
		break;
	default:
		break;
	}

	return result;
}