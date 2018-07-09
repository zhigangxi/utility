#pragma once
#include "macros.h"
#include "NetMsg.pb.h"
#include <stdint.h>
#include "RankReputation.h"

class CUserTopList
{
public:
	CUserTopList() :m_sortId(0), m_playerName(""), m_force(0), m_title(false), m_military(false),
		m_reputation(0)
	{
	}
	bool Init(uint32_t type);

	DEFINE_PROPERTY(uint32_t, m_sortId, SortId);
	DEFINE_PROPERTY_REF(std::string, m_playerName, PlayerName);
	DEFINE_PROPERTY(uint32_t, m_force, Force);
	DEFINE_PROPERTY(uint32_t, m_title, Title);
	DEFINE_PROPERTY(uint32_t, m_military, Military);
	DEFINE_PROPERTY(uint32_t, m_reputation, Reputation);

	DEFINE_PROPERTY(uint32_t, m_type, Type);
	
	DEFINE_PROPERTY(uint32_t, m_myRanking, MyRanking);
	DEFINE_PROPERTY(uint32_t, m_userID, UserID);

	DEFINE_PROPERTY_REF(NetMsg::TopListData, m_topListData, TopListData);

	void GetSeperateName(std::string & playerName, std::string & newName);

	void AddSortInfoByRutation( CRankType * rank, int sortID);
	void AddSortInfoByLevel( CRankType * rank, int sortID);
	bool GetMyRankingOfReputationFromTable();
	bool GetMyRankingOfLevelFromTable();
	void SetAllMyRanking(NetMsg::TopListAck * ack, uint32_t userID);
	void GetRankingInfo(NetMsg::TopListAck * ack);
	//DEFINE_PROPERTY_REF(NetMsg::BattleAck, m_battleData, BattleData);

};

