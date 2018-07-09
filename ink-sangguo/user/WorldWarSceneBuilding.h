#pragma once
#include <time.h>
#include <stdint.h>
#include <unordered_map>
#include "macros.h"
#include "WorldWarSceneConst.h"
#include "NetMsg.pb.h"

class CWorldWarSceneBlock;
class CWorldWarScene;

class CWorldWarSceneBuilding
{
public:
	DEFINE_PROPERTY(WorldWarSceneBuildingType, m_buildingType, BuildingType);
	DEFINE_PROPERTY(WorldWarSceneForceType, m_forceType, ForceType);
	DEFINE_PROPERTY(bool, m_isDirty, IsDirty);
	CWorldWarSceneBlock* GetBlock();
	void SetBlock(CWorldWarSceneBlock* block);
	virtual void Init(CWorldWarScene* scene) { m_scene = scene; };
	virtual void Update(time_t now) { };
	virtual void FillStatus(NetMsg::WorldWarStatusAck* ack);
protected:
	CWorldWarSceneBlock* m_block;
	CWorldWarScene* m_scene;
};

class CWorldWarSceneBuildingBarn : public CWorldWarSceneBuilding
{
public:
	DEFINE_PROPERTY(uint32_t, m_feed, Feed);
	DEFINE_PROPERTY(uint32_t, m_feedRadius, FeedRadius);
	DEFINE_PROPERTY(uint32_t, m_feedCd, FeedCd);
	DEFINE_PROPERTY(time_t, m_feedTimer, FeedTimer);
	virtual void Init(CWorldWarScene* scene);
	virtual void Update(time_t now);
private:
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> m_feedBlocks;
};

class CWorldWarSceneBuildingWithHealth : public CWorldWarSceneBuilding
{
public:
	DEFINE_PROPERTY(uint32_t, m_health, Health);
	DEFINE_PROPERTY(uint32_t, m_healthMax, HealthMax);
	virtual void FillStatus(NetMsg::WorldWarStatusAck* ack);
};

class CWorldWarSceneBuildingTower : public CWorldWarSceneBuildingWithHealth
{
public:
	DEFINE_PROPERTY(uint32_t, m_attack, Attack);
	DEFINE_PROPERTY(uint32_t, m_attackRadius, AttackRadius);
	DEFINE_PROPERTY(uint32_t, m_attackCd, AttackCd);
	DEFINE_PROPERTY(time_t, m_attackTimer, AttackTimer);
	virtual void Init(CWorldWarScene* scene);
	virtual void Update(time_t now);
private:
	std::unordered_map<uint32_t, CWorldWarSceneBlock*> m_attackBlocks;
};

class CWorldWarSceneBuildingFactory
{
public:
	static CWorldWarSceneBuilding* CreateBuilding(WorldWarSceneBuildingType type);
};