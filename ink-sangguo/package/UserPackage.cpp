#include "UserPackage.h"
#include "command.h"
#include "NetMsg.pb.h"
#include "GlobalVal.h"
#include "ReadWriteUser.h"
#include "user.h"
#include "error.pb.h"
#include "utility.h"
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <sstream>
#include <boost/format.hpp>
#include "RsaCode.h"
#include "OnlineUser.h"
#include <string.h>
#include "Force.h"
#include "HeroInst.h"
#include "WorldPath.h"
#include "WorldCity.h"
#include "UserQuest.h"
#include "Title.h"
#include "Quest.h"
#include "HeroTmpl.h"
#include "UserBattle.h"
#include "EnemyTeam.h"
#include "ItemInst.h"
#include "ItemTmpl.h"
#include "Award.h"
#include "Expressions.h"
#include "Shop.h"
#include "Advanture.h"
#include "Evolve.h"
#include "Rank.h"
#include "RankReputation.h"
#include "UserTopList.h"
#include "LearnSkill.h"
#include "LearnProperty.h"
#include "ElectionTitle.h"
#include "CampMgr.h"
#include "CycleBattle.h"
#include "RecruitHero.h"
#include "VsBattle.h"
#include "Biography.h"
#include "RaffleHero.h"
#include "Banquet.h"
#include "MailInfo.h"
#include "WorldWar.h"
#include "WorldWarScene.h"
#include "LearnAndVsCD.h"
#include "GeneralSoulMgr.h"
#include "HeroSkillBook.h"
#include "ResourceInst.h"
#include "ResourceTmpl.h"
#include <set>
#include "TeamCluster.h"

#define GET_REQ_MSG(req,MsgType,msg) \
	req = static_cast<MsgType*>(msg.get()); \
	if (req == nullptr) \
		return;

#define DEF_GET_REQ_MSG(req,MsgType,msg) \
	MsgType *req;\
	req = static_cast<MsgType*>(msg.get()); \
	if (req == nullptr) \
		return;

#define GET_LOGIN_USER(user,session) \
	user = session->GetData<CUser>();\
	if (user == nullptr || user->GetUserId() == 0 || user->GetAccountId() == 0) \
		return;

#define GET_USER(user,session) \
	user = session->GetData<CUser>();\
	if (user == nullptr) \
		return;

#define DEF_GET_USER(user,session) \
	CUser *user;\
	user = session->GetData<CUser>();\
	if (user == nullptr) \
		return;

CUserPackage::CUserPackage()
{
	m_rsa = new CRsaCode;
}

CUserPackage::~CUserPackage()
{
	delete m_rsa;
}

