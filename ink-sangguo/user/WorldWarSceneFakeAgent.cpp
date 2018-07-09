#include "WorldWarScene.h"
#include "WorldWarSceneAgent.h"
#include "WorldWarSceneBuilding.h"
#include "WorldWarSceneFakeAgent.h"
#include "utility.h"

void CWorldWarSceneFakeAgent::Init(CWorldWarScene* scene)
{
	m_scene = scene;
}

void CWorldWarSceneFakeAgent::UpdateFakeAgents(time_t now)
{
	if (m_fakeAgents.size() < 1500)
	{
		m_fakeAgents.push_back(m_scene->AddFakeAgent(FORCE_1));
		m_fakeAgents.push_back(m_scene->AddFakeAgent(FORCE_2));
	}

	for (uint32_t i = 0; i < m_fakeAgents.size(); ++i)
	{
		if (m_fakeAgents[i]->GetMoveCD() < now && m_fakeAgents[i]->GetPath().size() == 0)
		{
			std::unordered_map<uint32_t, CWorldWarSceneBlock*>* blocks = m_fakeAgents[i]->GetCurrentBlock()->GetConnectionBlocks();
			bool canMove = true;
			for (auto j = blocks->begin(); j != blocks->end(); ++j)
			{
				if (j->second->GetOccupied() != FORCE_NONE && j->second->GetOccupied() != m_fakeAgents[i]->GetForceType())
				{
					canMove = false;
					break;
				}
			}

			if (canMove)
			{
				int nextBlock = CUtility::RandomInt(0, blocks->size());
				for (auto j = blocks->begin(); j != blocks->end(); ++j)
				{
					if (nextBlock == 0)
					{
						m_fakeAgents[i]->SetForceState(STATE_MOVE);
						m_fakeAgents[i]->GetPath().push_back(j->second);
						break;
					}
					--nextBlock;
				}
			}
		}
	}
}