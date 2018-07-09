#pragma once
#include <vector>
#include "macros.h"
#include "WorldWarSceneConst.h"
#include "NetMsg.pb.h"

class CWorldWarScene;

class CWorldWarSceneTeam
{
public:
	DEFINE_PROPERTY(uint32_t, m_userId, UserId);
	DEFINE_PROPERTY(uint32_t, m_position, Position);
	DEFINE_PROPERTY(uint32_t, m_health, Health);
	DEFINE_PROPERTY(uint32_t, m_healthMax, HealthMax);
	DEFINE_PROPERTY(uint32_t, m_attack, Attack);
};

class CWorldWarSceneAgent
{
public:
	DEFINE_PROPERTY(uint32_t, m_uid, Uid);
	DEFINE_PROPERTY(uint32_t, m_userId, UserId);
	DEFINE_PROPERTY(bool, m_isDirty, IsDirty);
	DEFINE_PROPERTY(bool, m_isCluster, IsCluster);
	DEFINE_PROPERTY(uint64_t, m_clusterId, ClusterId);
	DEFINE_PROPERTY(WorldWarSceneForceType, m_forceType, ForceType);
	DEFINE_PROPERTY(WorldWarSceneForceState, m_forceState, ForceState);
	DEFINE_PROPERTY(uint32_t, m_moveCD, MoveCD);
	DEFINE_PROPERTY(uint32_t, m_attackCD, AttackCD);
	DEFINE_PROPERTY(time_t, m_moveTimer, MoveTimer);
	DEFINE_PROPERTY(time_t, m_attackTimer, AttackTimer);
	DEFINE_PROPERTY_REF(std::vector<CWorldWarSceneBlock*>, m_path, Path);
	DEFINE_PROPERTY_REF(std::vector<CWorldWarSceneTeam*>, m_teams, Teams);
	CWorldWarSceneBlock* GetCurrentBlock();
	void SetDefenceTarget(CWorldWarSceneBlock* block);
	void Init(CWorldWarScene* scene);
	void Birth(time_t now);
	void Update(time_t now);
	void FillStatus(NetMsg::WorldWarStatusAck* ack, bool isImportant, time_t now);
protected:
	void TryToMove(time_t now);
	void MoveToNextBlock(time_t now);
	void TryToDefence(time_t now);
	bool Defence(time_t now, CWorldWarSceneBlock* block);

	CWorldWarScene* m_scene;
	CWorldWarSceneBlock* m_currentBlock;
	CWorldWarSceneBlock* m_defenceTarget;
};