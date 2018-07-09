#pragma once
#include <vector>

class CWorldWarScene;
class CWorldWarSceneAgent;

class CWorldWarSceneFakeAgent
{
public:
	void Init(CWorldWarScene* scene);
	void UpdateFakeAgents(time_t now);
private:
	CWorldWarScene* m_scene;
	std::vector<CWorldWarSceneAgent*> m_fakeAgents;
};