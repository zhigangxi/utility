#include "ResourceTmpl.h"
#include "Expressions.h"
#include "ReadWriteUser.h"
#include "utility.h"
#include "user.h"
#include "ResourceInst.h"
#include <sstream>  
#include <string>
#include <iostream>  
#include "HeroInst.h"
#include "NetMsg.pb.h"
#include "GlobalVal.h"
#include "ServerPb.pb.h"
using namespace std;
COnlineUser *m_onlineUser;
bool CResourceMgr::Init()
{
	std::list<CResourceTmpl*> infos;
	if (!CReadWriteUser::ReadResourceField(infos))
		return false;

	std::list<CResourcePos*> infoPos;
	if (!CReadWriteUser::ReadResourcePos(infoPos))
		return false;

	for (auto i = infoPos.begin(); i != infoPos.end(); i++)
	{
		CResourcePos * pos = *i;
		m_ResPos.insert(std::make_pair(pos->GetID(), pos));
	}

	for (auto i = infos.begin(); i != infos.end(); i++)
	{
		CResourceTmpl * info = *i;
		m_ResFields.insert(std::make_pair(info->GetID(), info));
	}
	
	return true;
}


CResourceTmpl* CResourceMgr::GetResourceInfoByIndex(uint32_t index)const
{
	auto iter = m_ResFields.find(index);
	if (iter == m_ResFields.end())
		return nullptr;
	return iter->second;
}

bool CResourceMgr::GetResourcePosId(uint32_t force, uint32_t & posId, std::unordered_map<uint32_t, uint32_t> & hasPos)const
{
	std::vector<uint32_t> allPosByForce;
	for (auto i = m_ResPos.begin(); i != m_ResPos.end(); i++)
	{
		CResourcePos * pos = i->second;
		if (pos)
		{
			auto hasOnePos = hasPos.find(pos->GetID());
			if (hasOnePos == hasPos.end())
			{
				if (pos->GetForce() == force)
				{
					allPosByForce.push_back(pos->GetID());
				}
			}
		}
	}//生成位置

	if (allPosByForce.size() > 0)
	{
		posId = CUtility::GetRandomOneFromVector(allPosByForce);
		return true;
	}
	else
	{
		return false;
	}
}

std::tuple<uint32_t, uint32_t> CResourceMgr::CalTeamCarryNum(CUser* user, NetMsg::TeamInfo * teamInfo)const
{
	auto team = teamInfo;
	std::vector<uint64_t> heros;
	for (int j = 0; j < team->heroid_size(); j++)
	{
		auto id = team->heroid(j);
		heros.push_back(id);
	}
	auto soldier = CalTeamHerosVal(user, heros);
	auto val = GetSoldierCarryInfo(soldier);
	return val;
}

uint32_t CResourceMgr::CalTeamHerosVal(CUser* user, std::vector<uint64_t> heros)const
{
	uint32_t soldier = 0;
	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		uint64_t hero = *i;
		auto heroInst = user->GetHero(hero);
		soldier += heroInst->GetHeroTmpl()->GetStrength() * 60;
	}
	return soldier;
}

std::tuple<uint32_t,uint32_t> CResourceMgr::GetSoldierCarryInfo(uint32_t soldier)const
{
	ResourceNumberData data;
	data.Soldier = soldier;
	uint32_t everyTenSecond = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateProduct(0, &data);
	uint32_t maxLoad = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateMax(0, &data);
	return{everyTenSecond, maxLoad};
}

bool CResourceMgr::CalEveryTimeResourceNum(CUser* user, uint64_t resourceUid, uint32_t teamPos)
{
	CResourceInst * resInst = user->GetResourceField(resourceUid);
	if (resInst == nullptr)
		return false;

	ServerPB::MiningInfo *findOne = GetTeamMining(user,teamPos);
	auto thisNum = findOne->carrynum();
	uint32_t onceCarry = findOne->soldiercarryone();
	uint32_t maxCarry = findOne->soldiercarrymax();

	SetMiningCarryNum(user,teamPos, thisNum + onceCarry);
	auto newCarry = GetTeamMining(user,teamPos);
	if (newCarry->carrynum() > maxCarry)
		SetMiningCarryNum(user,teamPos, maxCarry);
}

