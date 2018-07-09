#include "WorldWarScene.h"
#include "WorldWarSceneAgent.h"
#include "WorldWarSceneBuilding.h"
#include "utility.h"

CWorldWarSceneBlock* CWorldWarSceneAgent::GetCurrentBlock()
{
	return m_currentBlock;
}

void CWorldWarSceneAgent::SetDefenceTarget(CWorldWarSceneBlock* block)
{
	m_defenceTarget = block;
}

void CWorldWarSceneAgent::Init(CWorldWarScene* scene)
{
	m_scene = scene;
	m_moveCD = 10;
	m_attackCD = 15;
}

void CWorldWarSceneAgent::Birth(time_t now)
{
	m_isDirty = true;
	m_forceState = STATE_DEFENCE;
	m_moveTimer = now + m_moveCD;
	m_attackTimer = now + m_attackCD;
	m_path.clear();
	m_defenceTarget = nullptr;
	CWorldWarSceneBuilding* spawnPoint = CUtility::RandomSel(*(m_scene->GetSpawnPoints(m_forceType)));
	m_currentBlock = spawnPoint->GetBlock();
	for (int i = 0; i < m_teams.size(); ++i)
		m_teams[i]->SetHealth(m_teams[i]->GetHealthMax());
}

void CWorldWarSceneAgent::Update(time_t now)
{
	if (m_forceState == STATE_MOVE)
		TryToMove(now);
	else if (m_forceState == STATE_DEFENCE)
		TryToDefence(now);
}

void CWorldWarSceneAgent::FillStatus(NetMsg::WorldWarStatusAck* ack, bool isImportant, time_t now)
{
	NetMsg::WorldWarSceneAgent* agent = nullptr;
	if (isImportant)
		agent = ack->add_importantagents();
	else
		agent = ack->add_agents();
	agent->set_agentid(m_uid);
	agent->set_userid(m_userId);
	agent->set_iscluster(m_isCluster);
	if (m_isCluster)
	{
		agent->set_clusteridorteampos(m_clusterId);
	}
	else if (m_teams.size() > 0)
	{
		agent->set_clusteridorteampos(m_teams[0]->GetPosition());
	}
	agent->set_state((NetMsg::WorldWarSceneForceState)m_forceState);
	if (m_isCluster)
	{
		int health = 0;
		int healthMax = 0;
		for (uint32_t i = 0; i < m_teams.size(); ++i)
		{
			health += m_teams[i]->GetHealth();
			healthMax += m_teams[i]->GetHealthMax();
		}
	}
	else if (m_teams.size() > 0)
	{
		agent->set_currenthealth(m_teams[0]->GetHealth());
		agent->set_totalhealth(m_teams[0]->GetHealthMax());
	}
	agent->set_attackcd(sanguo_max((uint32_t)(m_attackTimer - now), 0));
	agent->set_movecd(sanguo_max((uint32_t)(m_moveTimer - now), 0));
	agent->set_currentblockid(m_currentBlock->GetBlockId());
	if (m_defenceTarget)
		agent->set_defencetarget(m_defenceTarget->GetBlockId());
	else
		agent->set_defencetarget(0);
	for (uint32_t i = 0; i < m_path.size(); ++i)
		agent->add_path(m_path[i]->GetBlockId());
}

void CWorldWarSceneAgent::TryToMove(time_t now)
{
	if (m_path.size() > 0)
	{
		if (m_moveTimer <= now)
		{
			if (m_path[0]->GetOccupied() == FORCE_NONE || m_path[0]->GetOccupied() == m_forceType)
			{
				MoveToNextBlock(now);
			}
			else
			{
				m_defenceTarget = m_path[0];
				TryToDefence(now);

				if (m_path.size() > 0 && (m_path[0]->GetOccupied() == FORCE_NONE || m_path[0]->GetOccupied() == m_forceType))
					MoveToNextBlock(now);
			}
		}
		else
		{
			TryToDefence(now);
		}
	}

	if (m_path.size() == 0)
	{
		m_forceState = STATE_DEFENCE;
		m_defenceTarget = nullptr;
	}
}

void CWorldWarSceneAgent::MoveToNextBlock(time_t now)
{
	m_moveTimer = now + m_moveCD;
	m_currentBlock->RemoveAgent(this);
	m_path[0]->AddAgent(this);
	m_currentBlock = m_path[0];
	m_path.erase(m_path.begin());
	m_defenceTarget = nullptr;
	m_isDirty = true;
}

