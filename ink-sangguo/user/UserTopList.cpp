#include "UserTopList.h"
#include "ReadWriteUser.h"
#include <algorithm>
#include <boost/random.hpp>
#include "utility.h"
#include "user.h"
bool CUserTopList::Init(uint32_t type)
{
	m_type = type;
	return true;
}

void  CUserTopList::GetSeperateName(std::string & playerName, std::string & newName)
{
	char *p[3];
	char * buf = new char[strlen(playerName.c_str()) + 1];
	strcpy(buf, playerName.c_str());

	if (3 == CUtility::SplitLine(p, 3, buf, '|'))
	{
		newName.append(p[0]);
		newName.append(p[2]);
	}
}

void CUserTopList::AddSortInfoByRutation( CRankType * rank ,int sortID)
{
	m_topListData.mutable_sortreputation()->set_sortid(sortID);

	m_topListData.mutable_sortreputation()->set_icon(rank->GetIcon());

	m_topListData.mutable_sortreputation()->set_userid(rank->GetUserID());

	std::string newName;
	GetSeperateName(rank->GetPlayerName(),newName);
	m_topListData.mutable_sortreputation()->set_playername(newName);

	m_topListData.mutable_sortreputation()->set_force(rank->GetForce());
	m_topListData.mutable_sortreputation()->set_level(rank->GetLevel());
	m_topListData.mutable_sortreputation()->set_title(rank->GetTitle());
	m_topListData.mutable_sortreputation()->set_reputation(rank->GetReputation());
}

void CUserTopList::AddSortInfoByLevel( CRankType * rank, int sortID)
{
	m_topListData.mutable_sortlevel()->set_sortid(sortID);

	m_topListData.mutable_sortlevel()->set_icon(rank->GetIcon());
	m_topListData.mutable_sortlevel()->set_userid(rank->GetUserID());

	std::string newName;
	GetSeperateName(rank->GetPlayerName(), newName);
	m_topListData.mutable_sortlevel()->set_playername(newName);

	m_topListData.mutable_sortlevel()->set_force(rank->GetForce());
	m_topListData.mutable_sortlevel()->set_level(rank->GetLevel());
	m_topListData.mutable_sortlevel()->set_title(rank->GetTitle());
	m_topListData.mutable_sortlevel()->set_military(rank->GetMilitary());
}


bool CUserTopList::GetMyRankingOfReputationFromTable()
{
	if (!CReadWriteUser::ReadOwnReputaion(this))
		return false;
	return true;
}

bool CUserTopList::GetMyRankingOfLevelFromTable()
{
	if (!CReadWriteUser::ReadOwnLevel(this))
		return false;
	return true;
}

void CUserTopList::SetAllMyRanking(NetMsg::TopListAck * ack, uint32_t userID)
{
	m_userID = userID;

	switch (m_type)
	{
		case NetMsg::TopListType::Reputation:
		{
			if (GetMyRankingOfReputationFromTable()) {
				ack->set_myranking(m_myRanking);
			}
			else {
				//uncorrect data
				ack->set_myranking(0);
			}
		}
		break;

		case NetMsg::TopListType::Level:
		{
			if (GetMyRankingOfLevelFromTable()) {
				ack->set_myranking(m_myRanking);
			}
			else {
				//uncorrect data
				ack->set_myranking(0);
			}
		}
		break;
		default:
			break;
	}
}

void CUserTopList::GetRankingInfo(NetMsg::TopListAck * ack)
{
	//m_topListData = 

	switch (m_type)
	{
		case NetMsg::TopListType::Reputation:
		{
			auto titles = CRankReputationMgr::get_const_instance().GetRankReputation();
			int sortIndex = 1;
			for (auto i = titles->begin(); i != titles->end(); i++)
			{
				CRankType *rank = *i;
				AddSortInfoByRutation( *i, sortIndex);
				sortIndex++;
				NetMsg::TopListData * sortData = ack->add_topdata();
				sortData->CopyFrom(m_topListData);
			}
		}
		break;
		case NetMsg::TopListType::Level:
		{
			auto titles = CRankLevelMgr::get_const_instance().GetRankLevel();
			int sortIndex = 1;
			for (auto i = titles->begin(); i != titles->end(); i++)
			{
				CRankType *rank = *i;
				AddSortInfoByLevel(rank, sortIndex);
				sortIndex++;
				NetMsg::TopListData * sortData = ack->add_topdata();
				sortData->CopyFrom(m_topListData);
			}
		}
		break;
	default:
		break;
	}


}