bool CResourceMgr::GetDetailResource(CUser* user, uint64_t resourceUid, NetMsg::ResourceDetailInfoAck *ack)const
{
	/*CResourceInst * resInst = user->GetResourceField(resourceUid);
	if(resInst == nullptr)
		return false;
	auto AllTeams = resInst->GetMiningInfos();
	uint32_t leftRes = resInst->GetLeftResourceNum();
	for(auto i = AllTeams.begin();i != AllTeams.end(); i++)
	{
		NetMsg::ResourceDetailInfo* teamAck = ack->add_detailinfo();
		auto oneTeam = i->second;
		teamAck->set_teampos(oneTeam.teampos());
		if (leftRes > 0) {
			ResourceNumberData data;
			data.Soldier = 1000;
			uint32_t everyTenSecond = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateProduct(0, &data);
			time_t nowTime = time(nullptr);
			uint32_t passedTime = nowTime - oneTeam.startmining();
			uint32_t product = everyTenSecond * passedTime / 10;
			uint32_t maxLoad = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateMax(0, &data);
			if (product > maxLoad)
				product = maxLoad;
			if (product > leftRes)
				product = leftRes;
			leftRes -= product;
			oneTeam.set_carrynum(product);
		}
		teamAck->set_resourcenum(oneTeam.carrynum());
	}
	ack->set_uid(resourceUid);
	ack->set_resourcenum(leftRes);*/
	return true;
}


ServerPB::MiningInfo* CResourceMgr::GetTeamMining(CUser* user, uint32_t teamPos)const
{
	auto & addData = user->GetAddData();
	auto team = addData.mutable_teams(teamPos);
	return team->mutable_mininginfo();
}

void CResourceMgr::AddMiningInfo(CUser* user,uint32_t teamPos, time_t startMining, uint32_t soldierCarryNum, uint32_t carryMax)const
{
	auto miningInfo = GetTeamMining(user, teamPos);
	miningInfo->set_startmining(time(nullptr));
	miningInfo->set_carrynum(0);
	miningInfo->set_soldiercarryone(soldierCarryNum);
	miningInfo->set_soldiercarrymax(carryMax);
}

void CResourceMgr::RemoveMiningInfo(CUser *user, uint32_t teamPos, time_t startMining, uint32_t carrynum)const
{
	//auto iter = m_miningInfos.find(teamPos);
	//if (iter != m_miningInfos.end())
	//{
	//	uint32_t leftRes = GetLeftResourceNum();
	//	auto oneTeam = iter->second;
	//	if (leftRes > 0) {
	//		ResourceNumberData data;
	//		data.Soldier = 1000;
	//		uint32_t everyTenSecond = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateProduct(0, &data);
	//		time_t nowTime = time(nullptr);
	//		uint32_t passedTime = nowTime - oneTeam.startmining();
	//		uint32_t product = everyTenSecond * passedTime / 10;
	//		uint32_t maxLoad = (int)DesignData::Expressions::sGetInstance().sResourcePlayerGenerateMax(0, &data);
	//		if (product > maxLoad)
	//			product = maxLoad;
	//		if (product > leftRes)
	//			product = leftRes;
	//		leftRes -= product;
	//		oneTeam.set_carrynum(product);
	//	}
	//	SetLeftResourceNum(leftRes);
	//	m_miningInfos.erase(teamPos);
	//}
}

void CResourceMgr::SetMiningCarryNum(CUser* user, uint32_t teamPos, uint32_t carryNum)
{
	auto mining = GetTeamMining(user, teamPos);
	mining->set_carrynum(carryNum);
}


