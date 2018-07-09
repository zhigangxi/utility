#pragma once
#include "NetMsg.pb.h"
#include "macros.h"
#include <string.h>

class CUser;

class CUserQuest
{
public:
	CUserQuest() :m_updateToDb(false)
	{

	}
	static CUserQuest *CreateQuest(NetMsg::QuestType type,CUser *);

	bool Init(NetMsg::QuestType type,CUser*);

	void DoQuest(uint32_t num);
	uint32_t GetTargetNum();
	uint32_t GetCurTarNum();
	uint32_t GetCost(CUser *user);

	void SetId(uint64_t id);
	uint64_t GetId();
	bool InitData(const char *str);
	void GetDataStr(std::string &str);

	NetMsg::QuestInfoAck &GetQuestData()
	{
		return m_QuestData;
	}
	uint32_t GetQuestId()
	{
		return (uint32_t)m_QuestData.type();
	}
	uint32_t GetQuestState()
	{
		return (uint32_t)m_QuestData.state();
	}
	const NetMsg::Award *GetAward();
	uint32_t GetCity();

	void GetCityAward(CUser *user, uint32_t & type);
	void RefreshAward(CUser *);

	DEFINE_PROPERTY(bool, m_updateToDb, UpdateToDb);
	
	DEFINE_STATIC_UID(uint64_t, m_curQuestId);
private:
	bool InitConsAgri(CUser*);
	bool InitConsBusi(CUser*);
	bool InitConsMilit(CUser*);
	bool InitBattleCateran(CUser*);
	bool InitVisitQuest(CUser*);
	bool InitQuestConscrip(CUser*);
	bool InitDiscovery(CUser *);

	NetMsg::QuestInfoAck m_QuestData;
};