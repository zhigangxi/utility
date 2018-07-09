#include "PackageDeal.h"
#include "user.h"
#include "command.h"
#include "OnlineUser.h"
#include "utility.h"
#include <thread>
#include "CmdPackInterface.h"
#include "GlobalVal.h"
#include "ReadWriteUser.h"
#include "ElectionTitle.h"
#include "VsBattle.h"
#include <boost/timer.hpp>
#include "WorldWar.h"
#include "RankReputation.h"
#include "ResourceTmpl.h"
using namespace std;

void CPackageDeal::AddPack(UserSession &pSession,uint32_t msgType,SharedMsg pMsg)
{
	if(pSession.get() == nullptr)
		return;
	SPack pack;
	pack.pSession = pSession;
	pack.msgType = msgType;
	pack.pMsg = pMsg;
	boost::mutex::scoped_lock lk(m_mutex);
	m_packList.push_back(pack);
}

void CPackageDeal::Logout(UserSession &pSession)
{
	boost::mutex::scoped_lock lk(m_mutex);
	m_logoutSession.push_back(pSession);
}

void CPackageDeal::AddCmdPackage(CCommand *pCommand, CCmdPackageInterface *cmdPackage)
{
	cmdPackage->Init(pCommand);
}

void CPackageDeal::Run(CCommand *pCommand)
{
	m_pOnlineUser = CGlobalVal::GetData<COnlineUser>("online_user");
	m_pCommand = pCommand;
	m_run = true;
	m_pThread = new boost::thread(boost::bind(&CPackageDeal::DealPackThread,this));
}

bool CPackageDeal::GetPack(SPack &pack)
{
	boost::mutex::scoped_lock lk(m_mutex);
	if(m_packList.empty())
		return false;
	pack = *m_packList.begin();
	m_packList.pop_front();
	return true;
}

bool CPackageDeal::GetLogout(UserSession &pSession)
{
	boost::mutex::scoped_lock lk(m_mutex);
	if(m_logoutSession.empty())
		return false;
	pSession = *m_logoutSession.begin();
	m_logoutSession.pop_front();
	return true;
}
void CPackageDeal::DealOnePack(boost::posix_time::ptime now)
{
	SPack pack;
	UserSession pSession;
	bool sleep = true;
	CElectionTitle &election = CElectionTitle::get_mutable_instance();
	auto & worldWar = CWorldWar::get_mutable_instance();
	auto & battleMgr = CVsBattleMgr::get_mutable_instance();
	auto & reputationRank = CRankReputationMgr::get_mutable_instance();
	auto & levelRank = CRankLevelMgr::get_mutable_instance();
	auto & resourceMgr = CResourceMgr::get_mutable_instance();

	static boost::posix_time::ptime oldTime;

	boost::posix_time::time_duration msdiff;

	time_t nowTimeT = time(nullptr);
	static time_t oldTimeT = time(nullptr);

	time_t resNow = time(nullptr);
	static time_t oldResNow = time(nullptr);

	if (GetLogout(pSession))
	{
		CUser *pUser = pSession->GetData<CUser>();
		if (pUser != NULL)
		{
			m_pOnlineUser->DelUser(pUser->GetUserId());
			CReadWriteUser::WriteUser(*pUser);
			delete pUser;
			pSession->SetData<CUser>(NULL);
		}
		pSession.reset();
		sleep = false;
	}
	if (GetPack(pack))
	{
		m_pCommand->RunCmd(pack.pSession, pack.msgType, pack.pMsg);
		sleep = false;
	}
	if (sleep)
		this_thread::sleep_for(chrono::milliseconds(5));

	msdiff = now - oldTime;
	if(msdiff.total_milliseconds() >= 200)
	{
		battleMgr.Update(now);
		oldTime = now;
	}

	if (nowTimeT - oldTimeT > 10)
	{
		election.Update(nowTimeT);
		worldWar.Update(nowTimeT);
		reputationRank.Update();
		levelRank.Update();
		oldTimeT = nowTimeT;
	}

	if (resNow - oldResNow > 5)
	{
		resourceMgr.Update();
		oldResNow = resNow;
	}

	worldWar.UpdateScene(now, nowTimeT);
}

void CPackageDeal::DealPackThread()
{
	srand((uint32_t)time(nullptr));

	m_run = true;
	
	while(m_run)
	{
		try
		{
			DealOnePack(boost::posix_time::microsec_clock::local_time());
		}
		catch (...)
		{
			cout << "Exception" << endl;
		}
	}
}