bool CResourceMgr::TeamStartMining(uint32_t heroPos, uint32_t resourceUid, CUser* user)const
{
	auto & addData = user->GetAddData();
	auto teams = addData.mutable_teams(heroPos);
	auto team = teams->mutable_team();

	auto userAllResources = user->GetResourceFields();
	auto resField = userAllResources.find(resourceUid);
	if (resField != userAllResources.end())
	{
		CResourceInst *res = resField->second;
		uint32_t leftTime = GetResourceLeftTime(user, res->GetId());
		if (leftTime == 0)
			return false;
		time_t nowTimeT = time(nullptr);
		auto val = CalTeamCarryNum(user, team);
		uint32_t carryNum = std::get<0>(val);
		uint32_t carryMax = std::get<1>(val);
		AddMiningInfo(user, heroPos, nowTimeT, carryNum, carryMax);
	}
	return true;
}

bool CResourceMgr::TeamStopMining(uint32_t heroPos, uint32_t resourceUid, CUser* user)const
{
	auto userAllResources = user->GetResourceFields();
	auto resField = userAllResources.find(resourceUid);
	if (resField != userAllResources.end())
	{
		CResourceInst *res = resField->second;
		time_t nowTimeT = time(nullptr);
		RemoveMiningInfo(user,heroPos, nowTimeT, 0);

		uint32_t leftTime = GetResourceLeftTime(user, res->GetId());
		if (leftTime == 0)
			return false;
	}
	return true;
}
uint32_t CResourceMgr::GetResourceLeftTime(CUser* user,uint64_t uid)const
{
	CResourceInst *res = user->GetResourceField(uid);
	auto tmpl = GetResourceInfoByIndex(res->GetResourceID());
	time_t nowTime = time(nullptr);
	time_t createTime = res->GetCreateTime();
	uint32_t leftTime = createTime + tmpl->GetResCDTime() - nowTime;
	if (leftTime < 0)
		leftTime = 0;
	return leftTime;
}

void CResourceMgr::AddResourceInUser(CUser* user, uint64_t resId, uint32_t teamPos)const
{
	CResourceInst *res = user->GetResourceField(resId);
	auto tmpl = CResourceMgr::get_const_instance().GetResourceInfoByIndex(res->GetResourceID());
	auto type = tmpl->GetResType();

	uint32_t resNum = 0;
	ServerPB::MiningInfo *info = GetTeamMining(user,teamPos);
	if(info->startmining() != 0 )
	{
		 resNum += info->carrynum();
	}
	
	switch (type)
	{
		case NetMsg::ResourceType::ResourceProvisions:
		{
			auto num = user->GetProvisions();
			user->SetProvisions(num + resNum);
		}
		break;
		case NetMsg::ResourceType::ResourceWood:
		{
			auto num = user->GetWood();
			user->SetWood(num + resNum);
		}
		break;
		case NetMsg::ResourceType::ResourceStone:
		{
			auto num = user->GetStone();
			user->SetStone(num + resNum);
		}
		break;
		case NetMsg::ResourceType::ResourceIron:
		{
			auto num = user->GetResIron();
			user->SetResIron(num + resNum);
		}
		break;

		default:
			break;
	}
}

void CResourceMgr::GetMiningTeamInfoForCalculate(CUser* user)
{
	auto userAllResources = user->GetResourceFields();
	for(auto i = userAllResources.begin(); i != userAllResources.end();i++)
	{
		auto resInst = i->second;
		if(resInst)
		{
			auto resId = resInst->GetId();
			auto & addData = user->GetAddData();
			auto teams = addData.mutable_teams();
			for (int j = 0; j < teams->size(); j++)
			{
				CalEveryTimeResourceNum(user, resId,j);
			}
			if(resInst->GetLeftResourceNum() <= 0 && (0 == resInst->GetResourceClose()))
			{
				CUtility::SendUserAllResource(user);
				InformClientToBackCity(user,resInst->GetId());
				for (int k = 0; k < teams->size(); k++)
					//CUtility::SendResTeamRunPath(user, k);

				resInst->SetResourceClose(1);
			}
		}
	}
}

void CResourceMgr::ProcessOfflineResource(CUser* user)
{
	auto allres = user->GetResourceFields();
	for (auto i = allres.begin();i != allres.end(); i++)
	{
		time_t miningTime;
		time_t nowTime = time(nullptr);
		auto resInst = i->second;
		if(resInst)
		{

		}
	}
}