void CUserPackage::Init(CCommand *cmd)
{
	CReadWriteUser::ReadBadWrods(m_badWords);

	m_rsa->SetPubKey("sign_key.pub");

	m_socket = CGlobalVal::GetData<CSocket>("server_socket");
	m_onlineUser = CGlobalVal::GetData<COnlineUser>("online_user");

	cmd->AddCommand(NetMsg::MessageType::ReqVersionInfo, std::bind(&CUserPackage::GetClientVersion,this,std::placeholders::_1,std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqTest, std::bind(&CUserPackage::Test, this, std::placeholders::_1, std::placeholders::_2));

	//ReqLogin ;//登陆
	cmd->AddCommand(NetMsg::MessageType::ReqLogin, std::bind(&CUserPackage::UserLogin, this, std::placeholders::_1, std::placeholders::_2));

	//ReqUserNamePass ;//请求用户名密码（快速登陆）
	cmd->AddCommand(NetMsg::MessageType::ReqUserNamePass, std::bind(&CUserPackage::GetNamePassword, this, std::placeholders::_1, std::placeholders::_2));

	//ReqRegister ;//注册
	cmd->AddCommand(NetMsg::MessageType::ReqRegister, std::bind(&CUserPackage::RegAccount, this, std::placeholders::_1, std::placeholders::_2));
	
	//ReqEnterGame ;//进入游戏
	cmd->AddCommand(NetMsg::MessageType::ReqEnterGame, std::bind(&CUserPackage::EnterGame, this, std::placeholders::_1, std::placeholders::_2));
	
	//ReqCreateUser ;//创建角色
	cmd->AddCommand(NetMsg::MessageType::ReqCreateUser, std::bind(&CUserPackage::CreateUser, this, std::placeholders::_1, std::placeholders::_2));

	//ReqServerList ;//请求服务器列表
	cmd->AddCommand(NetMsg::MessageType::ReqServerList, std::bind(&CUserPackage::GetServerList, this, std::placeholders::_1, std::placeholders::_2));

	//ReqThirdPartyLogin//第三方账号登陆
	cmd->AddCommand(NetMsg::MessageType::ReqThirdPartyLogin, std::bind(&CUserPackage::ThirdPartyLogin, this, std::placeholders::_1, std::placeholders::_2));

	//获取订单号
	cmd->AddCommand(NetMsg::MessageType::ReqOrderId, std::bind(&CUserPackage::GetOrderId, this, std::placeholders::_1, std::placeholders::_2));

	//聊天
	cmd->AddCommand(NetMsg::MessageType::ReqChat, std::bind(&CUserPackage::Chat, this, std::placeholders::_1, std::placeholders::_2));

	//获取昵称
	cmd->AddCommand(NetMsg::MessageType::ReqUserNick, std::bind(&CUserPackage::GetUserNick, this, std::placeholders::_1, std::placeholders::_2));	

	//新手引导
	cmd->AddCommand(NetMsg::MessageType::SetNewGuide, std::bind(&CUserPackage::SetNewGuide, this, std::placeholders::_1, std::placeholders::_2));
	
	//英雄信息
	cmd->AddCommand(NetMsg::MessageType::ReqHero, std::bind(&CUserPackage::GetHeros, this, std::placeholders::_1, std::placeholders::_2));
	
	//请求当前路径
	cmd->AddCommand(NetMsg::MessageType::ReqRunPath, std::bind(&CUserPackage::GetRunPath, this, std::placeholders::_1, std::placeholders::_2));

	//设置路径
	cmd->AddCommand(NetMsg::MessageType::SetRunPath, std::bind(&CUserPackage::SetRunPath, this, std::placeholders::_1, std::placeholders::_2));

	//到达城市
	cmd->AddCommand(NetMsg::MessageType::ArrivedCity, std::bind(&CUserPackage::ArrivedCity, this, std::placeholders::_1, std::placeholders::_2));

	//阵型
	cmd->AddCommand(NetMsg::MessageType::ReqHeroFormation, std::bind(&CUserPackage::GetHeroFormation, this, std::placeholders::_1, std::placeholders::_2));
	
	//阵型
	cmd->AddCommand(NetMsg::MessageType::SetHeroFormation, std::bind(&CUserPackage::SetHeroFormation, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqCityInfo, std::bind(&CUserPackage::GetCityInfo, this, std::placeholders::_1, std::placeholders::_2));

	//任务
	cmd->AddCommand(NetMsg::MessageType::ReqQuestList, std::bind(&CUserPackage::GetQuestList, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqQuestInfo, std::bind(&CUserPackage::GetQuestInfo, this, std::placeholders::_1, std::placeholders::_2));

	//做任务
	cmd->AddCommand(NetMsg::MessageType::ReqDoQuest, std::bind(&CUserPackage::DoQuest, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqAddHero, std::bind(&CUserPackage::GetHero, this, std::placeholders::_1, std::placeholders::_2));

	//接受任务
	cmd->AddCommand(NetMsg::MessageType::ReqAcceptQuest, std::bind(&CUserPackage::AcceptQuest, this, std::placeholders::_1, std::placeholders::_2));

	//提交任务
	cmd->AddCommand(NetMsg::MessageType::ReqSubmitQuest, std::bind(&CUserPackage::SubmitQuest, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqClearHeroCD, std::bind(&CUserPackage::ClearHeroCD, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqRefreshQuest, std::bind(&CUserPackage::RefreshQuest, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqBattle, std::bind(&CUserPackage::ReqBattle, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqBattleEnd, std::bind(&CUserPackage::ReqBattleEnd, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqItem, std::bind(&CUserPackage::ReqItem, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqAddItem, std::bind(&CUserPackage::ReqAddItem, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqHeroUseItem, std::bind(&CUserPackage::ReqHeroUseItem, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqUseItem, std::bind(&CUserPackage::ReqUseItem, this, std::placeholders::_1, std::placeholders::_2));	

	cmd->AddCommand(NetMsg::MessageType::ReqHeroFriendship, std::bind(&CUserPackage::ReqHeroFriendship, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqHeroGeneralSoul, std::bind(&CUserPackage::ReqHeroGeneralSoul, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqGiveHeroGift, std::bind(&CUserPackage::ReqGiveHeroGift, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqShopItem, std::bind(&CUserPackage::ReqShopItem, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqBuyShop, std::bind(&CUserPackage::ReqBuyShop, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqAdvantureInfo, std::bind(&CUserPackage::ReqAdvantureInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqAdvanture, std::bind(&CUserPackage::ReqAdvanture, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqAdvantureEventDone, std::bind(&CUserPackage::ReqAdvantureEventDone, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqEvolveHero, std::bind(&CUserPackage::ReqEvolveHero, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqHeroClass, std::bind(&CUserPackage::ReqHeroClass, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqStudySkill, std::bind(&CUserPackage::ReqStudySkill, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqStudyProperty, std::bind(&CUserPackage::ReqStudyProperty, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqAddMoney, std::bind(&CUserPackage::ReqAddMoney, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqAdvancementRank, std::bind(&CUserPackage::ReqAdvancementRank, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqRankTitle, std::bind(&CUserPackage::ReqRankTitle, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqTitleInfo, std::bind(&CUserPackage::ReqTitleInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqElectionTitle, std::bind(&CUserPackage::ReqElectionTitle, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqLevelUpCamp, std::bind(&CUserPackage::ReqLevelUpCamp, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqCampState, std::bind(&CUserPackage::ReqCampState, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqGetCampOut, std::bind(&CUserPackage::ReqGetCampOut, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqClearCampCD, std::bind(&CUserPackage::ReqClearCampCD, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqCycleBattle, std::bind(&CUserPackage::ReqCycleBattle, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqRefresshCycle, std::bind(&CUserPackage::ReqRefresshCycle, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqSelectCycleDiff, std::bind(&CUserPackage::ReqSelectCycleDiff, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqCycleBattleInfo, std::bind(&CUserPackage::ReqCycleBattleInfo, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqRecruitHero, std::bind(&CUserPackage::ReqRecruitHero, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqRecruitedHero, std::bind(&CUserPackage::ReqRecruitedHero, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqVSBattleBegin, std::bind(&CUserPackage::ReqVSBattleBegin, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqVSBattleReady, std::bind(&CUserPackage::ReqVSBattleReady, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqVSBattleOpt, std::bind(&CUserPackage::ReqVSBattleOpt, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqBioHeroInfo, std::bind(&CUserPackage::ReqBioHeroInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqBioHeroStepDone, std::bind(&CUserPackage::ReqBioHeroStepDone, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqRaffleHero, std::bind(&CUserPackage::ReqRaffleHero, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqRaffleHeroTimes, std::bind(&CUserPackage::ReqRaffleHeroTimes, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqMailInfo, std::bind(&CUserPackage::ReqMailInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqMailDetail, std::bind(&CUserPackage::ReqMailDetail, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqDeleteMail, std::bind(&CUserPackage::ReqDeleteMail, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqGetMailAward, std::bind(&CUserPackage::ReqGetMailAward, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqBanquetInfo, std::bind(&CUserPackage::ReqBanquetInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqBanquetHero, std::bind(&CUserPackage::ReqBanquetHero, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqAddFriend, std::bind(&CUserPackage::ReqAddFriend, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqFriendsInfo, std::bind(&CUserPackage::ReqFriendsInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqRemoveFriend, std::bind(&CUserPackage::ReqRemoveFriend, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqPrepareBattle, std::bind(&CUserPackage::ReqPrepareBattle, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqWorldWar, std::bind(&CUserPackage::ReqWorldWar, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqWorldBattle, std::bind(&CUserPackage::ReqWorldBattle, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqTopList, std::bind(&CUserPackage::ReqTopList, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqWorldWarTopList, std::bind(&CUserPackage::ReqWorldWarTopList, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqUserLearnVsCD, std::bind(&CUserPackage::ReqUserLearnVsCD, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqGeneralSoulExchangeHero, std::bind(&CUserPackage::ReqGeneralSoulExchangeHero, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqTeamInfo, std::bind(&CUserPackage::ReqTeamInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqSetTeam, std::bind(&CUserPackage::ReqSetTeam, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqWriteSkillBook, std::bind(&CUserPackage::ReqWriteSkillBook, this, std::placeholders::_1, std::placeholders::_2));
	
	cmd->AddCommand(NetMsg::MessageType::ReqTeamRunPath, std::bind(&CUserPackage::ReqTeamRunPath, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::SetTeamRunPath, std::bind(&CUserPackage::SetTeamRunPath, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqTeamArrivedTar, std::bind(&CUserPackage::ReqTeamArrivedTar, this, std::placeholders::_1, std::placeholders::_2));

	//
	cmd->AddCommand(NetMsg::MessageType::ReqCityItem, std::bind(&CUserPackage::ReqCityItem, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqCityHero, std::bind(&CUserPackage::ReqCityHero, this, std::placeholders::_1, std::placeholders::_2));
	
	//军团
	cmd->AddCommand(NetMsg::MessageType::ReqClusterInfo, std::bind(&CUserPackage::ReqClusterInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqAllCluster, std::bind(&CUserPackage::ReqAllCluster, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqCreateCluster, std::bind(&CUserPackage::ReqCreateCluster, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqJoinCluster, std::bind(&CUserPackage::ReqJoinCluster, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqExitCluster, std::bind(&CUserPackage::ReqExitCluster, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqClusterDetailInfo, std::bind(&CUserPackage::ReqClusterDetailInfo, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqSetClusterAttr, std::bind(&CUserPackage::ReqSetClusterAttr, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqResourceInfo, std::bind(&CUserPackage::ReqResourceInfo, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqResourceDetailInfo, std::bind(&CUserPackage::ReqResourceDetailInfo, this, std::placeholders::_1, std::placeholders::_2));

	cmd->AddCommand(NetMsg::MessageType::ReqEnterWorldWar, std::bind(&CUserPackage::ReqEnterWorldWar, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqWorldWarStatus, std::bind(&CUserPackage::ReqWorldWarStatus, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqWorldWarMove, std::bind(&CUserPackage::ReqWorldWarMove, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqWorldWarDefence, std::bind(&CUserPackage::ReqWorldWarDefence, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqWorldWarDefenceTarget, std::bind(&CUserPackage::ReqWorldWarDefenceTarget, this, std::placeholders::_1, std::placeholders::_2));
	cmd->AddCommand(NetMsg::MessageType::ReqLeaveWorldWar, std::bind(&CUserPackage::ReqLeaveWorldWar, this, std::placeholders::_1, std::placeholders::_2));

	//cmd->AddCommand(NetMsg::MessageType::, std::bind(&CUserPackage::, this, std::placeholders::_1, std::placeholders::_2));
}


void CUserPackage::RegAccount(UserSession session, SharedMsg msg)
{
	NetMsg::ReqRegister *reqReg = static_cast<NetMsg::ReqRegister*>(msg.get());
	if (reqReg == nullptr)
		return;

	AccountInfo account;
	account.name = reqReg->name();
	account.passwd = reqReg->password();

	NetMsg::RegisterAck *registerAck = new NetMsg::RegisterAck;	
	SharedMsg ack(registerAck);

	registerAck->set_error(0);

	if (!CReadWriteUser::CreateAccount(account))
	{
		registerAck->set_error(ErrCode::HaveUserName);
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::UserLogin(UserSession session, SharedMsg msg)
{
	NetMsg::ReqLogin *reqLogin;
	GET_REQ_MSG(reqLogin, NetMsg::ReqLogin, msg);

	AccountInfo account;
	account.name = reqLogin->name();
	
	NetMsg::LoginAck *loginAck = new NetMsg::LoginAck;
	SharedMsg ack(loginAck);

	CReadWriteUser::ReadAccount(account);
	if (account.passwd != reqLogin->password())
	{
		loginAck->set_error(ErrCode::ErrorPassword);
	}
	else
	{
		std::string randSession;
		GetRandStr(randSession,30);
		randSession += std::to_string(time(nullptr));
		loginAck->set_session(randSession);

		m_userSessions.insert(std::make_pair(randSession, account.id));
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::GetServerList(UserSession session, SharedMsg)
{
	NetMsg::ServerListAck *serverListack = new NetMsg::ServerListAck;
	SharedMsg ack(serverListack);
	
	std::list<NetMsg::ServerInfo> serverList;
	CReadWriteUser::ReadServerList(serverList);

	for(auto i = serverList.begin();i!= serverList.end();i++)
	{
		NetMsg::ServerInfo *server = serverListack->add_info();
		server->CopyFrom(*i);
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::GetRandStr(std::string &str, uint32_t strLen)
{
	static char chars[] = "abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"1234567890";

	str.resize(strLen);

	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, sizeof(chars) - 1);
	
	char *p = (char*)str.c_str();
	for (uint32_t i = 0; i < strLen; ++i) {
		p[i] = chars[index_dist(rng)];
	}
}

void CUserPackage::ThirdPartyLogin(UserSession session, SharedMsg msg)
{
	NetMsg::ReqThirdPartyLogin *reqLogin = static_cast<NetMsg::ReqThirdPartyLogin*>(msg.get());
	if (reqLogin == nullptr)
		return;

	CUser *user = session->GetData<CUser>();
	if (user != nullptr)
		return;

	user = new CUser;
	session->SetData(user);
	user->SetSession(session);

	user->SetChannelId(reqLogin->id());
	
	std::string randSession;
	GetRandStr(randSession,30);
	randSession += std::to_string(time(nullptr));

	NetMsg::LoginAck *loginAck = new NetMsg::LoginAck;
	loginAck->set_session(randSession);

	SharedMsg ack(loginAck);
	m_socket->SendMsg(session, ack);

	AccountInfo account;
	account.id = 0;

	char *p[2] = { nullptr };
	std::string tocken;
	if (!m_rsa->Decode(true, reqLogin->tocken(), tocken))
		return;

	if (CUtility::SplitLine(p, 2, (char*)tocken.c_str(), '&') != 2)
		return;
	
	account.name = (boost::format("%1%_%2%") % user->GetChannelId() % p[1]).str();
	CReadWriteUser::ReadAccount(account);
	if (account.id == 0)
	{
		CReadWriteUser::CreateAccount(account);
	}
	user->SetAccountId(account.id);
	m_userSessions.insert(std::make_pair(randSession, account.id));
	//"uniqueid=&mobileBound=13636614953&nickname=玩家75356918&loginStatus=1&uuid=75356918&sourceid=0&accessToken=&timestamp=1470734867&sign=L2+3LacLQtYTGTWMkuuR+1ZhuARxAAOeeQ3XhSlrjWHb5ub7gstgOYycXvQoX+YTC1dTkbSRavJHFl04lCT8jbU8cSa0TWOoscaZVOYycFjGG4NYPjUIO/Yy78hljgMsBNrpqmYRicylrpUVrV5oCx1E7kTSoECq5TB+LQrYyDM=&emailBound="
}

void CUserPackage::GetOrderId(UserSession session, SharedMsg req)
{
	CUser *user = session->GetData<CUser>();
	if (user == nullptr || user->GetUserId() == 0)
		return;

	NetMsg::ReqOrderId *reqOrderId = static_cast<NetMsg::ReqOrderId*>(req.get());
	if (reqOrderId == nullptr)
		return;

	//orderId:	productId_timestamp
	//ext:	area_userId
	NetMsg::OrderIdAck *ack = new NetMsg::OrderIdAck;
	std::stringstream sstr;
	sstr << reqOrderId->productid() << "_" << CUtility::GetMicroseconds();
	ack->set_orderid(sstr.str());

	std::stringstream sExt;
	sExt << "0_" << user->GetUserId();
	ack->set_ext(sExt.str());

	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::Chat(UserSession session, SharedMsg msg)
{
	NetMsg::ReqChat *chatMsg;
	CUser *user;
	GET_REQ_MSG(chatMsg, NetMsg::ReqChat, msg);
	GET_LOGIN_USER(user, session);

	std::string chatStr = chatMsg->msg();
	if (RepleaseIllegalChar(chatStr))
		chatMsg->set_msg(chatStr);

	switch (chatMsg->chattype())
	{
	case NetMsg::PToPChat:
		PToPChat(user,session, chatMsg);
		break;
	case NetMsg::WorldChat:
		WorldChat(user, chatMsg);
		break;
	case NetMsg::ForceChat:
		ForceChat(user, chatMsg);
		break;
	case NetMsg::FriendChat:
		FriendChat(user, chatMsg);
		break;
	default:
		break;
	}
}

NetMsg::ChatAck * CUserPackage::CreateChatAck(CUser *user,NetMsg::ReqChat * req)
{
	NetMsg::ChatAck *ackChat = new NetMsg::ChatAck;
	ackChat->set_chattype(req->chattype());
	ackChat->set_msg(req->msg());
	ackChat->set_sendtime((uint32_t)time(nullptr));
	ackChat->set_fromuserid(user->GetUserId());
	std::string nick = user->GetNick();
	char *p[3];
	if (3 == CUtility::SplitLine(p, 3, (char*)nick.c_str(), '|'))
	{
		nick.clear();
		nick = p[0];
		nick.append(p[1]);
		ackChat->set_fromnick(nick);
	}
	ackChat->set_fromforce(user->GetForce());

	if (req->chattype() == NetMsg::PToPChat)
	{
		ackChat->set_touserid(req->touserid());
		ackChat->set_tonick(req->tonick());
	}
	return ackChat;
}

void CUserPackage::PToPChat(CUser * user, UserSession &session, NetMsg::ReqChat * chatMsg)
{
	NetMsg::ChatAck *ackChat = CreateChatAck(user,chatMsg);

	CUser *toUser = m_onlineUser->GetUser(chatMsg->touserid());

	SharedMsg ack(ackChat);

	if (toUser == nullptr)
	{
		ackChat->set_error(ErrCode::ToUserIsOffline);
		m_socket->SendMsg(session, ack);
	}
	else
	{
		m_socket->SendMsg(session, ack);
		m_socket->SendMsg(toUser->GetSession(), ack);
	}
}

void CUserPackage::WorldChat(CUser * user, NetMsg::ReqChat * chatMsg)
{
	NetMsg::ChatAck *ackChat = CreateChatAck(user, chatMsg);
	SharedMsg ack(ackChat);

	auto sendChatMsg = [=](CUser *toUser) {
		m_socket->SendMsg(toUser->GetSession(), ack);
	};
	m_onlineUser->ForEachUser(sendChatMsg);
}

void CUserPackage::ForceChat(CUser *user, NetMsg::ReqChat *chatMsg)
{
	NetMsg::ChatAck *ackChat = CreateChatAck(user, chatMsg);
	SharedMsg ack(ackChat);

	auto sendChatMsg = [=](CUser *toUser) {
		if(toUser->GetForce() == user->GetForce())
			m_socket->SendMsg(toUser->GetSession(), ack);
	};
	m_onlineUser->ForEachUser(sendChatMsg);
}

void CUserPackage::FriendChat(CUser *user, NetMsg::ReqChat *chatMsg)
{
	NetMsg::ChatAck *ackChat = CreateChatAck(user, chatMsg);
	SharedMsg ack(ackChat);
	m_socket->SendMsg(user->GetSession(), ack);

	auto &friends = user->GetFriend();
	for(auto i = friends.begin(); i != friends.end(); i++)
	{
		auto toUser = CUtility::GetOnlineUser(i->id);
		if(toUser != nullptr)
			m_socket->SendMsg(toUser->GetSession(), ack);
	}
}

void CUserPackage::GetUserNick(UserSession session, SharedMsg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::UserNickAck *nickAck = new NetMsg::UserNickAck;
	SharedMsg ack(nickAck);

	std::string nick;
	//GetRandStr(nick, 10);
	CReadWriteUser::GetUserNick(nick);
	char *n[3];
	if (3 != CUtility::SplitLine(n, 3, (char*)nick.c_str(), '|'))
		return;
	nickAck->set_firstname(n[0]);
	nickAck->set_secondname(n[1]);
	nickAck->set_lastname(n[2]);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::SetNewGuide(UserSession session, SharedMsg msg)
{
	NetMsg::SetNewGuide *newGuide;
	CUser *user;
	GET_REQ_MSG(newGuide, NetMsg::SetNewGuide, msg);
	GET_LOGIN_USER(user, session);

	uint32_t step = newGuide->step();
	uint32_t addData = newGuide->adddata();

	if (step <= user->GetNewGuide())
		return;

	user->SetNewGuide(step);

	switch (step)
	{
	case NGSelectCity:
	{
		user->SetCurCity(addData);
		user->SetCampCity(addData);
		CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(addData);
		if (city == nullptr)
			return;
		CForce *force = CForceMgr::get_const_instance().GetForce(city->GetForce());
		if (force == nullptr)
			return;
		if(force->GetCanChoose())
			user->SetForce(city->GetForce());
		break;
	}
	case NGSelectHero:
		SelectHero(user, addData);
		break;
	}
}

void CUserPackage::GetHeros(UserSession session, SharedMsg)
{
	CUser *user;
	GET_LOGIN_USER(user, session);

	user->ReadHeros();

	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;

	for (auto i = user->GetHeros().begin(); i != user->GetHeros().end(); i++)
	{
		CUtility::CopyHero(i->second, heroAck);
	}
	SharedMsg ack(heroAck);

	m_socket->SendMsg(session, ack);
}

bool CUserPackage::CalculateRunPos(NetMsg::RunPathAck *runAck, int dt, int speed)
{
	if (dt <= 0)
		return false;

	NetMsg::PathInfo *pathInfo = runAck->mutable_path();

	int runDis = speed * dt;
	uint32_t curWayPointInd = pathInfo->curwaypointind();
	float curPer = pathInfo->curper();

	if (curWayPointInd + 1 < (uint32_t)pathInfo->waypoint_size())
	{
		do
		{
			uint32_t start = pathInfo->waypoint(curWayPointInd);
			uint32_t end = pathInfo->waypoint(curWayPointInd + 1);
			const CWorldPath *path = CWorldPathMgr::get_const_instance().GetPath(start, end);
			if (path == nullptr)
			{
				curWayPointInd = pathInfo->waypoint_size();
				break;
			}
			int len;
			if (curWayPointInd == pathInfo->curwaypointind())
				len = (int)((path->GetLength()*(1 - curPer)));
			else
				len = path->GetLength();
			if (runDis > len)
				runDis -= len;
			else
			{
				curPer = (len - runDis)*1.0f / len;
				pathInfo->set_curwaypointind(curWayPointInd);
				pathInfo->set_curper(curPer);
				break;
			}
			curWayPointInd++;
		} while (curWayPointInd < (uint32_t)pathInfo->waypoint_size() - 1);
	}
	if (curWayPointInd + 1 >= (uint32_t)pathInfo->waypoint_size())
	{
		runAck->set_state(NetMsg::ArrivedTarget);
		return true;
	}
	return false;
}

void CUserPackage::CalculateRunPer(CUser *user, NetMsg::RunPathAck *runAck,int dt)
{
	NetMsg::PathInfo *pathInfo = runAck->mutable_path();
	const static int RUN_SPEED = 60;

	if (CalculateRunPos(runAck, dt, RUN_SPEED))
	{
		if (pathInfo->runtarget() == NetMsg::RunToCity)
		{
			user->SetCurCity((int)pathInfo->targetid());
		}
	}
}

void CUserPackage::GetRunPath(UserSession session, SharedMsg)
{
	CUser *user;
	GET_LOGIN_USER(user, session);
	
	NetMsg::RunPathAck *runAck = new NetMsg::RunPathAck;
	SharedMsg ack(runAck);

	*runAck = user->GetRunPath();

	int dt = (int)(time(nullptr) - user->GetBeginRunTime());
	if (dt < 0)
		return;

	m_socket->SendMsg(session, ack);
}

void CUserPackage::ArrivedCity(UserSession session, SharedMsg msg)
{
	NetMsg::ArrivedCity *arriveCity;
	CUser *user;
	GET_REQ_MSG(arriveCity, NetMsg::ArrivedCity, msg);
	GET_LOGIN_USER(user, session);

	user->SetCurCity(arriveCity->cityid());
}

void CUserPackage::SetHeroFormation(UserSession session, SharedMsg msg)
{
	NetMsg::SetHeroFormation *setFormation;
	CUser *user;
	GET_REQ_MSG(setFormation, NetMsg::SetHeroFormation, msg);
	GET_LOGIN_USER(user, session);

	//for()
	user->SetFormation(setFormation->pos(),setFormation->formation());
}

void CUserPackage::GetHeroFormation(UserSession session, SharedMsg)
{
	CUser *user;
	GET_LOGIN_USER(user, session);

	NetMsg::HeroFormationAck *formationAck = new NetMsg::HeroFormationAck;
	SharedMsg ack(formationAck);

	formationAck->CopyFrom(user->GetHeroFormation());
	m_socket->SendMsg(session, ack);
}

void CopyCity(CWorldCity *inCity, NetMsg::CityInfoAck *outCity)
{
	outCity->set_cityid(inCity->GetId());
	outCity->set_agriculture(inCity->GetAgriculture());
	outCity->set_business(inCity->GetBusiness());
	outCity->set_force(inCity->GetForce());
	outCity->set_governor(inCity->GetGovernor());
	outCity->set_military(inCity->GetMilitary());
	outCity->set_culturedevelopment(inCity->GetCultureDevelopment());
	outCity->set_money(inCity->GetMoney());
	outCity->set_provisions(inCity->GetProvisions());
	outCity->set_moneydelta(inCity->GetMoneyDelta());
	outCity->set_provisionsdelta(inCity->GetProvisionsDelta());
	outCity->set_reserver(inCity->GetReserver());
	outCity->set_infantry(inCity->GetInfantry());
	outCity->set_archer(inCity->GetArcher());
	outCity->set_cavalry(inCity->GetCavalry());
	outCity->set_swordshield(inCity->GetSwordShield());
	outCity->set_crossbowman(inCity->GetCrossbowMan());
	outCity->set_elephant(inCity->GetElephant());
	outCity->set_infantrywounded(inCity->GetInfantryWounded());
	outCity->set_archerwounded(inCity->GetArcherWounded());
	outCity->set_cavalrywounded(inCity->GetCavalryWounded());
	outCity->set_swordshieldwounded(inCity->GetSwordShieldWounded());
	outCity->set_crossbowmanwounded(inCity->GetCrossbowManWounded());
	outCity->set_elephantwounded(inCity->GetElephantWounded());
	outCity->set_population(inCity->GetPopulation());
	outCity->set_populationspeed(inCity->GetPopulationSpeed());
	outCity->set_loyalty(inCity->GetLoyalty());
	outCity->set_durability(inCity->GetDurability());
	//inCity->CopyCityItem(outCity);
	//inCity->CopyCityHero(outCity);
	
	//lolo
	
	auto heros = inCity->GetHeros();
	for (auto i = heros.begin(); i != heros.end(); i++)
	{
		outCity->add_heroclass(*i);
	}
}

void CUserPackage::GetCityInfo(UserSession session, SharedMsg msg)
{
	NetMsg::ReqCityInfo *cityInfo;
	CUser *user;
	GET_REQ_MSG(cityInfo, NetMsg::ReqCityInfo, msg);
	GET_LOGIN_USER(user, session);

	CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityInfo->cityid());
	if (city == nullptr)
		return;

	NetMsg::CityInfoAck *cityAck = new NetMsg::CityInfoAck;
	SharedMsg msgAck(cityAck);
	CopyCity(city, cityAck);
	m_socket->SendMsg(session, msgAck);
}

void CUserPackage::GetQuestList(UserSession session, SharedMsg)
{
	CUser *user;
	GET_LOGIN_USER(user, session);

	if (user->GetQuestNum() <= 0)
	{
		CReadWriteUser::ReadUserQuest(user);
	}
	if (user->GetQuestNum() <= 0)
	{
		CTitle *title = CTitleMgr::get_const_instance().GetTitle(user->GetTitle());
		if (title == nullptr)
			return;
		uint32_t types[] = {
			NetMsg::BattleCateran,
			NetMsg::ConstructMilitary,
			NetMsg::ConstructAgriculture,
			NetMsg::BattleCateran,
			NetMsg::VisitQuest,
		};
		for (uint32_t i = 0; i < user->GetMaxQuestNum(); i++)
		{
			uint32_t type = types[i];// title->RandQuest();
			CUserQuest *quest = CUserQuest::CreateQuest((NetMsg::QuestType)type, user);
			if (quest == nullptr)
			{
				return;
			}
			if (type == NetMsg::ConstructAgriculture)
			{
				NetMsg::QuestConsAgriculture *consAgr = quest->GetQuestData().mutable_data()->mutable_agriculture();
				consAgr->set_cityid(user->GetCampCity());
			}
			else if (type == NetMsg::BattleCateran)
			{
				NetMsg::QuestBattle *battle = quest->GetQuestData().mutable_data()->mutable_battle();
				if(i == 0)
					battle->set_enemyteam(2);
				else 
					battle->set_enemyteam(3);
			}
			user->AddQuest(quest);
		}
	}
	NetMsg::QuestListAck *questList = new NetMsg::QuestListAck;
	SharedMsg msgAck(questList);
	user->GetQuestList(*questList);
	
	m_socket->SendMsg(session, msgAck);
}

void CUserPackage::GetQuestInfo(UserSession session, SharedMsg msg)
{
	NetMsg::ReqQuestInfo *questInfo;
	CUser *user;
	GET_REQ_MSG(questInfo, NetMsg::ReqQuestInfo, msg);
	GET_LOGIN_USER(user, session);

	CUserQuest *quest = user->FindQuest(questInfo->uid());
	if (quest == nullptr)
		return;
	NetMsg::QuestInfoAck *questAck = new NetMsg::QuestInfoAck;
	SharedMsg msgAck(questAck);
	questAck->CopyFrom(quest->GetQuestData());

	m_socket->SendMsg(session, msgAck);
}

static void AddQuestEffect(CUser *user, CUserQuest *quest,uint32_t num,uint32_t & tempHeroClass)
{
	switch (quest->GetQuestData().type())
	{
	case NetMsg::ConstructAgriculture:
	{
		uint32_t cityId = quest->GetCity();
		CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
		if (city != nullptr)
			city->SetAgriculture(city->GetAgriculture() + num);
	}
	break;
	case NetMsg::ConstructBusiness:
	{
		uint32_t cityId = quest->GetCity();
		CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
		if (city != nullptr)
			city->SetBusiness(city->GetBusiness() + num);
	}
	break;
	case NetMsg::ConstructMilitary:
	{
		uint32_t cityId = quest->GetCity();
		CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
		if (city != nullptr)
			city->SetMilitary(city->GetMilitary() + num);
	}
	break;
	case NetMsg::VisitQuest:
	{
		NetMsg::QuestData *data = quest->GetQuestData().mutable_data();
		uint32_t heroClass = data->visit().heroclass();
		tempHeroClass = heroClass;
		user->AddHeroFriendship(heroClass, num);
		user->AddHeroGeneralSoul(heroClass, num);
	}
	break;
	case NetMsg::ConscriptionQuest:
	{
		auto city = CWorldCityMgr::get_const_instance().GetCity(user->GetCampCity());
		if (city != nullptr)
		{
			uint32_t cost = quest->GetCost(user);
			if (city->GetMoney() >= cost && city->GetPopulation() >= num)
			{
				city->SetMoney(city->GetMoney() - cost);
				city->SetPopulation(city->GetPopulation() - num);
				city->SetReserver(city->GetReserver() + num);
			}
		}
	}
	break;
	}
	
}
void CUserPackage::DoQuest(UserSession session, SharedMsg msg)
{
	NetMsg::ReqDoQuest *doQuest;
	CUser *user;
	GET_REQ_MSG(doQuest, NetMsg::ReqDoQuest, msg);
	GET_LOGIN_USER(user, session);

	CUserQuest *userQuest = user->FindQuest(doQuest->questid());
	if (userQuest == nullptr)
		return;
	int state = userQuest->GetQuestData().state();
	if ((state != NetMsg::AcceptQuest) && (state != NetMsg::FinishQuest))
		return;

	CQuest *quest = CQuestMgr::get_const_instance().GetQuest(userQuest->GetQuestId());
	if (quest == nullptr)
		return;

	uint32_t num = 0;
	
	NetMsg::DoQuestAck *questAck = new NetMsg::DoQuestAck;
	SharedMsg msgAck(questAck);
	time_t now = time(nullptr);
	std::list<CHeroInst*> useHeros;
	int clearCdNum = 0;
	for (int i = 0; i < doQuest->heroids_size(); i++)
	{
		CHeroInst *hero = user->GetHero(doQuest->heroids(i));
		if (hero == nullptr)
			continue;
		if (hero->GetCDTime() > now)
		{
			clearCdNum += (hero->GetCDTime() - now)/60;
			//if(no money)
			//{
			//	questAck->set_error(ErrCode::NoMoney);
			//	//questAck->set_error(ErrCode::DoTaskHeroInCD);
			//}
			//else
			/*{
				hero->SetCDTime(0);
			}*/
		}
		uint32_t effectNum = quest->GetEffectNum(hero);
		num += effectNum;
		useHeros.push_back(hero);
		NetMsg::DoQuestAward *doAward = questAck->add_award();
		quest->GetHeroAward(hero, effectNum, doAward);
	}
	if (clearCdNum > 0)
	{
		auto questCom = CQuestMgr::get_const_instance().GetCommonInfo();
		uint32_t money = clearCdNum*questCom.cdCost;
		if(!CUtility::DelPrice(user,questCom.cdCostType,money))
			questAck->set_error(ErrCode::NoMoney);
	}
	if (questAck->error() == 0)
	{
		uint32_t tmpHeroClass = 0;
		AddQuestEffect(user, userQuest, num,tmpHeroClass);
		if(tmpHeroClass != 0)
		{
			SendHeroClassFri(user, tmpHeroClass);
			SendHeroClassGeneralSoul(user,tmpHeroClass);
		}
		auto questCom = CQuestMgr::get_const_instance().GetCommonInfo();
		NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
		SharedMsg ack(heroAck);
		for (auto i = useHeros.begin(); i != useHeros.end(); i++)
		{
			CHeroInst *hero = *i;
			if (hero != nullptr)
			{
				hero->SetCDTime(now + questCom.heroCd);
				CUtility::CopyHero(hero, heroAck);
				m_socket->SendMsg(session, ack);
			}
		}
		NetMsg::QuestInfoAck *questAck = new NetMsg::QuestInfoAck;
		SharedMsg msgQuestAck(questAck);
		userQuest->DoQuest(num);
		questAck->CopyFrom(userQuest->GetQuestData());
		m_socket->SendMsg(session, msgQuestAck);
	}
	questAck->set_effect(num);
	questAck->set_questid(doQuest->questid());
	m_socket->SendMsg(session, msgAck);
}

void CUserPackage::GetHero(UserSession session, SharedMsg msg)
{
	NetMsg::ReqAddHero *addHero;
	CUser *user;
	GET_REQ_MSG(addHero, NetMsg::ReqAddHero, msg);
	GET_LOGIN_USER(user, session);

	NetMsg::AddHeroAck *addHeroAck = new NetMsg::AddHeroAck;
	SharedMsg msgAck(addHeroAck);
	CUtility::AddUserHero(user, addHero->heroid());

	addHeroAck->set_heroid(addHero->heroid());

	m_socket->SendMsg(session, msgAck);

	std::vector<uint32_t> vec;
	for(int i=1;i<12;i++)
	{
		vec.push_back(i);
	}
	uint32_t selId = CUtility::GetRandomOneFromVector(vec);
	CUtility::AddUserResourceField(user, selId);
}

static uint32_t GetEnemyId(int pos, EnemyInfo &info)
{
	pos++;
	std::vector<uint32_t> enemys;

	auto allEnemys = CEnemyMgr::get_const_instance().GetEnemys();

	std::function<bool(int, const EnemyFilterData*)> fun;
	
	switch (pos)
	{
	case 1:
		fun = DesignData::Expressions::sEnemyTeamMember1;
		break;
	case 2:
		fun = DesignData::Expressions::sEnemyTeamMember2;
		break;
	case 3:
		fun = DesignData::Expressions::sEnemyTeamMember3;
		break;
	case 4:
		fun = DesignData::Expressions::sEnemyTeamMember4;
		break;
	case 5:
		fun = DesignData::Expressions::sEnemyTeamMember5;
		break;
	default:
		return 0;
	}
	auto isEnemy = [&](std::unordered_map<uint32_t, CEnemy*>::value_type iter) {
		CEnemy *enemy = iter.second;
		EnemyFilterData enemyData(enemy);
		if (fun(info.conditionPos, &enemyData))
			enemys.push_back(enemy->GetId());
	};
	std::for_each(allEnemys->begin(), allEnemys->end(), isEnemy);

	if (enemys.size() <= 0)
		return 0;
	return CUtility::RandomSel(enemys);
}

void CUserPackage::AcceptQuest(UserSession session, SharedMsg msg)
{
	NetMsg::ReqAcceptQuest *acceptQuest;
	CUser *user;
	GET_REQ_MSG(acceptQuest, NetMsg::ReqAcceptQuest, msg);
	GET_LOGIN_USER(user, session);

	CUserQuest *userQuest = user->FindQuest(acceptQuest->questid());
	if (userQuest == nullptr)
		return;

	if (userQuest->GetQuestData().state() != NetMsg::NotAcceptQuest)
		return;
	
	CTitle *title = CTitleMgr::get_const_instance().GetTitle(user->GetTitle());
	if (title == nullptr)
		return;

	auto quests = user->GetQuests();
	uint32_t questNum = 0;
	for (auto i = quests->begin(); i != quests->end(); i++)
	{
		CUserQuest *q = *i;
		if (q->GetQuestState() == NetMsg::AcceptQuest)
			questNum++;
	}
	NetMsg::AcceptQuestAck *ack = new NetMsg::AcceptQuestAck;
	SharedMsg ackMsg(ack);
	ack->set_questid(acceptQuest->questid());
	if (questNum >= title->GetQuestNum())
	{
		ack->set_error(ErrCode::TooManyRecvQuest);
	}
	else
	{
		userQuest->GetQuestData().set_state(NetMsg::AcceptQuest);
		NetMsg::QuestInfoAck &questInfo = userQuest->GetQuestData();
		if (questInfo.type() == NetMsg::BattleCateran)
		{
			NetMsg::QuestBattle *questBattle = questInfo.mutable_data()->mutable_battle();
			CEnemyTeam *team = CEnemyTeamMgr::get_const_instance().GetTeam(questBattle->enemyteam());
			if (team == nullptr)
				return;
			CUserBattle *userBattle = new CUserBattle;
			userBattle->SetTeamId(team->GetId());
			userBattle->SetAwardId(team->GetAwardId());
			auto enemys = team->GetEnemys();
			
			for (size_t i = 0; i < enemys->size(); i++)
			{
				EnemyInfo &info = enemys->at(i);
				uint32_t enemyId = GetEnemyId(i, info);
				if(enemyId != 0)
					userBattle->AddEnemy(enemyId, user->GetLevel(), info);
			}
			userBattle->SetType(CUserBattle::QuestBattle);
			userBattle->SetTypeData(acceptQuest->questid());
			userBattle->SetId(questBattle->battleid());
			userBattle->GetBattleData().set_battleid(questBattle->battleid());
			user->AddBattle(userBattle);
		}
	}
	userQuest->RefreshAward(user);
	NetMsg::QuestInfoAck *questAck = new NetMsg::QuestInfoAck;
	SharedMsg msgQuestAck(questAck);
	questAck->CopyFrom(userQuest->GetQuestData());
	m_socket->SendMsg(session, msgQuestAck);

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::SubmitQuest(UserSession session, SharedMsg msg)
{
	NetMsg::ReqSubmitQuest *subQuest;
	CUser *user;
	GET_REQ_MSG(subQuest, NetMsg::ReqSubmitQuest, msg);
	GET_LOGIN_USER(user, session);

	CUserQuest *userQuest = user->FindQuest(subQuest->questid());
	if (userQuest == nullptr)
		return;

	if (userQuest->GetQuestData().state() != NetMsg::FinishQuest)
		return;
	
	NetMsg::SubmitQuestAck *ack = new NetMsg::SubmitQuestAck;
	SharedMsg ackMsg(ack);
	ack->set_questid(subQuest->questid());
	if (userQuest->GetCurTarNum() < userQuest->GetTargetNum())
	{
		ack->set_error(ErrCode::TargetUnfinished);
	}
	else
	{
		//send city award
		uint32_t cityType = 0;
		//uint32_t cityId = 0;
		userQuest->GetCityAward(user, cityType);
		switch(cityType)
		{
			case  NetMsg::FindItem:
				SendCityItem(user,user->GetCampCity());
			break;
			case  NetMsg::FindHero:
				SendCityHero(user, user->GetCampCity());
			break;
			default:
			break;
		}
		userQuest->RefreshAward(user);
		userQuest->GetQuestData().set_state(NetMsg::SubmitQuest);
		const NetMsg::Award *netAward = userQuest->GetAward();
		CUtility::AddUserAwards(user, netAward);
	}
	NetMsg::QuestInfoAck *questAck = new NetMsg::QuestInfoAck;
	SharedMsg msgQuestAck(questAck);
	questAck->CopyFrom(userQuest->GetQuestData());
	m_socket->SendMsg(session, msgQuestAck);
	m_socket->SendMsg(session, ackMsg);

	auto quests = user->GetQuests();
	CTitle *title = CTitleMgr::get_const_instance().GetTitle(user->GetTitle());
	if (title == nullptr)
		return;

	for (auto i = quests->begin(); i != quests->end(); i++)
	{
		CUserQuest *q = *i;
		if (q->GetQuestState() != NetMsg::SubmitQuest)
		{
			return;
		}
	}

	for (auto i = quests->begin(); i != quests->end(); i++)
	{
		CUserQuest *q = *i;
		uint32_t type = title->RandQuest();
		q->Init((NetMsg::QuestType)type, user);
	}
	NetMsg::QuestListAck *questList = new NetMsg::QuestListAck;
	SharedMsg questListAck(questList);
	user->GetQuestList(*questList);
	m_socket->SendMsg(session, questListAck);
}

void CUserPackage::ClearHeroCD(UserSession session, SharedMsg msg)
{
	NetMsg::ReqClearHeroCD *clearCd;
	CUser *user;
	GET_REQ_MSG(clearCd, NetMsg::ReqClearHeroCD, msg);
	GET_LOGIN_USER(user, session);

	CHeroInst *hero = user->GetHero(clearCd->heroid());
	if (hero == nullptr)
		return;

	NetMsg::ClearHeroCDAck *clearAck = new NetMsg::ClearHeroCDAck;

	auto questCom = CQuestMgr::get_const_instance().GetCommonInfo();

	int num = (hero->GetCDTime() - time(nullptr))/60;
	if (num > 0)
	{
		if (!CUtility::DelPrice(user, questCom.cdCostType, num*questCom.cdCost)) {
			clearAck->set_error(ErrCode::NoMoney);
		}
		else {
			hero->SetCDTime(0);
			clearAck->set_heroid(clearCd->heroid());
		}
	}
	SharedMsg ackMsg(clearAck);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::RefreshQuest(UserSession session, SharedMsg)
{
	CUser *user;
	GET_LOGIN_USER(user, session);

	auto quests = user->GetQuests();
	
	CTitle *title = CTitleMgr::get_const_instance().GetTitle(user->GetTitle());
	if (title == nullptr)
		return;

	auto questCom = CQuestMgr::get_const_instance().GetCommonInfo();
	if (!CUtility::DelPrice(user, questCom.refreshCostType, questCom.refreshCost)) {
		NetMsg::RefreshQuestAck *refAck = new NetMsg::RefreshQuestAck;
		refAck->set_success(false);
		SharedMsg ackMsg(refAck);
		m_socket->SendMsg(session, ackMsg);
	} else {
		for (auto i = quests->begin(); i != quests->end(); i++)
		{
			CUserQuest *q = *i;
			if ((q->GetQuestState() == NetMsg::SubmitQuest) || (q->GetQuestState() == NetMsg::NotAcceptQuest))
			{
				uint32_t type = title->RandQuest();
				if (!q->Init((NetMsg::QuestType)type, user))
				{
					int i = 0;
				}
			}
		}

		NetMsg::RefreshQuestAck *refAck = new NetMsg::RefreshQuestAck;
		refAck->set_success(true);
		SharedMsg ackMsg(refAck);
		m_socket->SendMsg(session, ackMsg);

		NetMsg::QuestListAck *questList = new NetMsg::QuestListAck;
		SharedMsg questListAck(questList);
		user->GetQuestList(*questList);
		m_socket->SendMsg(session, questListAck);
	}
}

void CUserPackage::ReqBattle(UserSession session, SharedMsg msg)
{
	NetMsg::ReqBattle *recBattle;
	CUser *user;
	GET_REQ_MSG(recBattle, NetMsg::ReqBattle, msg);
	GET_LOGIN_USER(user, session);

	user->ReadBattle();

	CUserBattle *battle = user->GetBattle(recBattle->battleid());
	if (battle == nullptr)
	{
		return;
	}

	user->SetBattleId(recBattle->battleid());
	NetMsg::BattleAck *battleAck = new NetMsg::BattleAck;
	SharedMsg ack(battleAck);
	auto &battleData = battle->GetBattleData();
	battleAck->CopyFrom(battleData);
	auto team = CEnemyTeamMgr::get_const_instance().GetTeam(battleData.teamid());
	battleAck->set_error(0);
	if (team != nullptr && team->GetCost() != 0)
	{
		if (team->GetCost() > user->GetStamina())
			battleAck->set_error(ErrCode::NoProvisions);
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqBattleEnd(UserSession session, SharedMsg msg)
{
	NetMsg::ReqBattleEnd *battleEnd;
	CUser *user;
	GET_REQ_MSG(battleEnd, NetMsg::ReqBattleEnd, msg);
	GET_LOGIN_USER(user, session);

	CUserBattle *battle = user->GetBattle(battleEnd->battleid());
	if (battle == nullptr)
		return;

	auto &battleData = battle->GetBattleData();
	auto team = CEnemyTeamMgr::get_const_instance().GetTeam(battleData.teamid());
	if (team != nullptr && team->GetCost() != 0)
	{
		if (!CUtility::DelPrice(user, CUtility::PROVISIONS, team->GetCost()))
			return;
	}

	NetMsg::BattleEndAck *ackBattle = new NetMsg::BattleEndAck;
	ackBattle->set_battleid(battleEnd->battleid());
	SharedMsg ackBat(ackBattle);

	if (!battleEnd->victory())
	{
		ackBattle->set_victory(false);
		m_socket->SendMsg(session, ackBat);

		if (battle->GetType() == CUserBattle::WorldWarBattle)
		{
			CWorldWar::get_mutable_instance().SubmitExploit(battle->GetTypeData(), user, 10);
		}

		auto &battleAck = battle->GetBattleData();
		if (battleEnd->enemys_size() != battleAck.enemys_size())
			return;
		for (int i = 0; i < battleEnd->enemys_size(); i++)
		{
			battleAck.mutable_enemys(i)->set_herohp(battleEnd->enemys(i).herohp());
			battleAck.mutable_enemys(i)->set_soldierhp(battleEnd->enemys(i).soldierhp());
		}

		return;
	}
	
	std::vector<uint32_t> awardIds;
	awardIds.push_back(battle->GetAwardId());
	
	battle->SetDelBattle(true);
	int type = battle->GetType();
	if (type == CUserBattle::QuestBattle)
	{
		CUserQuest *quest = user->FindQuest(battle->GetTypeData());
		if (quest == nullptr)
			return;
		quest->GetQuestData().set_state(NetMsg::FinishQuest);
		NetMsg::QuestInfoAck *questAck = new NetMsg::QuestInfoAck;
		SharedMsg msgAck(questAck);
		questAck->CopyFrom(quest->GetQuestData());
		m_socket->SendMsg(session, msgAck);
	}
	else if (type == CUserBattle::CycBattle)
	{
		uint32_t awardid = CycBattleEnd(user);
		if (awardid != 0)
			awardIds.push_back(awardid);
	}
	else if (type == CUserBattle::AdvEventBattle)
	{
		EndAdvEventBattle(user,battle);
	}
	else if (type == CUserBattle::BiographyBattle)
	{

	}
	else if (type == CUserBattle::WorldWarBattle)
	{
		CWorldWar::get_mutable_instance().SubmitExploit(battle->GetTypeData(),user, 20);
	}
	CUtility::AddAwards(user, awardIds);

	ackBattle->set_victory(true);
	m_socket->SendMsg(session, ackBat);

	user->SetBattleId(0);
}

void CUserPackage::SetRunPath(UserSession session, SharedMsg msg)
{
	NetMsg::SetRunPath *runPath;
	CUser *user;
	GET_REQ_MSG(runPath, NetMsg::SetRunPath, msg);
	GET_LOGIN_USER(user, session);

	if (runPath->state() == NetMsg::Runing)
	{
		user->SetCurCity(-1);
		user->SetBeginRunTime(time(nullptr));
	}

	NetMsg::RunPathAck &userPath = user->GetRunPath();

	NetMsg::PathInfo *pathInfo = userPath.mutable_path();
	*pathInfo = runPath->path();
	userPath.set_state(runPath->state());
}

void CUserPackage::SelectHero(CUser *user, uint32_t selPos)
{
	CForce *force = CForceMgr::get_const_instance().GetForce(user->GetForce());
	if (force == nullptr)
		return;

	std::vector<uint32_t> heros = force->GetChooseHeros();
	if (selPos >= heros.size())
		return;

	auto &addData = user->GetAddData();
	addData.set_selhero(heros[selPos]);
	auto hero = CUtility::AddUserHero(user, heros[selPos]);
	
	if (hero == nullptr || user->GetHeros().size() > 1)
		return;

	NetMsg::HeroFormationAck &userHeroFor = user->GetHeroFormation();
	if (userHeroFor.formation_size() <= 0)
		user->InitFormation();

	userHeroFor.mutable_formation(0)->set_heroid(0,hero->GetId());
	NetMsg::HeroFormationAck *formationAck = new NetMsg::HeroFormationAck;
	SharedMsg msgAck(formationAck);

	formationAck->CopyFrom(user->GetHeroFormation());
	m_socket->SendMsg(user->GetSession(), msgAck);
}

bool CUserPackage::RepleaseIllegalChar(std::string & str)
{
	bool have = false;
	char *p;
	for (auto i = m_badWords.begin(); i != m_badWords.end(); i++)
	{
		std::string &word = *i;
		p = (char*)strstr(str.c_str(), word.c_str());
		if (p != nullptr)
		{
			memset(p, '*', word.size());
			have = true;
		}
	}
	return have;
}

bool CUserPackage::HaveIllegalChar(std::string & str)
{
	for (auto i = m_badWords.begin(); i != m_badWords.end(); i++)
	{
		std::string &word = *i;
		if (nullptr != strstr(str.c_str(), word.c_str()))
			return true;
	}
	return false;
}


void CUserPackage::AddOnlineUser(CUser *user)
{
	CUser *oldUser = m_onlineUser->GetUser(user->GetUserId());
	if (oldUser != nullptr)
	{
		//CReadWriteUser::WriteUser(*oldUser);
		UserSession &oldSess = oldUser->GetSession();
		oldSess->close();
		m_onlineUser->DelUser(user->GetUserId());
		//user->SetUserId(0);
		oldSess = user->GetSession();
		//user->CopyUser(*oldUser);
		*user = *oldUser;
		oldUser->Clear();
		oldUser->SetUserId(0);
		user->SetSession(oldSess);
	}
	
	user->SetLoginTime(time(nullptr));
	m_onlineUser->AddUser(user->GetUserId(), user);
}

void CUserPackage::CopyUser(CUser *user, NetMsg::PlayerData *playerInfo)
{
	playerInfo->set_id(user->GetUserId());
	playerInfo->set_level(user->GetLevel());

	char *n[3];
	std::string nick = user->GetNick();
	if (3 == CUtility::SplitLine(n, 3, (char*)nick.c_str(), '|'))
	{
		playerInfo->set_firstname(n[0]);
		playerInfo->set_secondname(n[1]);
		playerInfo->set_lastname(n[2]);
	}
	
	playerInfo->set_stamina(user->GetStamina());
	playerInfo->set_viplevel(user->GetVipLevel());
	playerInfo->set_coin(user->GetCoin());
	playerInfo->set_exp(user->GetExp());
	playerInfo->set_gold(user->GetGold());
	playerInfo->set_exploit(user->GetExploit());
	playerInfo->set_newguide(user->GetNewGuide());
	playerInfo->set_cityid(user->GetCurCity());
	playerInfo->set_campcityid(user->GetCampCity());
	playerInfo->set_force(user->GetForce());
	playerInfo->set_title(user->GetTitle());
	playerInfo->set_contribution(user->GetContribution());
	playerInfo->set_reputation(user->GetReputation());
	playerInfo->set_battleid(user->GetBattleId());
	playerInfo->set_rank(user->GetRank());
	playerInfo->set_sex(user->GetSex());
	playerInfo->set_icon(user->GetIcon());
	auto &addData = user->GetAddData();
	playerInfo->set_selecthero(addData.selhero());
}

void CUserPackage::EnterGame(UserSession session, SharedMsg msg)
{
	NetMsg::ReqEnterGame *reqEnter;
	GET_REQ_MSG(reqEnter, NetMsg::ReqEnterGame, msg);
	
	auto iter = m_userSessions.find(reqEnter->session());
	if (iter == m_userSessions.end())
		return;
	
	CUser *user = new CUser;
	session->SetData(user);
	user->SetSession(session);
	user->SetAccountId(iter->second);
	CReadWriteUser::ReadUser(*user);

	NetMsg::EnterGameAck *ack = new NetMsg::EnterGameAck;
	
	if (user->GetUserId() == 0)
	{
		ack->set_error(ErrCode::NoUserInfo);
	}
	else
	{
		if (user->GetCurCity() < 0)
		{
			NetMsg::RunPathAck &runPathAck = user->GetRunPath();
			if(runPathAck.state() == NetMsg::Runing)
				CalculateRunPer(user, &runPathAck, (int)(time(nullptr) - user->GetBeginRunTime()));
		}

		NetMsg::PlayerData *playerInfo = ack->mutable_player();
		AddOnlineUser(user);
		CopyUser(user, playerInfo);
	}
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::CreateUser(UserSession session, SharedMsg msg)
{
	NetMsg::ReqCreateUser *creatUser;
	CUser *user;
	GET_REQ_MSG(creatUser, NetMsg::ReqCreateUser, msg);
	GET_USER(user, session);

	if (user->GetAccountId() == 0 || user->GetUserId() != 0)
		return;
	std::string userNick = creatUser->firstname() + "|" + creatUser->secondname()
		+ "|" + creatUser->lastname();
	user->SetNick(userNick);
	user->SetNewGuide(creatUser->newguideid());
	user->SetSex(creatUser->sex());
	user->SetIcon(creatUser->icon());

	NetMsg::CreateUserAck *ack = new NetMsg::CreateUserAck;
	SharedMsg ackMsg(ack);
	std::string nick = creatUser->firstname() + creatUser->lastname() + creatUser->secondname();
	if (HaveIllegalChar(nick))
	{
		ack->set_error(ErrCode::IllegalName);
	}
	else if (!CReadWriteUser::CreateUser(*user))
	{
		ack->set_error(ErrCode::HaveUserName);
	}
	else
	{
		CReadWriteUser::UseNick(userNick);
		NetMsg::PlayerData *player = ack->mutable_player();
		AddOnlineUser(user);
		CopyUser(user, player);

		auto mailInfo = CMailInfo::get_const_instance().GetInfo(2);
		if (mailInfo != nullptr)
		{
			char mailTitle[256];
			char mailContent[2048];
			std::string nick = CUtility::GetNick(user->GetNick().c_str());
			snprintf(mailTitle, sizeof(mailTitle), mailInfo->title.c_str(), nick.c_str());
			snprintf(mailContent, sizeof(mailContent), mailInfo->content.c_str(), nick.c_str());
			CUtility::SendSysMail(user, mailInfo->sender.c_str(), mailTitle, mailContent, mailInfo->awardId, NetMsg::MailType::SystemMail);
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::GetClientVersion(UserSession session, SharedMsg)
{
	NetMsg::VersionInfoAck *verAck = new NetMsg::VersionInfoAck;
	verAck->set_clientversion(2);
	SharedMsg msg(verAck);
	m_socket->SendMsg(session,msg);
}

void CUserPackage::GetNamePassword(UserSession, SharedMsg)
{
}

void CUserPackage::Test(UserSession session, SharedMsg reqMsg)
{
	NetMsg::TestAck *verAck = new NetMsg::TestAck;
	verAck->set_a("test_a");
	verAck->set_b(1.0254554f);
	verAck->set_c(-123);
	verAck->set_d(0xffffffff);
	verAck->set_e(false);
	verAck->add_f("f1");
	verAck->add_f("f2");
	verAck->add_f("f3");
	verAck->add_f("f6");
	/*NetMsg::PlayerData *player = verAck->mutable_g();
	player->set_nick("nick");
	player->set_id(1);
	player->set_level(10);
	player->set_exp(0xfff89);
	player->set_stamina(1234567);*/

	SharedMsg msg(verAck);
	m_socket->SendMsg(session, msg);
}

void CUserPackage::ReqItem(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	user->ReadItem();

	NetMsg::ItemAck *itemAck = new NetMsg::ItemAck;
	SharedMsg ack(itemAck);
	
	auto items = user->GetItems();
	CItemInst *item;
	for (auto i = items->begin(); i != items->end(); i++)
	{
		item = i->second;
		NetMsg::ItemInfo *info = itemAck->add_items();
		info->set_id(item->GetItemId());
		info->set_uid(item->GetId());
		info->set_num(item->GetNum());
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqAddItem(UserSession session, SharedMsg msg)
{
	NetMsg::ReqAddItem *addItem;
	CUser *user;
	GET_REQ_MSG(addItem, NetMsg::ReqAddItem, msg);
	GET_USER(user, session);

	NetMsg::AddItemAck *addAck = new NetMsg::AddItemAck;
	addAck->set_itemid(addItem->itemid());

	if(!CUtility::AddUserItem(user,addItem->itemid(),addItem->num()))
		addAck->set_error(ErrCode::NoThisItem);
	SharedMsg addAckMsg(addAck);
	m_socket->SendMsg(session, addAckMsg);
}

void CUserPackage::ReqHeroUseItem(UserSession session, SharedMsg msg)
{
	NetMsg::ReqHeroUseItem *useItem;
	CUser *user;
	GET_REQ_MSG(useItem, NetMsg::ReqHeroUseItem, msg);
	GET_USER(user, session);

	CHeroInst *hero = user->GetHero(useItem->heroid());
	CItemInst *item = user->GetItem(useItem->itemid());
	
	NetMsg::HeroUserItemAck *useItemAck = new NetMsg::HeroUserItemAck;
	SharedMsg ack(useItemAck);
	useItemAck->set_heroid(useItem->heroid());

	if (item == nullptr || hero == nullptr)
	{
		useItemAck->set_error(ErrCode::NoThisItem);
		m_socket->SendMsg(session, ack);
		return;
	}
	CItemTmpl *itemTmpl = item->GetItemTmpl();
	if (itemTmpl == nullptr)
		return;
	if (item->GetNum() < useItem->num() || useItem->num() == 0 || item->GetNum() == 0)
	{
		useItemAck->set_error(ErrCode::ItemNotEnough);
		m_socket->SendMsg(session, ack);
		return;
	}

	uint32_t type = itemTmpl->GetType();
	switch (type)
	{
	case CItemTmpl::ITEM_BOOK_EXP:
		UseHeroExp(user,hero, item,useItem->num());
		break;
	case CItemTmpl::ITEM_BOOK_SKILL:
		UseHeroSkill(user,hero, item);
		break;
	default:
		return;
	}
	
	DelItem(user, item, useItem->num());

	m_socket->SendMsg(session, ack);
}


void CUserPackage::ReqGiveHeroGift(UserSession session, SharedMsg msg)
{
	NetMsg::ReqGiveHeroGift *useItem;
	CUser *user;
	GET_REQ_MSG(useItem, NetMsg::ReqGiveHeroGift, msg);
	GET_USER(user, session);

	CItemInst *item = user->GetItem(useItem->itemid());

	NetMsg::GiveHeroGiftAck *useItemAck = new NetMsg::GiveHeroGiftAck;
	SharedMsg ack(useItemAck);
	useItemAck->set_itemid(useItem->itemid());

	if (item == nullptr)
	{
		useItemAck->set_error(ErrCode::NoThisItem);
		m_socket->SendMsg(session, ack);
		return;
	}
	CItemTmpl *itemTmpl = item->GetItemTmpl();
	if (itemTmpl == nullptr)
		return;
	if (item->GetNum() < useItem->itemnum() || useItem->itemnum() == 0 || item->GetNum() == 0)
	{
		useItemAck->set_error(ErrCode::ItemNotEnough);
		m_socket->SendMsg(session, ack);
		return;
	}

	bool useSuccess = false;
	uint32_t type = itemTmpl->GetType();
	switch (type)
	{
	case CItemTmpl::ITEM_GIFT:
		useSuccess = UseHeroGift(user,useItem->heroclass() , item, useItem->itemnum());
		break;
	default:
		return;
	}
	if (useSuccess)
	{
		DelItem(user, item, useItem->itemnum());
	}
	else
	{
		useItemAck->set_error(ErrCode::NoThisHero);
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqShopItem(UserSession session, SharedMsg msg)
{
	NetMsg::ReqShopItem *shopItem;
	CUser *user;
	GET_REQ_MSG(shopItem, NetMsg::ReqShopItem, msg);
	GET_USER(user, session);

	NetMsg::ShopItemAck &itemAck = user->GetShopItem();
	if (itemAck.items_size() <= 0)
	{
		CShopTmpl *shop = CShopMgr::get_const_instance().GetShop(shopItem->shopid());
		if (shop == nullptr)
			return;
		shop->GetShopItemAck(&itemAck);
	}
	NetMsg::ShopItemAck *shopItemAck = new NetMsg::ShopItemAck;
	shopItemAck->CopyFrom(itemAck);
	SharedMsg ack(shopItemAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqBuyShop(UserSession session, SharedMsg msg)
{
	NetMsg::ReqBuyShop *buyShop;
	CUser *user;
	GET_REQ_MSG(buyShop, NetMsg::ReqBuyShop, msg);
	GET_USER(user, session);

	NetMsg::ShopItemAck &itemAck = user->GetShopItem();
	if (buyShop->shoppos() >= itemAck.items_size())
		return;

	NetMsg::ShopItem *item = itemAck.mutable_items(buyShop->shoppos());

    NetMsg::BuyShopAck *buyAck = new NetMsg::BuyShopAck;

    if (!CUtility::DelPrice(user, item->currencytype(), item->price())) {
        buyAck->set_error(ErrCode::NoMoney);
    } else {
        CUtility::AddUserItem(user, item->itemid(),item->itemnum());
        buyAck->set_shopid(buyShop->shopid());
        buyAck->set_shoppos(buyShop->shoppos());
    }
	SharedMsg ack(buyAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::CalculateAdvInfo(CUser *user,NetMsg::AdvantureInfoAck &advInfo)
{
	const int maxTimes = 30;
	if (advInfo.timesmax() == 0)
		advInfo.set_timesmax(maxTimes);

	int now = time(nullptr);
	int dtTime = now - advInfo.updatetime() + CAdvantureConf::get_const_instance().GetRecoverTime() - advInfo.nexttime();
	
	int times = dtTime / CAdvantureConf::get_const_instance().GetRecoverTime();
	advInfo.set_updatetime(now);

	times += advInfo.times();
	if (times > advInfo.timesmax())
		times = advInfo.timesmax();
	int leftTime = dtTime % CAdvantureConf::get_const_instance().GetRecoverTime();

	if (advInfo.times() == advInfo.timesmax())
		times = advInfo.timesmax();

	advInfo.set_nexttime(CAdvantureConf::get_const_instance().GetRecoverTime() - leftTime);
	advInfo.set_times(times);

	::google::protobuf::RepeatedPtrField< ::NetMsg::AdvantureEvent > *events = advInfo.mutable_events();

	auto i = events->begin();
	while(i != events->end())
	{
		auto &event = *i;
		int eventLeftTime = (int)event.createtime();
		eventLeftTime = now - eventLeftTime;
		eventLeftTime = event.time() - eventLeftTime;
		event.set_createtime(now);
		if (eventLeftTime < 0)
		{
			eventLeftTime = 0;
			event.set_time(eventLeftTime);
			auto ev = CAdvantureMgr::get_const_instance().GetAdvEvent(event.type());
			if (ev != nullptr && ev->GetType() == CAdvantureEvent::CONTINUOUS_BATTLE)
			{
				SendAdvEventState(user, i->id(), NetMsg::AdvantureEventStatus::Invalid);
				i = events->erase(i);
			}
			else
			{
				SendAdvEventState(user, i->id(), NetMsg::AdvantureEventStatus::Done);
				event.set_status(NetMsg::AdvantureEventStatus::Done);
				i++;
			}
		}
		else
		{
			event.set_time(eventLeftTime);
			i++;
		}
	}
}

void CUserPackage::ReqAdvantureInfo(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::AdvantureInfoAck &advInfo = user->GetAdvInfoAck();
	CalculateAdvInfo(user,advInfo);

	NetMsg::AdvantureInfoAck *info = new NetMsg::AdvantureInfoAck;
	SharedMsg ack(info);
	info->CopyFrom(advInfo);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqAdvanture(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::AdvantureInfoAck &advInfo = user->GetAdvInfoAck();
	CalculateAdvInfo(user,advInfo);
	if (advInfo.times() > 0)
		advInfo.set_times(advInfo.times() - 1);
	else
		return;

	advInfo.set_dotimes(advInfo.dotimes() + 1);

	NetMsg::AdvantureAck *advAck = new NetMsg::AdvantureAck;
	advAck->set_times(advInfo.times());
	advAck->set_nexttime(advInfo.nexttime());

	AdEventData data;
	bool haveEvent = CAdvantureMgr::get_const_instance().GetEvent(user, data);
	advAck->set_crit(data.crit);
	int exp = CAdvantureConf::get_const_instance().GetAwardExp(user)*data.crit;
	int glod = CAdvantureConf::get_const_instance().GetAwardGold(user)*data.crit;
	advAck->set_exp(exp);
	advAck->set_gold(glod);
	CUtility::AddCoin(user,glod);
	CUtility::AddExp(user, exp);

	if (haveEvent)
	{
		NetMsg::AdvantureEvent *event = advAck->mutable_event();
		event->set_id(user->GetAdvEventId());
		event->set_type(data.event->GetId());
		event->set_time(data.delay);
		if(data.delay <= 0)
			event->set_status(NetMsg::AdvantureEventStatus::Done);
				
		auto ev = CAdvantureMgr::get_const_instance().GetAdvEvent(event->type());
		if (ev != nullptr && ev->GetType() == CAdvantureEvent::CONTINUOUS_BATTLE)
		{
			AddAdvEventBattle(user, *event, data);
		}

		NetMsg::Award *award = event->mutable_award();
		CUtility::GetAward(data.awardId, user, award);
		NetMsg::AdvantureEvent *userEvent = advInfo.add_events();
		userEvent->CopyFrom(*event);
		userEvent->set_createtime(time(nullptr));
	}
	SharedMsg ack(advAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::AddAdvEventBattle(CUser *user,NetMsg::AdvantureEvent &event, AdEventData &data)
{
	if (data.event == nullptr)
		return;

	CEnemyTeam *team = CEnemyTeamMgr::get_const_instance().GetTeam(data.event->GetEnemyTeam());
	if (team == nullptr)
		return;

	NetMsg::AdvantureContinuousBattle *b = event.mutable_battle();
	uint32_t battleId = CUserBattle::GenerateUID();
	b->set_battleid(battleId);

	std::vector<uint32_t> paths;
	CWorldPathMgr::get_mutable_instance().GetPaths(paths);
	if (paths.size() <= 0)
		return;
	uint32_t path = CUtility::RandomSel(paths);
	b->set_enemypath(path);
	float r = CUtility::RandomInt(1, 100) / 100.0f;
	b->set_percent(r);
	b->set_enemyteam(team->GetId());

	CUserBattle *userBattle = new CUserBattle;
	userBattle->SetTeamId(team->GetId());
	userBattle->SetAwardId(team->GetAwardId());
	auto enemys = team->GetEnemys();

	for (size_t i = 0; i < enemys->size(); i++)
	{
		EnemyInfo &info = enemys->at(i);
		uint32_t enemyId = GetEnemyId(i, info);
		if (enemyId != 0)
			userBattle->AddEnemy(enemyId, user->GetLevel(), info);
	}

	userBattle->SetType(CUserBattle::AdvEventBattle);
	userBattle->SetTypeData(event.id());
	userBattle->SetId(battleId);
	userBattle->GetBattleData().set_battleid(battleId);
	userBattle->SetAwardId(data.awardId);
	user->AddBattle(userBattle);
}

void CUserPackage::EndAdvEventBattle(CUser *user,CUserBattle * battle)
{
	NetMsg::AdvantureInfoAck &advInfo = user->GetAdvInfoAck();
	::google::protobuf::RepeatedPtrField< ::NetMsg::AdvantureEvent > *events = advInfo.mutable_events();

	uint32_t now = time(nullptr);
	for (auto i = events->begin(); i != events->end(); i++)
	{
		if (i->id() == battle->GetTypeData())
		{
			std::vector<CAward::AwardItem> userAward;
			CUtility::CopyAwards(i->mutable_award(), userAward);
			CUtility::AddUserAwards(user, userAward);
			SendAdvEventState(user,i->id(), NetMsg::AdvantureEventStatus::Done);
			i->set_status(NetMsg::AdvantureEventStatus::Done);
			break;
		}
	}
}

void CUserPackage::SendAdvEventState(CUser *user, uint32_t eventId,uint32_t state)
{
	NetMsg::AdvantureEventStatusChange *donAck = new NetMsg::AdvantureEventStatusChange;
	SharedMsg ack(donAck);
	donAck->set_adveventid(eventId);
	donAck->set_adveventstatus((NetMsg::AdvantureEventStatus)state);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::ReqAdvantureEventDone(UserSession session, SharedMsg msg)
{
	NetMsg::ReqAdvantureEventDone *advEvent;
	CUser *user;
	GET_REQ_MSG(advEvent, NetMsg::ReqAdvantureEventDone, msg);
	GET_USER(user, session);

	NetMsg::AdvantureInfoAck &advInfo = user->GetAdvInfoAck();
	::google::protobuf::RepeatedPtrField< ::NetMsg::AdvantureEvent > *events = advInfo.mutable_events();

	CalculateAdvInfo(user, advInfo);

	uint32_t now = time(nullptr);
	for (auto i = events->begin(); i != events->end(); i++)
	{
		if (i->id() == advEvent->id())
		{
			if (i->status() != NetMsg::AdvantureEventStatus::Done)
			{
				NetMsg::AdvantureEventDoneAck *donAck = new NetMsg::AdvantureEventDoneAck;
				SharedMsg ack(donAck);
				donAck->set_id(advEvent->id());
				donAck->set_error(ErrCode::AdvantureEventNotDone);
				m_socket->SendMsg(session, ack);
				return;
			}

			std::vector<CAward::AwardItem> userAward;
			CUtility::CopyAwards(i->mutable_award(), userAward);
			CUtility::AddUserAwards(user, userAward);
			events->erase(i);
			break;
		}
	}
	NetMsg::AdvantureEventDoneAck *donAck = new NetMsg::AdvantureEventDoneAck;
	SharedMsg ack(donAck);
	donAck->set_id(advEvent->id());
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqHeroClass(UserSession session, SharedMsg msg)
{
	NetMsg::ReqHeroClass *req;
	CUser *user;
	GET_REQ_MSG(req, NetMsg::ReqHeroClass, msg);
	GET_USER(user, session);

	auto hero = CHeroClassMgr::get_const_instance().GetHeroClass(req->classid());
	if (hero == nullptr)
		return;

	NetMsg::HeroClassAck *ack = new NetMsg::HeroClassAck;
	SharedMsg ackMsg(ack);

	CopyHeroClass(hero, ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqStudySkill(UserSession session, SharedMsg msg)
{
	NetMsg::ReqStudySkill *req;
	CUser *user;
	GET_REQ_MSG(req, NetMsg::ReqStudySkill, msg);
	GET_USER(user, session);

	NetMsg::StudySkillAck *ack = new NetMsg::StudySkillAck;
	SharedMsg ackMsg(ack);

	CHeroInst *hero = user->GetHero(req->heroid());
	if (hero == nullptr)
		return;
	time_t now = time(nullptr);

	if(!user->CouldBeLearn(req->heroclass()))
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		m_socket->SendMsg(session, ackMsg);
		SendUserLearnVsCD(user,req->heroclass());
		return;
	}

	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	SharedMsg heroMsg(heroAck);

	int friLevel = user->GetHeroFriendship(req->heroclass()).lv;
	if (hero->GetCDTime() <= now)
	{
		CLearnAndVsCD cd;
		CLearnAndVsCDMgr::get_const_instance().GetCDinfo(friLevel, cd);
		hero->SetCDTime(now + cd.student_learn_cd);

		CUtility::CopyHero(hero, heroAck);
		m_socket->SendMsg(session, heroMsg);
	}
	else
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	user->doAddUserCD(req->heroclass(), friLevel);
	SendUserLearnVsCD(user, req->heroclass());
	
	auto skill = hero->GetSkill(req->skillid());
	if (skill == nullptr)
		return;

	auto heroClass = CHeroClassMgr::get_const_instance().GetHeroClass(req->heroclass());
	if (heroClass == nullptr)
		return;



	if (!user->CouldGetFriendshipByLevel(req->heroclass()))
	{
		ack->set_error(ErrCode::FriendShipLevelError);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	int fri = CLearnSKill::get_const_instance().GetFriendship();
	int generalSoul = CLearnSKill::get_const_instance().GetGeneralSoul();
	int crit = 1;
	int exp = CLearnSKill::get_const_instance().GetSkillExp(hero->GetTolWisdom(),hero->GetTolLuck(), crit);
	exp *= crit;
	skill->AddExp(exp);

	user->AddHeroFriendship(req->heroclass(), fri);
	user->AddHeroGeneralSoul(req->heroclass(),generalSoul);
	SendHeroClassFri(user, req->heroclass());
	SendHeroClassGeneralSoul(user, req->heroclass());

	UpdateHero(user, hero);

	ack->set_heroclass(req->heroclass());
	ack->set_heroid(req->heroid());
	ack->set_skillid(req->skillid());
	ack->set_exp(exp);
	ack->set_crit(crit);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqStudyProperty(UserSession session, SharedMsg msg)
{
	NetMsg::ReqStudyProperty *req;
	CUser *user;
	GET_REQ_MSG(req, NetMsg::ReqStudyProperty, msg);
	GET_USER(user, session);


	NetMsg::StudyPropertyAck *ack = new NetMsg::StudyPropertyAck;
	SharedMsg ackMsg(ack);
	CHeroInst *hero = user->GetHero(req->heroid());
	if (hero == nullptr)
		return;

	if (!user->CouldBeLearn(req->heroclass()))
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		m_socket->SendMsg(session, ackMsg);
		SendUserLearnVsCD(user, req->heroclass());
		return;
	}

	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	SharedMsg heroMsg(heroAck);

	time_t now = time(nullptr);
	
	int friLevel = user->GetHeroFriendship(req->heroclass()).lv;

	if (hero->GetCDTime() <= now)
	{
		CLearnAndVsCD cd;
		CLearnAndVsCDMgr::get_const_instance().GetCDinfo(friLevel, cd);
		hero->SetCDTime(now + cd.student_learn_cd);
		CUtility::CopyHero(hero, heroAck);
		m_socket->SendMsg(session, heroMsg);
	}
	else
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		m_socket->SendMsg(session, ackMsg);
		return;
	}
	
	user->doAddUserCD(req->heroclass(), friLevel);
	SendUserLearnVsCD(user, req->heroclass());

	auto heroClass = CHeroClassMgr::get_const_instance().GetHeroClass(req->heroclass());
	if (heroClass == nullptr)
		return;
	if (!user->CouldGetFriendshipByLevel(req->heroclass()))
	{
		ack->set_error(ErrCode::FriendShipLevelError);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	int diff, property;
	switch (req->type())
	{
	case NetMsg::HeroStrength:
		property = hero->GetTolStrength();
		diff = heroClass->GetStrength().lv - property;
		break;
	case NetMsg::HeroLeadership:
		property = hero->GetTolLeadership();
		diff = heroClass->GetLeadership().lv - property;
		break;
	case NetMsg::HeroWisdom:
		property = hero->GetTolWisdom();
		diff = heroClass->GetWisdom().lv - property;
		break;
	case NetMsg::HeroCharm:
		property = hero->GetTolCharm();
		diff = heroClass->GetCharm().lv - property;
		break;
	case NetMsg::HeroLuck:
		property = hero->GetTolLuck();
		diff = heroClass->GetLuck().lv - property;
		break;
	default:
		return;
	}
	int exp = 0;
	int fri = 0;
	int soul = 0;
	int crit = 1;
	int luck = hero->GetTolLuck();
	CLearnProperty::get_const_instance().GetExpFri(diff, property,luck, exp, fri,crit, soul);
	exp += crit;

	switch (req->type())
	{
	case NetMsg::HeroStrength:
		hero->AddStrength(exp);
		break;
	case NetMsg::HeroLeadership:
		hero->AddLeadership(exp);
		break;
	case NetMsg::HeroWisdom:
		hero->AddWisdom(exp);
		break;
	case NetMsg::HeroCharm:
		hero->AddCharm(exp);
		break;
	case NetMsg::HeroLuck:
		hero->AddLuck(exp);
		break;
	}

	user->AddHeroFriendship(req->heroclass(), fri);
	user->AddHeroGeneralSoul(req->heroclass(),soul);
	SendHeroClassFri(user, req->heroclass());
	SendHeroClassGeneralSoul(user, req->heroclass());

	UpdateHero(user, hero);


	ack->set_heroclass(req->heroclass());
	ack->set_heroid(req->heroid());
	ack->set_type(req->type());
	ack->set_property(exp);
	ack->set_crit(crit);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::CopyHeroClass(CHeroClass *in, NetMsg::HeroClassAck *netHero)
{
	auto hero = netHero->mutable_hero();
	
	auto copyAttr = [](HeroAttr in, NetMsg::HeroAttr *out) {
		out->set_exp(in.exp);
		out->set_lv(in.lv);
	};
	copyAttr(in->GetStar(), hero->mutable_star());
	copyAttr(in->GetCharm(), hero->mutable_charm());
	copyAttr(in->GetLeadership(), hero->mutable_leadership());
	copyAttr(in->GetLuck(), hero->mutable_luck());
	copyAttr(in->GetStrength(), hero->mutable_strength());
	copyAttr(in->GetWisdom(), hero->mutable_wisdom());
	hero->set_classid(in->GetId());
	hero->set_force(in->GetForce());
	
	auto skills = in->GetSkills();
	for (auto i = skills.begin(); i != skills.end(); i++)
	{
		auto skill = hero->add_skills();
		skill->set_exp(i->exp);
		skill->set_skillid(i->skillId);
		skill->set_skilllv(i->lv);
	}
}

void CUserPackage::ReqUseItem(UserSession session, SharedMsg msg)
{
	NetMsg::ReqUseItem *useItem;
	CUser *user;
	GET_REQ_MSG(useItem, NetMsg::ReqUseItem, msg);
	GET_USER(user, session);

	CItemInst *item = user->GetItem(useItem->itemid());

	NetMsg::UseItemAck *useItemAck = new NetMsg::UseItemAck;
	SharedMsg ack(useItemAck);
	useItemAck->set_itemid(useItem->itemid());

	if (item == nullptr)
	{
		useItemAck->set_error(ErrCode::NoThisItem);
		m_socket->SendMsg(session, ack);
		return;
	}
	CItemTmpl *itemTmpl = item->GetItemTmpl();
	if (itemTmpl == nullptr)
		return;
	uint32_t useNum = useItem->num();
	if (item->GetNum() < useNum || useNum == 0 || item->GetNum() == 0)
	{
		useItemAck->set_error(ErrCode::ItemNotEnough);
		m_socket->SendMsg(session, ack);
		return;
	}

	uint32_t type = itemTmpl->GetType();

	switch (type)
	{
	case CItemTmpl::ITEM_BOX:
		CUtility::UseBoxItem(user, item,useNum);
		break;
	default:
		return;
	}
	
	DelItem(user, item, useNum);

	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqHeroFriendship(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::HeroFriendshipAck *friendShipAck = new NetMsg::HeroFriendshipAck;
	SharedMsg ack(friendShipAck);
	user->GetHeroFriendship(friendShipAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::SendHeroClassFri(CUser *user, uint32_t heroClass)
{
	NetMsg::HeroFriendshipAck *friendShipAck = new NetMsg::HeroFriendshipAck;
	SharedMsg ack(friendShipAck);
	CUser::Friendship fri = user->GetHeroFriendship(heroClass);
	auto netFri = friendShipAck->add_herofri();
	netFri->set_heroclass(heroClass);
	netFri->set_friendshipexp(fri.exp);
	netFri->set_friendshiplv(fri.lv);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::SendHeroClassGeneralSoul(CUser *user, uint32_t heroClass)
{
	NetMsg::HeroGeneralSoulAck *generalSoulAck = new NetMsg::HeroGeneralSoulAck;
	SharedMsg ack(generalSoulAck);
	CUser::GeneralSoul soul = user->GetHeroGeneralSoul(heroClass);

	auto netSoul = generalSoulAck->add_herosoul();
	netSoul->set_heroclass(heroClass);
	netSoul->set_num(soul.num);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::SendUserLearnVsCD(CUser *user, uint32_t heroClass)
{
	NetMsg::UserLearnVsCDAck *CDAck = new NetMsg::UserLearnVsCDAck;
	SharedMsg ack(CDAck);
	CUser::UserLearnCD learnCD = user->GetUserLearnCD(heroClass);
	auto cd = CDAck->add_cdinfo();
	cd->set_heroclass(heroClass);

	time_t now = time(nullptr);
	if (learnCD.userCD > now)
		cd->set_userlearnvscd((uint32_t)(learnCD.userCD - now));
	else
		cd->set_userlearnvscd(0);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::DelItem(CUser *user, CItemInst * item, uint32_t useNum)
{
	int leftNum = item->GetNum() - useNum;
	NetMsg::ItemAck *itemAck = new NetMsg::ItemAck;
	SharedMsg itemAckMsg(itemAck);
	NetMsg::ItemInfo *info = itemAck->add_items();
	info->set_id(item->GetItemTmpl()->GetId());
	info->set_uid(item->GetId());
	if (leftNum <= 0)
	{
		user->DelItem(item->GetId());
		info->set_num(0);
	}
	else
	{
		item->SetNum(leftNum);
		info->set_num(leftNum);
	}
	m_socket->SendMsg(user->GetSession(), itemAckMsg);
}

void CUserPackage::AddHero(NetMsg::GiveAward *giveAward, uint64_t heroId)
{
	NetMsg::Award *award = giveAward->mutable_award();
	for (int i = 0; i < award->items_size(); i++)
	{
		NetMsg::AwardItem *item = award->mutable_items(i);
		if (CUtility::IsHeroAward(item->type()))
			item->add_heroids(heroId);
	}
}

void CUserPackage::ReqEvolveHero(UserSession session, SharedMsg msg)
{
	NetMsg::ReqEvolveHero *evolveHero;
	CUser *user;
	GET_REQ_MSG(evolveHero, NetMsg::ReqEvolveHero, msg);
	GET_USER(user, session);

	NetMsg::EvolveHeroAck *evlAck = new NetMsg::EvolveHeroAck;
	SharedMsg ack(evlAck);

	evlAck->set_heroid(evolveHero->heroid());
	CEvolve::Cost cost;
	int err = CEvolve::get_const_instance().EvolveHero(user, evolveHero->heroid(), evolveHero->eatheroid(), cost);
	if (err == 1)
		evlAck->set_error(ErrCode::NoMoney);
	else if (err == 2)
		evlAck->set_error(ErrCode::NoMaterial);
	else if(err == 3)
		evlAck->set_error(ErrCode::EvolveError);
	else
	{
		for (size_t i = 0; i < cost.items.size(); i++)
		{
			DelItem(user, cost.items[i], cost.useNums[i]);
		}
		CUtility::AddCoin(user, -cost.coin);
		DelHero(user, cost.useHero);
		UpdateHero(user, cost.hero);
	}
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::UpdateHero(CUser *user, CHeroInst *hero)
{
	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	CUtility::CopyHero(hero, heroAck);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::DelHero(CUser * user, CHeroInst * hero)
{
	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	hero->SetHeroId(0);
	CUtility::CopyHero(hero, heroAck);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);
	user->DelHero(hero->GetId());
}

void CUserPackage::ReqAddMoney(UserSession session, SharedMsg msg)
{
	NetMsg::ReqAddMoney *req;
	CUser *user;
	GET_REQ_MSG(req, NetMsg::ReqAddMoney, msg);
	GET_USER(user, session);

	if (req->type() == 1)
	{
		CUtility::AddCoin(user, req->add());
	}
	else if (req->type() == 2)
	{
		CUtility::AddGold(user, req->add());
	}
	NetMsg::AddMoneyAck *ack = new NetMsg::AddMoneyAck;
	ack->set_type(req->type());
	ack->set_add(req->add());
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(user->GetSession(), ackMsg);
}

void CUserPackage::UseHeroExp(CUser *user, CHeroInst *hero, CItemInst *item, uint32_t itemNum)
{
	uint32_t exp = item->GetItemTmpl()->GetExtVal() * itemNum;
	hero->AddExp(exp);

	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	CUtility::CopyHero(hero, heroAck);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::UseHeroSkill(CUser *user, CHeroInst *hero, CItemInst *item)
{
	CItemTmpl *tmpl = item->GetItemTmpl();
	if (tmpl == nullptr)
		return;
	CSkillInst *pSkill = hero->GetSkill(tmpl->GetExtId());
	if (pSkill != nullptr)
	{
		pSkill->AddExp(tmpl->GetExtVal());
	}
	else
	{
		CSkillInst skill;
		if (!skill.Init(tmpl->GetExtId()))
			return;
		skill.AddExp(tmpl->GetExtVal());
		hero->AddSkill(skill);
	}
	NetMsg::UpdateSkill *update = new NetMsg::UpdateSkill;
	update->set_herouid(hero->GetId());
	for (auto i = hero->GetSkills().begin(); i != hero->GetSkills().end(); i++)
	{
		NetMsg::SkillInfo *info = update->add_skills();
		info->set_uid(i->GetId());
		info->set_skillid(i->GetSkillId());
		info->set_skilllevel(i->GetLevel());
		info->set_skillexp(i->GetExp());
	}

	SharedMsg ack(update);
	m_socket->SendMsg(user->GetSession(), ack);
}

bool CUserPackage::UseHeroGift(CUser * user, uint32_t heroClass, CItemInst * item, uint32_t itemNum)
{
	auto tmpl = CHeroClassMgr::get_const_instance().GetHeroClass(heroClass);
	if (tmpl == nullptr)
		return false;

    float interestVal = 0;
	CItemTmpl *itemTmpl = item->GetItemTmpl();
	tmpl->GetGiftInterest(itemTmpl->GetExtId(), interestVal);
	uint32_t friendShip = item->GetItemTmpl()->GetExtVal() * itemNum * interestVal; //根据喜好度增加值，从heroClass读取
	uint32_t soul = item->GetItemTmpl()->GetExtVal2() * itemNum * interestVal; //表格需要加一列
	//lyle 

	NetMsg::HeroFriendshipAck *heroAck = new NetMsg::HeroFriendshipAck;
	user->AddHeroFriendship(heroClass, friendShip);
	user->AddHeroGeneralSoul(heroClass, soul);
	SendHeroClassGeneralSoul(user,heroClass);
	NetMsg::HeroFriendship *fri = heroAck->add_herofri();
	fri->set_heroclass(heroClass);
	CUser::Friendship fs = user->GetHeroFriendship(heroClass);
	fri->set_friendshipexp(fs.exp);
	fri->set_friendshiplv(fs.lv);
	SharedMsg ack(heroAck);
	m_socket->SendMsg(user->GetSession(), ack);
	return true;
}

void CopyBanquetInfo(CUser *user, NetMsg::BanquetInfoAck *to)
{
	auto &addData = user->GetAddData();
	auto banInfo = addData.mutable_banquetinfos();
	
	time_t now = time(nullptr);

	auto i = banInfo->begin();
	while (i != banInfo->end())
	{
		auto info = i->mutable_info();
		if (i->duratime() > now - i->createtime())
		{
			info->set_lefttime(i->duratime() - (now - i->createtime()));
			auto banHero = to->add_info();
			banHero->CopyFrom(*info);
			i++;
		}
		else
		{
			i = banInfo->erase(i);
		}
	}
}

// add by clx
void CUserPackage::ReqTopList(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(recTopList, NetMsg::ReqTopList, msg);
	DEF_GET_USER(user, session);

	NetMsg::TopListAck * ack = new NetMsg::TopListAck();
	SharedMsg ackMsg(ack);
 	CUserTopList * userTopList = new CUserTopList();
	userTopList->Init(recTopList->type());
	// reqType
	ack->set_type(recTopList->type());
	// RankingList

	userTopList->GetRankingInfo(ack);
    //myRanking
	userTopList->SetAllMyRanking(ack, user->GetUserId());

	m_socket->SendMsg(session,ackMsg);
}


void CUserPackage::ReqAdvancementRank(UserSession session, SharedMsg msg)
{	
	DEF_GET_REQ_MSG(req, NetMsg::ReqAdvancementRank, msg);
	DEF_GET_USER(user, session);

	uint32_t exp = user->GetContribution();
	CRank *rank = CRankMgr::get_const_instance().GetRankByExp(exp);
	if (rank == nullptr)
		return;
	NetMsg::AdvancementRankAck *ack = new NetMsg::AdvancementRankAck;
	SharedMsg ackMsg(ack);
	uint32_t lv = rank->GetLevel();
	ack->set_rank(req->rank());
	if (lv < req->rank())
	{
		ack->set_error(ErrCode::NotEnoughContribution);
	}
	else
	{
		user->SetRank(req->rank());
		CUtility::UpdateUserInfo(user, 1<<NetMsg::UpdatePlayerAck::UpdateRank);

		if (CBanquet::get_mutable_instance().Create(user))
		{
			NetMsg::BanquetInfoAck *banAck = new NetMsg::BanquetInfoAck;
			SharedMsg banAckMsg(banAck);
			CopyBanquetInfo(user, banAck);
			m_socket->SendMsg(session, banAckMsg);
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRankTitle(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRankTitle, msg);
	DEF_GET_USER(user, session);

	CRank *rank = CRankMgr::get_const_instance().GetRankByLevel(req->rank());
	if (rank == nullptr)
		return;

	auto titles = rank->GetTitles();

	NetMsg::RankTitleAck *ack = new NetMsg::RankTitleAck;
	ack->set_rank(req->rank());

	for (auto i = titles->begin(); i != titles->end(); i++)
	{
		NetMsg::RankTitlePlayer *p = ack->add_players();
		p->set_title(*i);
		p->set_playernum(CElectionTitle::get_mutable_instance().GetTitleUserNum(user->GetForce(),*i));
	}
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqTitleInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqTitleInfo, msg);
	DEF_GET_USER(user, session);

	CTitle *title = CTitleMgr::get_const_instance().GetTitle(req->title());
	if (title == nullptr)
		return;

	NetMsg::TitleInfoAck *ack = new NetMsg::TitleInfoAck;

	ack->set_title(req->title());
	ack->set_lefttime(CElectionTitle::get_mutable_instance().GetLeftTime(req->title()));
	ack->set_curplayernum(CElectionTitle::get_mutable_instance().GetElectionUserNum(user->GetForce(), req->title()));

	auto users = CElectionTitle::get_mutable_instance().GetTitleUsers(user->GetForce(), req->title());
	if (users != nullptr)
	{
		for (auto i = users->begin(); i != users->end(); i++)
		{
			NetMsg::TitleInfoPlayer *p = ack->add_players();
			p->set_id(i->userId);
			p->set_name(i->name);
		}
	}
	uint32_t titleId;
	uint32_t data;
	ack->set_meiselection(false);
	if (CElectionTitle::get_mutable_instance().GetElectionUser(user->GetUserId(), titleId, data))
	{
		if (titleId == req->title())
		{
			ack->set_meiselection(true);
			ack->set_electiondata(data);
		}
	}
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqElectionTitle(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqElectionTitle, msg);
	DEF_GET_USER(user, session);

	CTitle *title = CTitleMgr::get_const_instance().GetTitle(req->title());
	if (title == nullptr)
		return;

	CElectionTitle &election = CElectionTitle::get_mutable_instance();
	
	NetMsg::ElectionTitleAck *ack = new NetMsg::ElectionTitleAck;
	SharedMsg ackMsg(ack);
	ack->set_title(req->title());
	if (!election.CanElection(user->GetRank(), req->title()))
	{
		ack->set_error(ErrCode::NotEnoughPower);
	}
	else
	{
		if (title->GetLimitNum() < 0)
		{
			user->SetTitle(req->title());
			CUtility::UpdateUserInfo(user, 1 << NetMsg::UpdatePlayerAck::UpdateTitle);
			//CUtility::SendSysMail(user, "Title Award", "Title Award", title->GetAwardId(), NetMsg::MailType::TitleMail);
		}
		else
		{
			election.AddElectionUser(user, req->title());
			ack->set_playernum(election.GetElectionUserNum(user->GetForce(), req->title()));
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

ServerPB::UserCamp *FindCamp(ServerPB::UserCamps &camps,int type)
{
	for (int j = 0; j < camps.camps_size(); j++)
	{
		auto &camp = camps.camps(j);
		if (type == camp.type())
		{
			return (ServerPB::UserCamp *)&camp;
		}
	}
	return nullptr;
}

ServerPB::UserCamp *AddUserCamp(ServerPB::UserCamps &camps, int type)
{
	auto camp = camps.add_camps();
	CCampMgr::get_const_instance().InitUserCamp(camp, type, 1);
	return camp;
}

void CopyCamp(const ServerPB::UserCamp *userCamp, NetMsg::CampState *netCamp)
{
	netCamp->set_lefttime(userCamp->lefttime());
	netCamp->set_level(userCamp->level());
	netCamp->set_outputdata(userCamp->outputdata());
	netCamp->set_type((NetMsg::CampType)userCamp->type());
}

void UpdateCamp(ServerPB::UserCamp *userCamp)
{
	auto campData = CCampMgr::get_const_instance().GetCamp(userCamp->type(), userCamp->level());
	if (campData == nullptr)
		return;

	float outData = userCamp->outputdata();
	float add = (time(nullptr) - userCamp->outputtime()) * campData->OutputPerHour / 3600;
	outData += add;
	float maxData = campData->OutputHourLimit * campData->OutputPerHour;
	if (outData > maxData)
		outData = maxData;
	userCamp->set_outputdata(outData);
	userCamp->set_outputtime(time(nullptr));

	if (userCamp->leveluptime() == 0)
	{
		userCamp->set_lefttime(0);
		return;
	}

	int left = userCamp->lefttime();
	left -= time(nullptr) - userCamp->leveluptime();
	if (left < 0)
	{
		left = 0;
		campData = CCampMgr::get_const_instance().GetCamp(userCamp->type(), userCamp->level()+1);
		if (campData != nullptr)
		{
			CCampMgr::get_const_instance().InitUserCamp(userCamp, userCamp->type(), userCamp->level()+1);
		}
	}
	else
	{
		userCamp->set_lefttime(left);
	}
}

void CUserPackage::ReqCampState(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqCampState, msg);
	DEF_GET_USER(user, session);

	ServerPB::UserCamps &userCamps = user->GetCamps();
	
	NetMsg::CampStateAck *ack = new NetMsg::CampStateAck;
	SharedMsg ackMsg(ack);

	for (int i = 0; i < req->types_size(); i++)
	{
		int type = req->types(i);
		if (type < NetMsg::BaseCamp || type > NetMsg::ArmoryCamp)
			break;
		auto userCamp = FindCamp(userCamps, type);
		auto camp = ack->add_states();
		if (userCamp != nullptr)
		{
			UpdateCamp((ServerPB::UserCamp *)userCamp);
		}
		else
		{
			userCamp = AddUserCamp(userCamps, type);
		}
		CopyCamp(userCamp, camp);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqLevelUpCamp(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqLevelUpCamp, msg);
	DEF_GET_USER(user, session);

	ServerPB::UserCamps &userCamps = user->GetCamps();
	auto userCamp = FindCamp(userCamps, req->type());
	if (userCamp == nullptr)
		return;

	auto campData = CCampMgr::get_const_instance().GetCamp(userCamp->type(), userCamp->level()+1);
	if (campData == nullptr)
		return;

	NetMsg::LevelUpCampAck *ack = new NetMsg::LevelUpCampAck;
	SharedMsg ackMsg(ack);
	ack->set_type(req->type());

	switch (req->type())
	{
	case NetMsg::BaseCamp:
		if (user->GetLevel() < campData->RequireLevel)
		{
			ack->set_error(ErrCode::NotEnoughPower);
			m_socket->SendMsg(session, ackMsg);
			return;
		}
		break;
	default:
	{
		auto baseCamp = FindCamp(userCamps, NetMsg::BaseCamp);
		if (baseCamp == nullptr)
			return;
		if (userCamp->level() >= baseCamp->level())
			return;
	}
	break;
	}
	if (user->GetCoin() < campData->LevelUpCost)
	{
		ack->set_error(ErrCode::NoMoney);
		m_socket->SendMsg(session, ackMsg);
		return;
	}
	CUtility::AddCoin(user, -(int)campData->LevelUpCost);
	//CCampMgr::get_const_instance().InitUserCamp(userCamp, req->type(), userCamp->level() + 1);
	//ack->set_lefttime(campData->LevelUpTime);
	userCamp->set_lefttime(campData->LevelUpTime);
	userCamp->set_leveluptime(time(nullptr));

	NetMsg::CampStateAck *campAck = new NetMsg::CampStateAck;
	SharedMsg campAckMsg(campAck);
	CopyCamp(userCamp, campAck->add_states());
	m_socket->SendMsg(session, campAckMsg);

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqGetCampOut(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqGetCampOut, msg);
	DEF_GET_USER(user, session);

	ServerPB::UserCamps &userCamps = user->GetCamps();
	auto userCamp = FindCamp(userCamps, req->type());
	if (userCamp == nullptr)
		return;

	UpdateCamp(userCamp);

	NetMsg::GetCampOutAck *ack = new NetMsg::GetCampOutAck;
	ack->set_type(req->type());
	int outputData = (int)userCamp->outputdata();
	ack->set_output(outputData);

	switch (req->type())
	{
	case NetMsg::MarketCamp:
		CUtility::AddCoin(user, outputData);
		break;
	case NetMsg::FarmlandCamp:
		user->SetStamina(user->GetStamina() + outputData);
		CUtility::UpdateUserInfo(user, 1 << NetMsg::UpdatePlayerAck::UpdateStamina);
		break;
	}
	userCamp->set_outputtime(time(nullptr));
	userCamp->set_outputdata(userCamp->outputdata() - outputData);

	NetMsg::CampStateAck *campAck = new NetMsg::CampStateAck;
	SharedMsg campAckMsg(campAck);
	CopyCamp(userCamp, campAck->add_states());
	m_socket->SendMsg(session, campAckMsg);

	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqClearCampCD(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqClearCampCD, msg);
	DEF_GET_USER(user, session);

	ServerPB::UserCamps &userCamps = user->GetCamps();
	auto userCamp = FindCamp(userCamps, req->type());
	if (userCamp == nullptr)
		return;

	uint32_t useCost = CCampMgr::get_const_instance().GetClearCDCost(req->type());
	if (useCost == 0)
		return;

	NetMsg::ClearCampCDAck *ack = new NetMsg::ClearCampCDAck;
	SharedMsg ackMsg(ack);
	ack->set_type(req->type());
	
	UpdateCamp(userCamp);
	uint32_t leftTime = userCamp->lefttime();
	uint32_t m = leftTime / 60;
	if (leftTime % 60 != 0)
		m++;
	int cost = useCost*m;
	if (user->GetGold() < cost)
	{
		ack->set_error(ErrCode::NoMoney);
	}
	else
	{
		CUtility::AddGold(user, -cost);
		CCampMgr::get_const_instance().InitUserCamp(userCamp, userCamp->type(), userCamp->level() + 1);
		NetMsg::CampStateAck *campAck = new NetMsg::CampStateAck;
		SharedMsg campAckMsg(campAck);
		CopyCamp(userCamp, campAck->add_states());
		m_socket->SendMsg(session, campAckMsg);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::DelCycBattle(CUser *user)
{
	auto  &battle = user->GetCycleBattle();
	uint64_t battleId = battle.battleid();
	if (battleId != 0)
	{
		auto b = user->GetBattle(battleId);
		if (b != nullptr)
		{
			b->SetDelBattle(true);
		}
	}
	battle.set_battleid(0);
	battle.set_enemypath(0);
	battle.set_enemyteam(0);
	battle.set_percent(0);
}

void CUserPackage::AddCycBattle(CUser *user)
{
	DelCycBattle(user);

	auto  &battle = user->GetCycleBattle();
	uint64_t battleId = CUserBattle::GenerateUID();
	CycleBattleData cycData = CCycleBattle::get_const_instance().GetData(user);
	CEnemyTeam *team = CEnemyTeamMgr::get_const_instance().GetTeam(cycData.enemyTeamId);
	if (team == nullptr)
		return;

	std::vector<uint32_t> paths;
	CWorldPathMgr::get_mutable_instance().GetPaths(paths);
	if (paths.size() <= 0)
		return;
	uint32_t path = CUtility::RandomSel(paths);
	battle.set_enemypath(path);
	float r = CUtility::RandomInt(1,100) / 100.0f;
	battle.set_battleid(battleId);
	battle.set_percent(r);
	battle.set_enemypath(path);
	battle.set_enemyteam(cycData.enemyTeamId);
	battle.set_percent(r);

	CUserBattle *userBattle = new CUserBattle;
	userBattle->SetTeamId(team->GetId());
	userBattle->SetAwardId(team->GetAwardId());
	auto enemys = team->GetEnemys();

	for (size_t i = 0; i < enemys->size(); i++)
	{
		EnemyInfo &info = enemys->at(i);
		uint32_t enemyId = GetEnemyId(i, info);
		if (enemyId != 0)
			userBattle->AddEnemy(enemyId, user->GetLevel(), info);
	}

	userBattle->SetType(CUserBattle::CycBattle);
	userBattle->SetTypeData(battleId);
	userBattle->SetId(battleId);
	userBattle->GetBattleData().set_battleid(battleId);
	userBattle->SetAwardId(cycData.awardId);
	user->AddBattle(userBattle);
}

uint32_t CUserPackage::CycBattleEnd(CUser * user)
{
	ServerPB::UserCycBattle &battle = user->GetCycleBattle();
	//battle.set_maxdifficulty(1);
	//if()
	auto &cycBattle = CCycleBattle::get_const_instance();
	uint32_t maxRound = cycBattle.GetMaxRound(battle.curdifficulty());
	uint32_t maxDiff = cycBattle.GetMaxDiff();
	
	battle.set_curround(battle.curround() + 1);

	uint32_t awardId = cycBattle.GetData(user).awardId;

	DelCycBattle(user);

	if (battle.curround() > maxRound)
	{
		int diff = battle.curdifficulty() + 1;
		if (battle.maxdifficulty() <= maxDiff && diff > battle.maxdifficulty())
		{
			battle.set_maxdifficulty(diff);
			battle.set_leftunlocktimes(1);
		}
		battle.set_curround(0);
	}
	else
	{
		AddCycBattle(user);
	}

	NetMsg::CycleBattleAck *battleAck = new NetMsg::CycleBattleAck;
	CopyCycBattle(battle, battleAck);
	SharedMsg battleAckMsg(battleAck);
	m_socket->SendMsg(user->GetSession(), battleAckMsg);

	NetMsg::CycleBattleInfoAck *infoAck = new NetMsg::CycleBattleInfoAck;
	SharedMsg infoAckMsg(infoAck);
	CopyCycBattle(battle, infoAck);
	m_socket->SendMsg(user->GetSession(), infoAckMsg);

	return awardId;
}

void CUserPackage::InitCycBattle(CUser *user,time_t refTime)
{
	ServerPB::UserCycBattle &battle = user->GetCycleBattle();
	battle.set_maxdifficulty(1);
	battle.set_curdifficulty(0);
	battle.set_curround(1);
	battle.set_leftunlocktimes(0);
	battle.set_lefttime(0);
	//AddCycBattle(user);
}

void CUserPackage::CopyCycBattle(ServerPB::UserCycBattle &battle, NetMsg::CycleBattleAck *netBattle)
{
	netBattle->set_curdifficulty(battle.curdifficulty());
	netBattle->set_curround(battle.curround());
	netBattle->set_lefttime(battle.lefttime());
	netBattle->set_leftunlocktimes(battle.leftunlocktimes());
	netBattle->set_maxdifficulty(battle.maxdifficulty());
}

void CUserPackage::CopyCycBattle(ServerPB::UserCycBattle &battle, NetMsg::CycleBattleInfoAck *netBattle)
{
	netBattle->set_battleid(battle.battleid());
	netBattle->set_enemypath(battle.enemypath());
	netBattle->set_enemyteam(battle.enemyteam());
	netBattle->set_percent(battle.percent());
}

void CUserPackage::UpdateCycBattle(ServerPB::UserCycBattle &battle,time_t refreshTime)
{
	int leftTime = refreshTime - time(nullptr);
	if (leftTime <= 0)
	{
		leftTime = 0;
		battle.set_leftunlocktimes(1);
	}
	battle.set_lefttime(leftTime);

}

time_t CUserPackage::GetCycBattRefTime()
{
	static time_t refreshTime = 0;
	time_t now = time(nullptr);
	const int TIME_SPACE = 60 * 20;
	if (now - refreshTime > TIME_SPACE)
		refreshTime = now + TIME_SPACE;
	//刷新时间：每天临晨5点
	return refreshTime;
}

void CUserPackage::ReqCycleBattle(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	time_t refreshTime = GetCycBattRefTime();

	auto &userCycBatt = user->GetCycleBattle();
	if (userCycBatt.maxdifficulty() == 0)
	{
		InitCycBattle(user,refreshTime);
	}
	else
	{
		UpdateCycBattle(userCycBatt, refreshTime);
	}
	NetMsg::CycleBattleAck *ack = new NetMsg::CycleBattleAck;
	CopyCycBattle(userCycBatt, ack);
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRefresshCycle(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	auto &userBattle = user->GetCycleBattle();
	UpdateCycBattle(userBattle, GetCycBattRefTime());

	NetMsg::RefresshCycleAck *ack = new NetMsg::RefresshCycleAck;
	SharedMsg ackMsg(ack);

	if (userBattle.leftunlocktimes() > 0)
	{
		userBattle.set_leftunlocktimes(userBattle.leftunlocktimes()-1);
	}
	else
	{
		ack->set_error(ErrCode::RefresshCycError);
		m_socket->SendMsg(session, ackMsg);
		return;
	}
	uint64_t battleId = userBattle.battleid();

	userBattle.set_curdifficulty(0);
	userBattle.set_curround(1);
	userBattle.set_enemypath(0);
	userBattle.set_percent(0);
	userBattle.set_battleid(0);
	userBattle.set_enemyteam(0);

	if (battleId != 0)
	{
		auto b = user->GetBattle(battleId);
		if (b != nullptr)
		{
			b->SetDelBattle(true);
		}
		user->SetBattleId(0);
	}
	NetMsg::CycleBattleAck *battleAck = new NetMsg::CycleBattleAck;
	CopyCycBattle(userBattle, battleAck);
	SharedMsg battleAckMsg(battleAck);
	m_socket->SendMsg(session, battleAckMsg);

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqSelectCycleDiff(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqSelectCycleDiff, msg);
	DEF_GET_USER(user, session);

	auto &userBattle = user->GetCycleBattle();

	NetMsg::SelectCycleDiffAck *ack = new NetMsg::SelectCycleDiffAck;
	SharedMsg ackMsg(ack);
	ack->set_difficulty(req->difficulty());

	if (req->difficulty() > CCycleBattle::get_const_instance().GetMaxDiff() || userBattle.maxdifficulty() < req->difficulty() || userBattle.curdifficulty() != 0)
	{
		ack->set_error(ErrCode::SelectCycDiffError);
		m_socket->SendMsg(session, ackMsg);
		return;
	}
	userBattle.set_curround(1);
	userBattle.set_curdifficulty(req->difficulty());

	AddCycBattle(user);

	NetMsg::CycleBattleAck *battleAck = new NetMsg::CycleBattleAck;
	CopyCycBattle(userBattle, battleAck);
	SharedMsg battleAckMsg(battleAck);
	m_socket->SendMsg(session, battleAckMsg);

	NetMsg::CycleBattleInfoAck *infoAck = new NetMsg::CycleBattleInfoAck;
	SharedMsg infoAckMsg(infoAck);
	CopyCycBattle(userBattle, infoAck);
	m_socket->SendMsg(session, infoAckMsg);

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqCycleBattleInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	auto &userBattle = user->GetCycleBattle();

	NetMsg::CycleBattleInfoAck *ack = new NetMsg::CycleBattleInfoAck;
	SharedMsg ackMsg(ack);
	CopyCycBattle(userBattle, ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRecruitHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRecruitHero, msg);
	DEF_GET_USER(user, session);

	auto info = CRecruitHero::get_const_instance().GetRecruitInfo(req->recruitid());
	if (info == nullptr)
		return;

	NetMsg::RecruitHeroAck *ack = new NetMsg::RecruitHeroAck;
	SharedMsg ackMsg(ack);
	ack->set_recruitid(req->recruitid());
	//NoMoney,AlreadyRecruited,NotEnoughContribution
	if (user->RecruitHero(req->recruitid()))
	{
		ack->set_error(ErrCode::AlreadyRecruited);
	}
	else if (user->GetContribution() < info->contribution)
	{
		ack->set_error(ErrCode::NotEnoughContribution);
	}
	else if (user->GetGold() < info->price)
	{
		ack->set_error(ErrCode::NoMoney);
	}
	else
	{
		int gold = info->price;
		CUtility::AddGold(user, -gold);
		CUtility::AddUserHero(user, info->heroId);
		user->AddRecruitHero(req->recruitid());
		
		NetMsg::RecruitedHeroAck *ackRecruit = new NetMsg::RecruitedHeroAck;
		SharedMsg ackRecruitMsg(ackRecruit);
		ackRecruit->add_recruitid(req->recruitid());
		ackRecruit->add_recruited(true);
		m_socket->SendMsg(session, ackRecruitMsg);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRecruitedHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRecruitedHero, msg);
	DEF_GET_USER(user, session);

	NetMsg::RecruitedHeroAck *ack = new NetMsg::RecruitedHeroAck;
	SharedMsg ackMsg(ack);

	for (int i = 0; i < req->recruitids_size(); i++)
	{
		uint32_t id = req->recruitids(i);
		ack->add_recruitid(id);
		if (user->RecruitHero(id))
			ack->add_recruited(true);
		else
			ack->add_recruited(false);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqVSBattleBegin(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqVSBattleBegin, msg);
	DEF_GET_USER(user, session);

	NetMsg::VSBattleBeginAck *ack = new NetMsg::VSBattleBeginAck;

	auto heroClass = CHeroClassMgr::get_const_instance().GetHeroClass(req->heroid());

	if (user->GetNewGuide() >= 22 && !user->CouldGetFriendshipByLevel(req->id()))
	{
		ack->set_error(ErrCode::FriendShipLevelError);
		SharedMsg ackMsg(ack);
		m_socket->SendMsg(session, ackMsg);
		return;
	}
	auto hero = user->GetHero(req->heroid());
	if (hero == nullptr)
		return;
	if(!user->CouldBeLearn(req->id()))
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		SharedMsg ackMsg(ack);
		m_socket->SendMsg(session, ackMsg);
		return;
	}


	NetMsg::HeroAck *heroAck = new NetMsg::HeroAck;
	SharedMsg heroMsg(heroAck);

	int friLevel = user->GetHeroFriendship(req->id()).lv;
	if (friLevel < 1)
		friLevel = 1;
	time_t now = time(nullptr);
	if (hero->GetCDTime() <= now)
	{
		CLearnAndVsCD cd;
		CLearnAndVsCDMgr::get_const_instance().GetCDinfo(friLevel, cd);
		hero->SetCDTime(now + cd.student_learn_cd);
		CUtility::CopyHero(hero, heroAck);
		m_socket->SendMsg(session, heroMsg);

	}
	else
	{
		ack->set_error(ErrCode::DoTaskHeroInCD);
		SharedMsg ackMsg(ack);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	user->doAddUserCD(req->id(), friLevel);
	SendUserLearnVsCD(user, req->id());
	
	auto &battleMgr = CVsBattleMgr::get_mutable_instance();
	auto battle = battleMgr.CreateBattle((VSType)req->type(), user->GetUserId(), hero, req->id());
	if (battle == nullptr)
		return;


	battle->GetEnemy(*(ack->add_enemys()));
	ack->set_battleid(battle->GetBattleId());
	ack->set_id(req->id());
	ack->set_type(req->type());
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqVSBattleReady(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqVSBattleReady, msg);
	DEF_GET_USER(user, session);

	auto battle = CVsBattleMgr::get_mutable_instance().GetBattle(req->battleid());
	if (battle == nullptr)
		return;

	battle->SetReadTime();

	NetMsg::VSBattleReadyAck *ack = new NetMsg::VSBattleReadyAck;
	ack->set_battleid(req->battleid());
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqVSBattleOpt(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqVSBattleOpt, msg);
	DEF_GET_USER(user, session);

	auto battle = CVsBattleMgr::get_mutable_instance().GetBattle(req->battleid());
	if (battle == nullptr)
		return;

	NetMsg::VSBattleOptAck *ack = new NetMsg::VSBattleOptAck;
	ack->set_battleid(req->battleid());
	ack->set_turn(req->turn());
	uint32_t type = battle->SetOptType(req->turn(), req->type());
	ack->set_type(type);
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}


void CUserPackage::ReqBioHeroInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqBioHeroInfo, msg);
	DEF_GET_USER(user, session);

	auto &userAddData = user->GetAddData();
	auto heroBios = userAddData.mutable_herobios();

	NetMsg::BioHeroInfoAck *ack = new NetMsg::BioHeroInfoAck;

    ack->set_heroclass(req->heroclass());
	
	auto findHeroBio = [=](ServerPB::BioClass &b) {
		return b.heroclass() == req->heroclass();
	};
	auto findBio = std::find_if(heroBios->begin(), heroBios->end(), findHeroBio);
	if (heroBios->end() != findBio)
	{
		for (int i = 0; i < findBio->step_size(); i++)
		{
			auto step = findBio->step(i);
			auto addStep = ack->add_steps();
			addStep->set_id(step.stepid());
			for (int j = 0; j < step.branchids_size(); j++)
			{
				addStep->add_branchescomplete(step.branchids(j));
			}
		}
	}
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqBioHeroStepDone(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqBioHeroStepDone, msg);
	DEF_GET_USER(user, session);

	auto &userAddData = user->GetAddData();
	auto heroBios = userAddData.mutable_herobios();

	auto findHeroBio = [=](ServerPB::BioClass &b) {
		return b.heroclass() == req->heroclass();
	};

	NetMsg::BioHeroStepDoneAck * ack = new NetMsg::BioHeroStepDoneAck;
	ack->set_heroclass(req->heroclass());
	ack->set_stepid(req->stepid());
	SharedMsg ackMsg(ack);

	auto findBio = std::find_if(heroBios->begin(), heroBios->end(), findHeroBio);
	if (heroBios->end() == findBio)
	{
		auto bio = heroBios->Add();
		bio->set_heroclass(req->heroclass());
		auto step = bio->add_step();
        step->set_stepid(req->stepid());
		step->add_branchids(req->branchid());
		ack->add_unlockstepids(req->branchid());
	}
	else
	{
		auto findStepFun = [=](ServerPB::BioStep &step) {
			return  step.stepid() == req->stepid();
		};
		auto steps = findBio->mutable_step();
		auto findStep = std::find_if(steps->begin(), steps->end(), findStepFun);
		if (steps->end() == findStep)
		{
			auto step = steps->Add();
			step->set_stepid(req->stepid());
			step->add_branchids(req->branchid());
			ack->add_unlockstepids(req->branchid());
		}
		else
		{			
			for (int i = 0; i < findStep->branchids_size(); i++)
			{
				auto branchId = findStep->branchids(i);
				if (branchId == req->branchid())
					return;
				ack->add_unlockstepids(branchId);
			}
			findStep->add_branchids(req->branchid());
			ack->add_unlockstepids(req->branchid());
		}
	}
	if (req->branchid() == 0)
	{
		uint32_t award = CBiography::get_const_instance().GetAward(req->heroclass(), req->stepid());
		if (award != 0)
		{
			std::vector<uint32_t> awards = { award };
			CUtility::AddAwards(user, awards);
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRaffleHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRaffleHero, msg);
	DEF_GET_USER(user, session);

	auto &raffHero = CRaffleHero::get_const_instance();
	auto cost = raffHero.GetCost(req->id(),req->num());
	if (cost.priceTye < 0)
		return;

	NetMsg::RaffleHeroAck * ack = new NetMsg::RaffleHeroAck;
	SharedMsg ackMsg(ack);

	CRaffleHero::LeftTimes leftTimes = raffHero.GetLeftRaffTimes(user, req->id());
	if (leftTimes.tolTimes < req->num())
	{
		ack->set_error(ErrCode::NoTimes);
	}
	else if (!CUtility::DelPrice(user, cost.priceTye, cost.price))
	{
		ack->set_error(ErrCode::NoMoney);
	}
	else
	{
		std::vector<uint32_t> heros;
		raffHero.RaffleHeros(user, req->id(), req->num(), heros);
		for (auto i = heros.begin(); i != heros.end(); i++)
		{
			auto hero = CUtility::AddUserHero(user, *i);
			if (hero != nullptr)
				ack->add_herouid(hero->GetId());
		}
		raffHero.AddUseTimes(user, req->id(), req->num());
		leftTimes = raffHero.GetLeftRaffTimes(user, req->id());

		NetMsg::RaffleHeroTimesAck * timesAck = new NetMsg::RaffleHeroTimesAck;
		timesAck->set_times(leftTimes.tolTimes);
		timesAck->set_onerafftime(leftTimes.oneRaffTimes);
		SharedMsg ackTimesMsg(timesAck);
		m_socket->SendMsg(session, ackTimesMsg);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqRaffleHeroTimes(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRaffleHeroTimes, msg);
	DEF_GET_USER(user, session);

	NetMsg::RaffleHeroTimesAck * ack = new NetMsg::RaffleHeroTimesAck;
	CRaffleHero::LeftTimes lt = CRaffleHero::get_const_instance().GetLeftRaffTimes(user, req->id());
	if (lt.tolTimes < 0)
		lt.tolTimes = 0;

	ack->set_times(lt.tolTimes);
	ack->set_onerafftime(lt.oneRaffTimes);
	SharedMsg ackMsg(ack);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqMailInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	user->ReadMail();
	NetMsg::MailInfoAck * ack = new NetMsg::MailInfoAck;
	SharedMsg ackMsg(ack);

	auto &mails = user->GetMails();
	for (auto i = mails.begin(); i != mails.end(); i++)
	{
		if (i->GetDelMail())
			continue;
		auto mailData = ack->add_mails();
		CUtility::CopyMail(user,&(*i), mailData);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqMailDetail(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqMailDetail, msg);
	DEF_GET_USER(user, session);

	NetMsg::MailDetailAck * ack = new NetMsg::MailDetailAck;
	SharedMsg ackMsg(ack);
	auto &mails = user->GetMails();
	for (auto i = mails.begin(); i != mails.end(); i++)
	{
		if (i->GetId() == req->id())
		{
			ack->set_id(req->id());
			ack->set_content(i->GetContent());
			auto &award = i->GetAward();

			if (award.items_size() > 0)
			{
				auto ackAward = ack->mutable_award();
				ackAward->CopyFrom(award);
			}
			i->UpdateIsRead();
			break;
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqDeleteMail(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqDeleteMail, msg);
	DEF_GET_USER(user, session);

	auto &mails = user->GetMails();
	for (auto i = mails.begin(); i != mails.end(); i++)
	{
		if (i->GetId() == req->id())
		{
			i->SetDelMail(true);
			break;
		}
	}
	NetMsg::DeleteMailAck * ack = new NetMsg::DeleteMailAck;
	SharedMsg ackMsg(ack);
	ack->set_id(req->id());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqGetMailAward(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqGetMailAward, msg);
	DEF_GET_USER(user, session);

	auto &mails = user->GetMails();
	for (auto i = mails.begin(); i != mails.end(); i++)
	{
		if (i->GetId() == req->id())
		{
			if (i->GetIsReceived())
				break;
			auto &award = i->GetAward();
			if (award.items_size() > 0)
			{
				CUtility::AddUserAwards(user, &award);
			}
			i->UpdateGiveAward();
			break;
		}
	}

	NetMsg::GetMailAwardAck * ack = new NetMsg::GetMailAwardAck;
	SharedMsg ackMsg(ack);
	ack->set_id(req->id());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqBanquetInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	NetMsg::BanquetInfoAck * ack = new NetMsg::BanquetInfoAck;
	SharedMsg ackMsg(ack);
	CopyBanquetInfo(user, ack);
	m_socket->SendMsg(session, ackMsg);
}

void GetLeftBanquetHero(NetMsg::BanquetHeros *banHeros, std::vector<uint32_t> &leftHeros)
{
	for (int i = 0; i < banHeros->heros_size(); i++)
	{
		if (!banHeros->banquet(i))
		{
			leftHeros.push_back(banHeros->heros(i));
		}
	}
}

void SetBanqetHero(NetMsg::BanquetHeros *banHeros, uint32_t heroId)
{
	for (int i = 0; i < banHeros->heros_size(); i++)
	{
		if (banHeros->heros(i) == heroId)
		{
			banHeros->set_banquet(i, true);
			break;
		}
	}
}

void SetAllBanqetHero(NetMsg::BanquetHeros *banHeros)
{
	for (int i = 0; i < banHeros->heros_size(); i++)
	{
		banHeros->set_banquet(i, true);
	}
}

void EraseBanquet(CUser *user,uint32_t uid)
{
	auto &addData = user->GetAddData();
	auto banInfo = addData.mutable_banquetinfos();

	for (auto i = banInfo->begin(); i != banInfo->end(); i++)
	{
		auto heros = i->mutable_info();
		if (heros->uid() == uid)
		{
			banInfo->erase(i);
			break;
		}
	}
}

void CUserPackage::ReqBanquetHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqBanquetHero, msg);
	DEF_GET_USER(user, session);

	auto &addData = user->GetAddData();
	auto banInfo = addData.mutable_banquetinfos();
	
	NetMsg::BanquetHeros *banHeros = nullptr;
	for (auto i = banInfo->begin(); i != banInfo->end(); i++)
	{
		auto heros = i->mutable_info();
		if (heros->uid() == req->uid())
		{
			banHeros = heros;
			break;
		}
	}
	if (banHeros == nullptr)
		return;
	
	uint32_t priceType = 0;
	uint32_t price = 0;
	float dis = 0;
	uint32_t tolPrice = 0;
	if (!CBanquet::get_const_instance().GetPrice(banHeros->banquetid(), priceType, price, dis))
		return;

	std::vector<uint32_t> addHeros, leftHeros;
	GetLeftBanquetHero(banHeros, leftHeros);
	if (leftHeros.size() <= 0)
		return;
	NetMsg::BanquetHeroAck *ack = new NetMsg::BanquetHeroAck;
	SharedMsg ackMsg(ack);
	ack->set_uid(req->uid());
	
	if (req->type() == 1)
	{
		if (banHeros->leftfreetimes() > 0)
			banHeros->set_leftfreetimes(banHeros->leftfreetimes() - 1);
		else
		{
			tolPrice = price;
		}
		uint32_t heroId = CUtility::RandomSel(leftHeros);
		addHeros.push_back(heroId);
	}
	else if (req->type() == 2)
	{
		tolPrice = price*(leftHeros.size() - banHeros->leftfreetimes())*dis;
		addHeros = leftHeros;
	}
	else
		return;

	if (tolPrice > 0)
	{
		if (!CUtility::DelPrice(user, priceType, tolPrice))
		{
			ack->set_error(ErrCode::NoMoney);
			m_socket->SendMsg(session, ackMsg);
			return;
		}
	}
	if (addHeros.size() == 1)
		SetBanqetHero(banHeros, *(addHeros.begin()));
	else
	{
		SetAllBanqetHero(banHeros);
	}
	for (auto i = addHeros.begin(); i != addHeros.end(); i++)
	{
		auto hero = CUtility::AddUserHero(user, *i);
		if(hero != nullptr)
			ack->add_heros(hero->GetId());
	}
	m_socket->SendMsg(session, ackMsg);

	NetMsg::BanquetInfoAck * ackBan = new NetMsg::BanquetInfoAck;
	SharedMsg ackBanMsg(ackBan);
	CopyBanquetInfo(user, ackBan);
	m_socket->SendMsg(session, ackBanMsg);
}

void CUserPackage::ReqAddFriend(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqAddFriend, msg);
	DEF_GET_USER(user, session);

	user->AddFriend(req->userid());

	NetMsg::FriendsInfoAck * ack = new NetMsg::FriendsInfoAck;
	SharedMsg ackMsg(ack);

	auto fu = CUtility::GetOnlineUser(req->userid());
	if (fu != nullptr)
	{
		auto info = ack->add_friends();
		info->set_force(fu->GetForce());
		info->set_icon(fu->GetIcon());
		char *p[3];
		string tmp = fu->GetNick();
		string nick;
		if (3 == CUtility::SplitLine(p, 3, (char*)tmp.c_str(), '|'))
		{
			nick.append(p[0]);
			nick.append(p[1]);
			nick.append(p[2]);
		}
		info->set_name(nick);
		info->set_online(true);
		info->set_rank(fu->GetRank());
		info->set_title(fu->GetTitle());
		info->set_userid(fu->GetUserId());
	}
	else
	{
		boost::format fmt("where id =%1%");
		fmt % req->userid();
		std::list<CReadData> datas;
		std::vector<const char *> fieldNames = { "id", "force", "icon", "nick", "rank", "title" };
		if (CReadWriteUser::ReadData(datas, fieldNames, "user_info", fmt.str().c_str()))
		{
			if (datas.size() <= 0)
				return;
			auto i = datas.begin();
			auto info = ack->add_friends();
			info->set_userid(i->GetVal<uint32_t>(0));
			info->set_force(i->GetVal<uint32_t>(1));
			info->set_icon(i->GetVal<uint32_t>(2));
			char *p[3];
			string nick;
			if (3 == CUtility::SplitLine(p, 3, (char*)i->GetVal(3), '|'))
			{
				nick.append(p[0]);
				nick.append(p[1]);
				nick.append(p[2]);
			}
			info->set_name(nick);
			info->set_online(true);
			info->set_rank(i->GetVal<uint32_t>(4));
			info->set_title(i->GetVal<uint32_t>(5));
		}
	}
	m_socket->SendMsg(session, ackMsg);
}
void CUserPackage::ReqFriendsInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);
	user->ReadFriend();

	NetMsg::FriendsInfoAck * ack = new NetMsg::FriendsInfoAck;
	SharedMsg ackMsg(ack);
	
	auto &friends = user->GetFriend();
	std::list<uint32_t> notOnlineFriends;
	for (auto i = friends.begin(); i != friends.end(); i++)
	{
		auto fu = CUtility::GetOnlineUser(i->id);
		if (fu != nullptr)
		{
			auto info = ack->add_friends();
			info->set_force(fu->GetForce());
			info->set_icon(fu->GetIcon());
			char *p[3];
			string tmp = fu->GetNick();
			string nick;
			if (3 == CUtility::SplitLine(p, 3, (char*)tmp.c_str(), '|'))
			{
				nick.append(p[0]);
				nick.append(p[1]);
				nick.append(p[2]);
			}
			info->set_name(nick);
			info->set_online(true);
			info->set_rank(fu->GetRank());
			info->set_title(fu->GetTitle());
			info->set_userid(fu->GetUserId());
		}
		else
		{
			notOnlineFriends.push_back(i->id);
		}
	}

	std::string strFriend;
	for (auto i = notOnlineFriends.begin(); i != notOnlineFriends.end(); i++)
	{
		CReadWriteUser::MakeInCondition(*i, strFriend);
	}
	if (!notOnlineFriends.empty())
	{
		boost::format fmt("where id in (%1%)");
		fmt % strFriend;
		std::list<CReadData> datas;
		std::vector<const char *> fieldNames = { "id","force","icon","nick","rank","title" };
		if (CReadWriteUser::ReadData(datas, fieldNames, "user_info", fmt.str().c_str()))
		{
			for (auto i = datas.begin(); i != datas.end(); i++)
			{
				auto info = ack->add_friends();
				info->set_userid(i->GetVal<uint32_t>(0));
				info->set_force(i->GetVal<uint32_t>(1));
				info->set_icon(i->GetVal<uint32_t>(2));
				char *p[3];
				string nick;
				if (3 == CUtility::SplitLine(p, 3, (char*)i->GetVal(3), '|'))
				{
					nick.append(p[0]);
					nick.append(p[1]);
					nick.append(p[2]);
				}
				info->set_name(nick);
				info->set_online(true);
				info->set_rank(i->GetVal<uint32_t>(4));
				info->set_title(i->GetVal<uint32_t>(5));
			}
		}
	}
	m_socket->SendMsg(session, ackMsg);
}
void CUserPackage::ReqRemoveFriend(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqRemoveFriend, msg);
	DEF_GET_USER(user, session);

	user->DelFriend(req->userid());

	NetMsg::RemoveFriendAck * ack = new NetMsg::RemoveFriendAck;
	SharedMsg ackMsg(ack);
	ack->set_userid(req->userid());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqPrepareBattle(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqPrepareBattle, msg);
	DEF_GET_USER(user, session);

	CEnemyTeam *team = CEnemyTeamMgr::get_const_instance().GetTeam(req->enemyteam());
	if (team == nullptr)
		return;

	CUserBattle *userBattle = new CUserBattle;
	userBattle->SetTeamId(team->GetId());
	userBattle->SetAwardId(team->GetAwardId());
	auto enemys = team->GetEnemys();

	for (size_t i = 0; i < enemys->size(); i++)
	{
		EnemyInfo &info = enemys->at(i);
		uint32_t enemyId = GetEnemyId(i, info);
		if (enemyId != 0)
			userBattle->AddEnemy(enemyId, user->GetLevel(), info);
	}
	userBattle->SetType(CUserBattle::BiographyBattle);
	//userBattle->SetTypeData(acceptQuest->questid());
	userBattle->SetId(CUserBattle::GenerateUID());
	userBattle->GetBattleData().set_battleid(userBattle->GetId());
	user->AddBattle(userBattle);

	const auto &battle = userBattle->GetBattleData();

	NetMsg::PrepareBattleAck *ack = new NetMsg::PrepareBattleAck;
	SharedMsg ackMsg(ack);
	ack->set_battleid(battle.battleid());
	ack->mutable_enemys()->CopyFrom(battle.enemys());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWar(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	std::list<CWorldWar::WarInfo*> wars;
	CWorldWar::get_mutable_instance().GetAllWars(wars);

	NetMsg::WorldWarAck *ack = new NetMsg::WorldWarAck;
	SharedMsg ackMsg(ack);
	
	for (auto i = wars.begin(); i != wars.end(); i++)
	{
		auto info = ack->add_wars();
		auto war = *i;
		info->set_id(war->id);
		info->set_force1(war->force1);
		info->set_force2(war->force2);
		info->set_path(war->path);
		info->set_percent(war->percent);
		info->set_type(war->type);
		int leftTime = war->endTime - time(nullptr);
		if (leftTime < 0)
			leftTime = 0;
		info->set_lefttime(leftTime);
		info->set_warid(war->warId);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldBattle(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldBattle, msg);
	DEF_GET_USER(user, session);

	user->ReadBattle();

	NetMsg::WorldBattleAck *ack = new NetMsg::WorldBattleAck;
	SharedMsg ackMsg(ack);
	ack->set_id(req->id());

	auto &worldWar = CWorldWar::get_mutable_instance();
	auto battles = user->GetBattles();
	for (auto i = battles->begin(); i != battles->end(); i++)
	{
		auto battle = i->second;
		if (battle->GetDelBattle())
			continue;
		if (battle->GetType() == CUserBattle::WorldWarBattle)
		{
			if (worldWar.GetWar(battle->GetTypeData()) == nullptr)
				battle->SetDelBattle(true);
			else if (battle->GetTypeData() == req->id())
			{
				ack->mutable_battle()->CopyFrom(battle->GetBattleData());
				m_socket->SendMsg(session, ackMsg);
				return;
			}
		}
	}

	auto battleInfo = worldWar.GetWar(req->id());
	if (battleInfo == nullptr)
	{
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	CUserBattle *userBattle = new CUserBattle;

	uint32_t enemyTeam = CUtility::RandomSel(battleInfo->enemyTeam);
	CEnemyTeam *team = CEnemyTeamMgr::get_const_instance().GetTeam(enemyTeam);
	if (team == nullptr)
		return;
	userBattle->SetTeamId(team->GetId());
	userBattle->SetAwardId(team->GetAwardId());
	userBattle->SetId(CUserBattle::GenerateUID());
	userBattle->GetBattleData().set_battleid(userBattle->GetId());
	user->AddBattle(userBattle);
	auto enemys = team->GetEnemys();

	for (size_t i = 0; i < enemys->size(); i++)
	{
		EnemyInfo &info = enemys->at(i);
		uint32_t enemyId = GetEnemyId(i, info);
		if (enemyId != 0)
			userBattle->AddEnemy(enemyId, user->GetLevel(), info);
	}
	userBattle->SetType(CUserBattle::WorldWarBattle);
	userBattle->SetTypeData(req->id());
	ack->mutable_battle()->CopyFrom(userBattle->GetBattleData());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWarTopList(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarTopList, msg);
	DEF_GET_USER(user, session);

	NetMsg::WorldWarTopListAck *ack = new NetMsg::WorldWarTopListAck;
	SharedMsg ackMsg(ack);
	CWorldWar::get_mutable_instance().GetWorldWarTopList(user, req->id(), ack);
	m_socket->SendMsg(session, ackMsg);
}

//add by clx 
void CUserPackage::ReqHeroGeneralSoul(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::HeroGeneralSoulAck *heroGeneralSoulAck = new NetMsg::HeroGeneralSoulAck;
	SharedMsg ack(heroGeneralSoulAck);
	user->GetHeroGeneralSoul(heroGeneralSoulAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqUserLearnVsCD(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	NetMsg::UserLearnVsCDAck *userLearnVsCDAck = new NetMsg::UserLearnVsCDAck;
	SharedMsg ack(userLearnVsCDAck);
	//user->GetUserLearnCD(userLearnVsCDAck);
	user->GetUserLearnCDToClient(userLearnVsCDAck);
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqGeneralSoulExchangeHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqGeneralSoulExchangeHero, msg);
	DEF_GET_USER(user, session);

	NetMsg::GeneralSoulExchangeHeroAck *generalSoulExchangeHeroAck = new NetMsg::GeneralSoulExchangeHeroAck;
	SharedMsg ack(generalSoulExchangeHeroAck);

	CUser::GeneralSoul soul = user->GetHeroGeneralSoul(req->heroid());

	
	if(CGeneralSoulMgr::get_const_instance().GeneralSoulExchangeHeroSuccess(req->heroid(), soul))
	{
		auto firstHero = CHeroMgr::get_const_instance().GetHeroId(req->heroid());
		auto hero = CUtility::AddUserHero(user, firstHero, true);//lyle error
		if (hero != nullptr)
			generalSoulExchangeHeroAck->set_heroid(req->heroid());

		CGeneralSoulMgr::get_const_instance().doExchangeSoul(user,req->heroid());
		SendHeroClassGeneralSoul(user, req->heroid());
	}
	m_socket->SendMsg(session, ack);
}

void CUserPackage::ReqCityItem(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqCityItem, msg);
	DEF_GET_USER(user, session);
	SendCityItem(user, req->cityid());
}

void CUserPackage::ReqCityHero(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqCityHero, msg);
	DEF_GET_USER(user, session);
	SendCityHero(user, req->cityid());
}

void CUserPackage::ReqResourceInfo(UserSession session, SharedMsg msg)
{
	CUser *user;
	GET_USER(user, session);

	user->ReadResource();
	//SendUserAllResource(user);
	CUtility::SendUserAllResource(user);
}

void CUserPackage::ReqResourceDetailInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqResourceDetailInfo, msg);
	DEF_GET_USER(user, session);

	NetMsg::ResourceDetailInfoAck *detailAck = new NetMsg::ResourceDetailInfoAck;
	SharedMsg ack(detailAck);

	auto hasRes = CResourceMgr::get_const_instance().GetDetailResource(user, req->uid(), detailAck);
	if(!hasRes)
	{
		detailAck->set_error(ErrCode::NoThisField);
	}
	m_socket->SendMsg(session, ack);
}


void CUserPackage::SendCityItem(CUser *user,uint32_t cityId)
{
	NetMsg::CityItemAck *cityItemAck = new NetMsg::CityItemAck;
	SharedMsg ack(cityItemAck);

	CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
	if (city){
		city->GetCityItem(cityItemAck);
	}else{
		cityItemAck->set_error(ErrCode::NoThisCity);
	}
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::SendCityHero(CUser *user, uint32_t cityId)
{
	NetMsg::CityHeroAck *cityHeroAck = new NetMsg::CityHeroAck;
	SharedMsg ack(cityHeroAck);

	CWorldCity *city = CWorldCityMgr::get_const_instance().GetCity(cityId);
	if (city){
		city->GetCityHero(cityHeroAck);
	}else{
		cityHeroAck->set_error(ErrCode::NoThisCity);
	}
	m_socket->SendMsg(user->GetSession(), ack);
}

void CUserPackage::ReqTeamInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_USER(user, session);

	auto &addData = user->GetAddData();
	auto teams = addData.mutable_teams();
	NetMsg::TeamInfoAck *ack = new NetMsg::TeamInfoAck;
	SharedMsg ackMsg(ack);
	for (int i = 0; i < teams->size(); i++)
	{
		auto info = ack->add_infos();
		info->CopyFrom(*(teams->Mutable(i)));
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::SetTeamCurCity(ServerPB::UserAddData *addData, uint32_t heroPos, int city)
{
	auto teams = addData->mutable_teams();
	if (heroPos >= teams->size())
		return;

	auto teamInfo = teams->Mutable(heroPos);
	teamInfo->set_curcity(city);
}

void CUserPackage::SetTeamUpdateTime(ServerPB::UserAddData *addData, uint32_t heroPos, time_t now)
{
	auto teams = addData->mutable_teams();
	if (heroPos >= teams->size())
		return;

	auto teamInfo = teams->Mutable(heroPos);
	teamInfo->set_updatetime(now);
}

void CUserPackage::SetTeamPath(ServerPB::UserAddData *addData, uint32_t heroPos, NetMsg::RunPathAck *path, CUser* user)
{
	auto teams = addData->mutable_teams();
	if (heroPos >= teams->size())
		return;

	auto teamInfo = teams->Mutable(heroPos);
	auto oldPath = teamInfo->mutable_path();
	if (oldPath->state() == NetMsg::ArrivedTarget && oldPath->mutable_path()->runtarget() == NetMsg::RunToResource)
	{
		uint64_t resourceUid = oldPath->mutable_path()->targetid();
		auto couldBeMining = CResourceMgr::get_const_instance().TeamStopMining(heroPos, resourceUid, user);
	}

	oldPath->CopyFrom(*path);
	if (path->state() == NetMsg::ArrivedTarget && path->mutable_path()->runtarget() == NetMsg::RunToCity)
	{
		teamInfo->set_curcity(path->mutable_path()->targetid());
		//CResourceMgr::get_const_instance().CalResourceFiedIsClose(user);
		teamInfo->set_curcity(path->mutable_path()->targetid());
	}
	else if (path->state() == NetMsg::ArrivedTarget && path->mutable_path()->runtarget() == NetMsg::RunToResource)
	{
		//lyle
		uint64_t resourceUid = path->mutable_path()->targetid();
		auto couldBeMining = CResourceMgr::get_const_instance().TeamStartMining(heroPos, resourceUid, user);
		//send msg to client , refuse to mining;
		teamInfo->set_curcity(-1);
	}
	else
	{
		teamInfo->set_curcity(-1);
	}
}

void CUserPackage::CalculateTeamRun(ServerPB::UserAddData *addData, uint32_t heroPos)
{
	auto teams = addData->mutable_teams();
	if (heroPos >= teams->size())
		return;

	auto userTeam = teams->Mutable(heroPos);
	time_t updateTime = userTeam->updatetime();
	time_t now = time(nullptr);
	if (updateTime == 0)
		updateTime = now;
	userTeam->set_updatetime(now);	

	const int speed = 10;
	auto info = userTeam->mutable_path();
	if (CalculateRunPos(info, now - updateTime, speed))
	{
		if (info->mutable_path()->runtarget() == NetMsg::RunToCity)
		{
			//user->SetCurCity((int)pathInfo->targetid());
			userTeam->set_curcity((int)info->mutable_path()->targetid());
		}
	}
}

void CUserPackage::ReqTeamRunPath(UserSession session, SharedMsg msg)
{
	uint32_t heroPos = 0;
	DEF_GET_USER(user, session);

	if (msg.get() != nullptr)
	{
		DEF_GET_REQ_MSG(req, NetMsg::ReqTeamRunPath, msg);
		heroPos = req->position();
	}
	auto &addData = user->GetAddData();
	auto teams = addData.mutable_teams();
	if (teams->size() <= heroPos)
		return;

	NetMsg::TeamRunPathAck *ack = new NetMsg::TeamRunPathAck;
	SharedMsg ackMsg(ack);
	
	CalculateTeamRun(&addData, heroPos);
	auto path = (teams->Mutable(heroPos)->mutable_path());
	ack->mutable_path()->CopyFrom(*path);
	ack->set_position(heroPos);
	ack->set_curcity(teams->Mutable(heroPos)->curcity());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::SetTeamRunPath(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::SetTeamRunPath, msg);
	DEF_GET_USER(user, session);

	auto &addData = user->GetAddData();
	SetTeamPath(&addData, req->position(), req->mutable_path(),user);
	SetTeamUpdateTime(&addData, req->position(), time(nullptr));
}

void CUserPackage::ReqTeamArrivedTar(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqTeamArrivedTar, msg);
	DEF_GET_USER(user, session);

	NetMsg::TeamArrivedTarAck *ack = new NetMsg::TeamArrivedTarAck;
	SharedMsg ackMsg(ack);

	auto &addData = user->GetAddData();
	SetTeamPath(&addData, req->position(), req->mutable_path(),user);
	ack->set_position(req->position());
	auto p = ack->mutable_path();// (req->mutable_path());
	p->CopyFrom(req->path());
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqSetTeam(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqSetTeam, msg);
	DEF_GET_USER(user, session);

	auto &addData = user->GetAddData();
	auto teams = addData.mutable_teams();
	
	if (teams->size() < req->pos())
		return;

	if (req->pos() >= 5)
		return;

	if (req->heroid_size() != 5)
		return;

	NetMsg::TeamInfo *teamInfo = nullptr;
	ServerPB::UserTeam *userTeam = nullptr;
	if (teams->size() == req->pos())
	{
		userTeam = teams->Add();
		SetTeamCurCity(&addData, req->pos(), user->GetCampCity());
	}
	else
	{
		userTeam = teams->Mutable(req->pos());
	}
	teamInfo = userTeam->mutable_team();

	NetMsg::SetTeamAck *ack = new NetMsg::SetTeamAck;
	SharedMsg ackMsg(ack);
	for (int i = 0; i < req->heroid_size(); i++)
	{
		uint64_t heroId = req->heroid(i);
		if (heroId != 0)
		{
			if (user->GetHero(heroId) == nullptr)
				ack->set_error(ErrCode::NoThisHero);
		}
		ack->add_heroid(heroId);
	}
	if (ack->error() == 0)
	{
		teamInfo->clear_heroid();
		for (int i = 0; i < req->heroid_size(); i++)
		{
			teamInfo->add_heroid(req->heroid(i));
		}
	}
	ack->set_pos(req->pos());
	m_socket->SendMsg(session, ackMsg);
}
void CUserPackage::ReqWriteSkillBook(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWriteSkillBook, msg);
	DEF_GET_USER(user, session);

	NetMsg::WriteSkillBookAck *ack = new NetMsg::WriteSkillBookAck;
	SharedMsg ackMsg(ack);

	std::vector<CItemInst*> itemInsts;
	std::list<std::shared_ptr<ItemReq>> clientMsg;
	clientMsg.clear();

	uint64_t heroId = req->heroid();
	CHeroInst *hero = user->GetHero(heroId);
	if (hero == nullptr)
	{
		ack->set_error(ErrCode::NoThisHero);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	for(int i=0; i<req->items_size(); i++)
	{
		const NetMsg::ItemInfo & info = req->items(i);

		bool hasItem = false;
		for (auto j = clientMsg.begin();j != clientMsg.end();j++)
		{
			auto item = *j;
			if (item->uid == info.uid())
			{
				hasItem = true;
				item->num += info.num();
				break;
			}
		}

		if(!hasItem)
		{
			std::shared_ptr<ItemReq> reqItem (new ItemReq);
			reqItem->uid = info.uid();
			reqItem->num = info.num();
			clientMsg.push_back(reqItem);
		}
	}

	for(auto i= clientMsg.begin();i != clientMsg.end();i++)
	{
		auto itemReq = *i;
		auto uid = itemReq->uid;
		auto num = itemReq->num;

		CItemInst *itemInst = user->GetItem(uid);
		if(!itemInst)
		{
			ack->set_error(ErrCode::NoThisItem);
			m_socket->SendMsg(session, ackMsg);
			return;
		}
		int ownNum = itemInst->GetNum();
		if (num > ownNum)
		{
			ack->set_error(ErrCode::EmptyBookIsError);
			m_socket->SendMsg(session, ackMsg);
			return;
		}

		CItemTmpl *itemTmpl = itemInst->GetItemTmpl();
		if(!itemTmpl)
		{
			ack->set_error(ErrCode::NoThisItem);
			m_socket->SendMsg(session, ackMsg);
			return;
		}

		auto itemType = itemTmpl->GetType();
		if (itemType != CItemTmpl::ITEM_EMPTY_BOOK_SKILL)
		{
			ack->set_error(ErrCode::ItemTypeError);
			m_socket->SendMsg(session, ackMsg);
			return;
		}

		for (int i = 0;i < num;i++)
			itemInsts.push_back(itemInst);
	}

	if(itemInsts.size()>4 || itemInsts.size() <= 0)
	{
		ack->set_error(ErrCode::MoreEmptyBook);
		m_socket->SendMsg(session, ackMsg);
		return;
	}

	CHeroSkillBookMgr::get_const_instance().CalNewBook(itemInsts, user, heroId);
	CHeroSkillBookMgr::get_const_instance().GetNewBooks(ack);

	DelHero(user, hero);
	for(auto i = itemInsts.begin(); i != itemInsts.end(); i++)
	{
		//delete emptybook
		DelItem(user, *i, 1);
	}

	CHeroSkillBookMgr::get_const_instance().AddNewSkillBook(user);
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqClusterInfo(UserSession session, SharedMsg msg)
{
	//DEF_GET_REQ_MSG(req, NetMsg::ReqClusterInfo, msg);

	DEF_GET_USER(user, session);
	NetMsg::ClusterInfoAck *ack = new NetMsg::ClusterInfoAck;
	SharedMsg ackMsg(ack);
	
	auto &addData = user->GetAddData();
	auto teams = addData.mutable_teams();
	std::set<uint64_t> clusters;
	if (teams->size() > 0)
	{
		for (int i = 0; i < teams->size(); i++)
		{
			auto team = teams->Mutable(i);
			uint64_t clusterId = team->team().clusterid();
			if (clusterId != 0)
				clusters.insert(clusterId);
		}
	}
	auto &clusterMgr = CTeamClusterMgr::get_mutable_instance();
	for (auto i = clusters.begin(); i != clusters.end(); i++)
	{
		auto cluster = clusterMgr.GetCluster(*i);
		if (cluster != nullptr)
		{
			auto info = ack->add_infos();
			cluster->GetClusterInfoAck(info);
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqAllCluster(UserSession session, SharedMsg msg)
{
	//DEF_GET_REQ_MSG(req, NetMsg::ReqAllCluster, msg);
	DEF_GET_USER(user, session);

	NetMsg::AllClusterAck *ack = new NetMsg::AllClusterAck;
	SharedMsg ackMsg(ack);
	
	auto allCluster = CTeamClusterMgr::get_mutable_instance().GetAllCluster();
	for (auto i = allCluster->begin(); i != allCluster->end(); i++)
	{
		auto cluster = i->second;
		if (cluster->IsPub())
		{
			auto info = ack->add_infos();
			cluster->GetClusterInfoAck(info);
		}
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqCreateCluster(UserSession session, SharedMsg msg)
{
	//DEF_GET_REQ_MSG(req, NetMsg::ReqCreateCluster, msg);
	DEF_GET_USER(user, session);
	NetMsg::CreateClusterAck *ack = new NetMsg::CreateClusterAck;
	SharedMsg ackMsg(ack);
	
	CTeamCluster *cluster = new CTeamCluster;
	if (cluster->Init(user))
	{
		CTeamClusterMgr::get_mutable_instance().AddCluster(cluster);
		ack->set_uid(cluster->GetId());
	}
	else
	{
		delete cluster;
		ack->set_error(ErrCode::CreateClusterError);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqJoinCluster(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqJoinCluster, msg);
	DEF_GET_USER(user, session);

	auto &clusterMgr = CTeamClusterMgr::get_mutable_instance();
	auto cluster = clusterMgr.GetCluster(req->uid());
	if (cluster == nullptr)
		return;
	
	NetMsg::JoinClusterAck *ack = new NetMsg::JoinClusterAck;
	SharedMsg ackMsg(ack);
	ack->set_uid(req->uid());
	ack->set_teampos(req->teampos());
	if (cluster->Join(user, req->teampos()))
	{

	}
	else
	{
		ack->set_error(ErrCode::JoinClusterError);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqExitCluster(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqExitCluster, msg);
	DEF_GET_USER(user, session);

	auto &clusterMgr = CTeamClusterMgr::get_mutable_instance();
	auto cluster = clusterMgr.GetCluster(req->uid());
	if (cluster == nullptr)
		return;

	NetMsg::ExitClusterAck *ack = new NetMsg::ExitClusterAck;
	SharedMsg ackMsg(ack);
	if (cluster->Exit(user, req->teampos()))
	{
		if (req->teampos() == 0)
		{
			clusterMgr.DelCluster(req->uid());
			delete cluster;
		}
	}
	else
	{
		ack->set_error(ErrCode::ExitClusterError);
	}
	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqClusterDetailInfo(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqClusterDetailInfo, msg);
	DEF_GET_USER(user, session);
}

void CUserPackage::ReqSetClusterAttr(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqSetClusterAttr, msg);
	DEF_GET_USER(user, session);
}

void CUserPackage::ReqEnterWorldWar(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqEnterWorldWar, msg);
	DEF_GET_USER(user, session);

	NetMsg::EnterWorldWarAck *ack = new NetMsg::EnterWorldWarAck;
	SharedMsg ackMsg(ack);
	ack->set_worldwarid(req->worldwarid());

	CWorldWarScene* scene = CWorldWar::get_mutable_instance().GetWorldWarScene(req->worldwarid());
	if (scene)
	{
		scene->AddUser(user, ack);
		ack->set_error(0);
	}
	else
	{
		ack->set_error(ErrCode::WorldWarSceneNotStart);
	}

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWarStatus(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarStatus, msg);
	DEF_GET_USER(user, session);

	NetMsg::WorldWarStatusAck *ack = new NetMsg::WorldWarStatusAck;
	SharedMsg ackMsg(ack);

	CWorldWarScene* scene = CWorldWar::get_mutable_instance().GetWorldWarScene(req->worldwarid());
	if (scene)
	{
		scene->FillStatus(user, ack);
		ack->set_error(0);
	}
	else
	{
		ack->set_error(ErrCode::WorldWarSceneNotStart);
	}

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWarMove(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarMove, msg);
	DEF_GET_USER(user, session);

	NetMsg::WorldWarMoveAck *ack = new NetMsg::WorldWarMoveAck;
	SharedMsg ackMsg(ack);
	ack->set_worldwarid(req->worldwarid());
	ack->set_agentid(req->agentid());

	CWorldWarScene* scene = CWorldWar::get_mutable_instance().GetWorldWarScene(req->worldwarid());
	if (scene)
	{
		scene->AgentMove(user, req);
		ack->set_error(0);
	}
	else
	{
		ack->set_error(ErrCode::WorldWarSceneNotStart);
	}

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWarDefence(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarDefence, msg);
	DEF_GET_USER(user, session);

	NetMsg::WorldWarDefenceAck *ack = new NetMsg::WorldWarDefenceAck;
	SharedMsg ackMsg(ack);
	ack->set_worldwarid(req->worldwarid());
	ack->set_agentid(req->agentid());

	CWorldWarScene* scene = CWorldWar::get_mutable_instance().GetWorldWarScene(req->worldwarid());
	if (scene)
	{
		scene->AgentDefence(user, req);
		ack->set_error(0);
	}
	else
	{
		ack->set_error(ErrCode::WorldWarSceneNotStart);
	}

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqWorldWarDefenceTarget(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarDefenceTarget, msg);
	DEF_GET_USER(user, session);

	NetMsg::WorldWarDefenceTargetAck *ack = new NetMsg::WorldWarDefenceTargetAck;
	SharedMsg ackMsg(ack);
	ack->set_worldwarid(req->worldwarid());
	ack->set_agentid(req->agentid());

	CWorldWarScene* scene = CWorldWar::get_mutable_instance().GetWorldWarScene(req->worldwarid());
	if (scene)
	{
		scene->AgentSetTarget(user, req);
		ack->set_error(0);
	}
	else
	{
		ack->set_error(ErrCode::WorldWarSceneNotStart);
	}

	m_socket->SendMsg(session, ackMsg);
}

void CUserPackage::ReqLeaveWorldWar(UserSession session, SharedMsg msg)
{
	DEF_GET_REQ_MSG(req, NetMsg::ReqWorldWarDefenceTarget, msg);
	DEF_GET_USER(user, session);

	user->SetIsInWorldWarScene(false);

	NetMsg::LeaveWorldWarAck *ack = new NetMsg::LeaveWorldWarAck;
	SharedMsg ackMsg(ack);
	ack->set_worldwarid(req->worldwarid());
	m_socket->SendMsg(session, ackMsg);
}