void CWorldWarSceneAgent::TryToDefence(time_t now)
{
	if (!m_defenceTarget && m_currentBlock && m_currentBlock->GetConnectionBlocks()->size() > 0)
	{
		std::unordered_map<uint32_t, CWorldWarSceneBlock*>* blocks = m_currentBlock->GetConnectionBlocks();
		for (auto i = blocks->begin(); i != blocks->end(); ++i)
		{
			if (i->second->GetOccupied() != FORCE_NONE && i->second->GetOccupied() != m_forceType)
			{
				m_defenceTarget = i->second;
				break;
			}
		}
	}

	if (m_attackTimer <= now && m_defenceTarget)
	{
		if (!Defence(now, m_defenceTarget))
		{
			std::unordered_map<uint32_t, CWorldWarSceneBlock*>* blocks = m_currentBlock->GetConnectionBlocks();
			for (auto i = blocks->begin(); i != blocks->end(); ++i)
			{
				if (i->second->GetBlockId() != m_defenceTarget->GetBlockId())
				{
					if (Defence(now, i->second))
						break;
				}
			}
		}
	}
}

bool CWorldWarSceneAgent::Defence(time_t now, CWorldWarSceneBlock* block)
{
	if (block->GetOccupied() != FORCE_NONE && block->GetOccupied() != m_forceType)
	{
		bool canAttack = true;
		auto iter = block->GetAttackerTimers()->find(m_currentBlock->GetBlockId());
		if (iter != block->GetAttackerTimers()->end())
			canAttack = (iter->second <= now);
		if (canAttack)
		{
			for (uint32_t i = 0; i < block->GetAgents()->size(); ++i)
			{
				CWorldWarSceneAgent* agent = (*(block->GetAgents()))[i];
				if (agent->GetAttackTimer() <= now)
				{
					CWorldWarSceneTeam* teamA = nullptr;
					uint32_t aIndex = m_teams.size() - 1;
					for (uint32_t j = 0; j < m_teams.size(); ++j)
					{
						if (m_teams[j]->GetHealth() > 0)
						{
							teamA = m_teams[j];
							aIndex = j;
							break;
						}
					}

					CWorldWarSceneTeam* teamB = nullptr;
					uint32_t bIndex = agent->GetTeams().size() - 1;
					for (uint32_t j = 0; j < agent->GetTeams().size(); ++j)
					{
						if (agent->GetTeams()[j]->GetHealth() > 0)
						{
							teamB = agent->GetTeams()[j];
							bIndex = j;
							break;
						}
					}

					if (teamA && teamB)
					{
						uint32_t attackA = sanguo_max(teamA->GetAttack(), 1);
						uint32_t attackB = sanguo_max(teamB->GetAttack(), 1);
						uint32_t timesA = (uint32_t)(ceil(teamA->GetHealth() / (float)attackB));
						uint32_t timesB = (uint32_t)(ceil(teamB->GetHealth() / (float)attackA));
						if (timesA < timesB)
						{
							// Team A is dead
							teamA->SetHealth(0);
							teamA = nullptr;
							int health = teamB->GetHealth() - attackA * timesA;
							if (health > 0)
							{
								teamB->SetHealth(health);
							}
							else
							{
								teamB->SetHealth(0);
								teamB = nullptr;
							}
						}
						else if (timesA > timesB)
						{
							// Team B is dead
							teamB->SetHealth(0);
							teamB = nullptr;
							int health = teamA->GetHealth() - attackB * timesB;
							if (health > 0)
							{
								teamA->SetHealth(health);
							}
							else
							{
								teamA->SetHealth(0);
								teamA = nullptr;
							}
						}
						else
						{
							// Both are dead
							teamA->SetHealth(0);
							teamB->SetHealth(0);
							teamA = nullptr;
							teamB = nullptr;
						}
					}

					m_attackTimer = now + m_attackCD;
					agent->SetAttackTimer(now + agent->GetAttackCD());
					m_isDirty = true;
					agent->SetIsDirty(true);

					// Agent A is dead
					if (!teamA && aIndex >= m_teams.size() - 1)
					{
						m_currentBlock->RemoveAgent(this);
						Birth(now);
					}

					// Agent B is dead
					if (!teamB && bIndex >= agent->GetTeams().size() - 1)
					{
						agent->GetCurrentBlock()->RemoveAgent(agent);
						agent->Birth(now);
					}

					return true;
				}
			}
		}
	}

	return false;
}