void CResourceMgr::ProcessOfflineResourceOneTeam(CUser* user,uint32_t teamPos)
{
	/*auto & addData = user->GetAddData();
	auto pathInfos = addData.mutable_teampaths();
	for (auto i = pathInfos->begin(); i != pathInfos->end(); i++)
	{
		 NetMsg::TeamRunPathAck & team = *i;
		 if(team.position() == teamPos)
		 {
			 NetMsg::RunPathAck *runPath = team.mutable_path();
			 auto state = runPath->state();
			 if(state == NetMsg::Runing && runPath->mutable_path()->runtarget() == NetMsg::RunToResource)
			 {
				  
			 }

			 if(state == NetMsg::ArrivedTarget && runPath->mutable_path()->runtarget() == NetMsg::RunToResource)
			 {
				 uint64_t resourceUid = runPath->mutable_path()->targetid();
				 auto couldBeMining = CResourceMgr::get_const_instance().TeamStartMining(team.position(), resourceUid, user);
			 }
		 }
	}*/
}

void CResourceMgr::InformClientToBackCity(CUser* user,uint64_t resId)
{
	auto &addData = user->GetAddData();
	auto pathInfos = addData.teams();

	auto resInst = user->GetResourceField(resId);
	if(resInst)
	{
		//for (auto i = pathInfos->begin(); i != pathInfos->end(); i++)
		//{
		//	uint32_t pos = i->position();
		//	auto findOne = GetTeamMining(user, pos);
		//	if(findOne->startmining() != 0)
		//	{
		//		std::shared_ptr<NetMsg::TeamRunPathAck> newPath (new NetMsg::TeamRunPathAck);
		//		auto curPath = *i;
		//		newPath->CopyFrom(curPath);
		//		NetMsg::RunPathAck * runPath = curPath.mutable_path();
		//		runPath->set_state(NetMsg::Runing);

		//		NetMsg::PathInfo * pathInfo = runPath->mutable_path();
		//		
		//		pathInfo->set_startpath(newPath->mutable_path()->mutable_path()->endpath());
		//		pathInfo->set_startper(newPath->mutable_path()->mutable_path()->endper());

		//		pathInfo->set_endpath(newPath->mutable_path()->mutable_path()->startpath());
		//		pathInfo->set_endper(newPath->mutable_path()->mutable_path()->startper());

		//		pathInfo->set_curpath(newPath->mutable_path()->mutable_path()->endpath());
		//		pathInfo->set_curper(newPath->mutable_path()->mutable_path()->endper());

		//		pathInfo->set_runtarget(NetMsg::RunToCity);
		//		pathInfo->set_targetid(user->GetCampCity());//首都城市
		//		
		//		curPath.set_position(pos);
		//		curPath.set_curcity(user->GetCampCity());

		//		//CUtility::CalculateRunPosCommon(runPath,10,10);
		//	}
		//}
	}
}



void CResourceMgr::DelResourceField(CUser * user)const
{
	auto allres = user->GetResourceFields();
	for (auto i = allres.begin();i != allres.end(); i++)
	{
		auto resInst = i->second;
		uint32_t leftTime = GetResourceLeftTime(user, resInst->GetId());
		if(resInst->GetResourceClose() ||
			(resInst->GetLeftResourceNum() <= 0) ||
			(leftTime <= 0))
		{
			user->DelResourceField(resInst->GetId());
		}
	}
}

//void CResourceMgr::CalResourceFiedIsClose(CUser * user)const
//{
//	auto allres = user->GetResourceFields();
//	for(auto i = allres.begin();i != allres.end(); i++)
//	{
//		auto resInst = i->second;
//		auto miningTeams = resInst->GetMiningInfos();
//		for(auto j = miningTeams.begin();j != miningTeams.end();j++)
//		{
//			auto mt = j->second;
//			AddResourceInUser(user,resInst->GetId(), mt.teampos() );
//		}
//	}
//	DelResourceField(user);
//}

void CResourceMgr::Update()
{
	auto cal = [=](CUser *toUser) {
		GetMiningTeamInfoForCalculate(toUser);
	};
	m_onlineUser = CGlobalVal::GetData<COnlineUser>("online_user");
	if(m_onlineUser)
	m_onlineUser->ForEachUser(cal);
}
