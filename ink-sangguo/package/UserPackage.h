#pragma once
#include "CmdPackInterface.h"
#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include "session.h"
#include "Award.h"
#include "ServerPb.pb.h"
#include "NetMsg.pb.h"

class CSocket;
class CRsaCode;
class COnlineUser;
class CUser;
class CHeroInst;
class CUserQuest;
class CItemInst;
class CHeroClass;
struct AdEventData;
class CUserBattle;
class CUserTopList;

class CUserPackage:public CCmdPackageInterface
{
public:
	CUserPackage();
	~CUserPackage();
	void Init(CCommand*);
private:
	//function<void(UserSession, SharedMsg)> CommandFun;
	void RegAccount(UserSession, SharedMsg);
	void UserLogin(UserSession, SharedMsg);
	void GetServerList(UserSession, SharedMsg);
	void EnterGame(UserSession, SharedMsg);
	void CreateUser(UserSession, SharedMsg);
	void GetClientVersion(UserSession, SharedMsg);

	void GetNamePassword(UserSession, SharedMsg);
	
	void ThirdPartyLogin(UserSession, SharedMsg);
	void GetOrderId(UserSession, SharedMsg);
	
	void Chat(UserSession session, SharedMsg msg);

	void PToPChat(CUser *user,UserSession &session, NetMsg::ReqChat *chatMsg);
	void WorldChat(CUser *user, NetMsg::ReqChat *chatMsg);
	void ForceChat(CUser *user, NetMsg::ReqChat *chatMsg);
	void FriendChat(CUser *user, NetMsg::ReqChat *chatMsg);

	void GetUserNick(UserSession session, SharedMsg msg);

	void SetNewGuide(UserSession session, SharedMsg);

	void GetHeros(UserSession session, SharedMsg);

	void GetRunPath(UserSession session, SharedMsg);
	void SetRunPath(UserSession session, SharedMsg msg);

	void ArrivedCity(UserSession session, SharedMsg msg);

	void SetHeroFormation(UserSession session, SharedMsg msg);
	void GetHeroFormation(UserSession session, SharedMsg);

	void GetCityInfo(UserSession session, SharedMsg msg);

	void GetQuestList(UserSession session, SharedMsg);
	void GetQuestInfo(UserSession session, SharedMsg msg);

	void DoQuest(UserSession session, SharedMsg msg);
	void GetHero(UserSession session, SharedMsg msg);

	void AcceptQuest(UserSession session, SharedMsg msg);
	void SubmitQuest(UserSession session, SharedMsg msg);

	void ClearHeroCD(UserSession session, SharedMsg msg);
	void RefreshQuest(UserSession session, SharedMsg);

	void ReqBattleEnd(UserSession session, SharedMsg msg);
	void ReqBattle(UserSession session, SharedMsg msg);

	void ReqItem(UserSession session, SharedMsg msg);
	void ReqAddItem(UserSession session, SharedMsg msg);
	void ReqHeroUseItem(UserSession session, SharedMsg msg);
	void ReqUseItem(UserSession session, SharedMsg msg);
	void ReqHeroFriendship(UserSession session, SharedMsg msg);
	void ReqHeroGeneralSoul(UserSession session, SharedMsg msg);
	void ReqUserLearnVsCD(UserSession session, SharedMsg msg);
	void ReqGiveHeroGift(UserSession session, SharedMsg msg);

	void ReqShopItem(UserSession session, SharedMsg msg);
	void ReqBuyShop(UserSession session, SharedMsg msg);
	void ReqAdvantureInfo(UserSession session, SharedMsg msg);
	void ReqAdvanture(UserSession session, SharedMsg msg);
	void ReqAdvantureEventDone(UserSession session, SharedMsg msg);

	void ReqHeroClass(UserSession session, SharedMsg msg);
	void ReqStudySkill(UserSession session, SharedMsg msg);
	void ReqStudyProperty(UserSession session, SharedMsg msg);

	void ReqRecruitHero(UserSession session, SharedMsg msg);
	void ReqRecruitedHero(UserSession session, SharedMsg msg);

	void CopyHeroClass(CHeroClass *hero, NetMsg::HeroClassAck *netHero);

	void DelItem(CUser*user, CItemInst *item, uint32_t useNum);

	void ReqEvolveHero(UserSession session, SharedMsg msg);
	
	void ReqAddMoney(UserSession session, SharedMsg msg);

	void ReqTopList(UserSession session, SharedMsg msg);

	void ReqAdvancementRank(UserSession session, SharedMsg msg);
	void ReqRankTitle(UserSession session, SharedMsg msg);
	void ReqTitleInfo(UserSession session, SharedMsg msg);
	void ReqElectionTitle(UserSession session, SharedMsg msg);
	
	void ReqCampState(UserSession session, SharedMsg msg);
	void ReqLevelUpCamp(UserSession session, SharedMsg msg);
	void ReqGetCampOut(UserSession session, SharedMsg msg);
	void ReqClearCampCD(UserSession session, SharedMsg msg);

	void ReqCycleBattle(UserSession session, SharedMsg msg);
	void ReqRefresshCycle(UserSession session, SharedMsg msg);
	void ReqSelectCycleDiff(UserSession session, SharedMsg msg);
	void ReqCycleBattleInfo(UserSession session, SharedMsg msg);

	void ReqVSBattleBegin(UserSession session, SharedMsg msg);
	void ReqVSBattleReady(UserSession session, SharedMsg msg);
	void ReqVSBattleOpt(UserSession session, SharedMsg msg);
	
	void ReqBioHeroInfo(UserSession session, SharedMsg msg);	
	void ReqBioHeroStepDone(UserSession session, SharedMsg msg);

	void ReqRaffleHero(UserSession session, SharedMsg msg);
	void ReqRaffleHeroTimes(UserSession session, SharedMsg msg);

	void ReqMailInfo(UserSession session, SharedMsg msg);
	void ReqMailDetail(UserSession session, SharedMsg msg);
	void ReqDeleteMail(UserSession session, SharedMsg msg);
	void ReqGetMailAward(UserSession session, SharedMsg msg);

	void ReqBanquetInfo(UserSession session, SharedMsg msg);
	void ReqBanquetHero(UserSession session, SharedMsg msg);

	void ReqAddFriend(UserSession session, SharedMsg msg);
	void ReqFriendsInfo(UserSession session, SharedMsg msg);
	void ReqRemoveFriend(UserSession session, SharedMsg msg);

	void ReqPrepareBattle(UserSession session, SharedMsg msg);
	void ReqWorldWar(UserSession session, SharedMsg msg);
	void ReqWorldBattle(UserSession session, SharedMsg msg);

	void ReqWorldWarTopList(UserSession session, SharedMsg msg);

	void ReqTeamInfo(UserSession session, SharedMsg msg);
	void ReqSetTeam(UserSession session, SharedMsg msg);

	void ReqClusterInfo(UserSession session, SharedMsg msg);
	void ReqAllCluster(UserSession session, SharedMsg msg);
	void ReqCreateCluster(UserSession session, SharedMsg msg);
	void ReqJoinCluster(UserSession session, SharedMsg msg);
	void ReqExitCluster(UserSession session, SharedMsg msg);
	void ReqClusterDetailInfo(UserSession session, SharedMsg msg);
	void ReqSetClusterAttr(UserSession session, SharedMsg msg);

	void ReqGeneralSoulExchangeHero(UserSession session, SharedMsg msg);
	void ReqWriteSkillBook(UserSession session, SharedMsg msg);

	void ReqCityItem(UserSession session, SharedMsg msg);
	void ReqCityHero(UserSession session, SharedMsg msg);

	void ReqResourceInfo(UserSession session, SharedMsg msg);
	void ReqResourceDetailInfo(UserSession session, SharedMsg msg);

	void ReqEnterWorldWar(UserSession session, SharedMsg msg);
	void ReqWorldWarStatus(UserSession session, SharedMsg msg);
	void ReqWorldWarMove(UserSession session, SharedMsg msg);
	void ReqWorldWarDefence(UserSession session, SharedMsg msg);
	void ReqWorldWarDefenceTarget(UserSession session, SharedMsg msg);
	void ReqLeaveWorldWar(UserSession session, SharedMsg msg);

	//void SendUserAllResource(CUser *user);

	void SendCityItem(CUser *user, uint32_t cityId);
	void SendCityHero(CUser *user, uint32_t cityId);
	void SendHeroClassFri(CUser *user, uint32_t heroClass);
	void SendHeroClassGeneralSoul(CUser *user, uint32_t heroClass);
	void SendUserLearnVsCD(CUser *user, uint32_t heroClass);

	void AddHero(NetMsg::GiveAward *award, uint64_t heroId);

	bool RepleaseIllegalChar(std::string &str);
	bool HaveIllegalChar(std::string &str);
	
	void AddOnlineUser(CUser *);
	NetMsg::ChatAck *CreateChatAck(CUser *,NetMsg::ReqChat *req);

	void GetRandStr(std::string &str,uint32_t strLen);

	void SelectHero(CUser *user, uint32_t selPos);
	
	void DelHero(CUser *user,CHeroInst *hero);
	void UpdateHero(CUser *user, CHeroInst *hero);

	void CopyUser(CUser *in, NetMsg::PlayerData *outData);
	
	void UseHeroExp(CUser *user, CHeroInst *hero, CItemInst *item,uint32_t itemNum);
	void UseHeroSkill(CUser *user,CHeroInst *hero, CItemInst *item);
	bool UseHeroGift(CUser *user, uint32_t heroClass, CItemInst *item, uint32_t itemNum);

	void UpdateCycBattle(ServerPB::UserCycBattle &battle, time_t refreshTime);
	void CopyCycBattle(ServerPB::UserCycBattle &battle, NetMsg::CycleBattleInfoAck *netBattle);
	void CopyCycBattle(ServerPB::UserCycBattle &battle, NetMsg::CycleBattleAck *netBattle);
	void InitCycBattle(CUser *user, time_t refTime);
	void AddCycBattle(CUser *user);
	void DelCycBattle(CUser *user);
	uint32_t CycBattleEnd(CUser *user);//return awardId
	time_t GetCycBattRefTime();

	void AddAdvEventBattle(CUser *user,NetMsg::AdvantureEvent &event, AdEventData &data);
	void EndAdvEventBattle(CUser *user,CUserBattle *battle);
	void SendAdvEventState(CUser *user, uint32_t eventId,uint32_t state);
	void CalculateAdvInfo(CUser *user, NetMsg::AdvantureInfoAck &advInfo);

	void CalculateRunPer(CUser *user, NetMsg::RunPathAck *runAck, int dt);

	bool CalculateRunPos(NetMsg::RunPathAck *runAck, int dt,int speed);//if is arrived target return true

	void SetTeamCurCity(ServerPB::UserAddData *addData,uint32_t heroPos,int city);
	void SetTeamPath(ServerPB::UserAddData *addData, uint32_t heroPos, NetMsg::RunPathAck *path, CUser* user);
	void SetTeamUpdateTime(ServerPB::UserAddData *addData, uint32_t heroPos,time_t now);

	void ReqTeamRunPath(UserSession session, SharedMsg msg);
	void SetTeamRunPath(UserSession session, SharedMsg msg);
	void ReqTeamArrivedTar(UserSession session, SharedMsg msg);

	void CalculateTeamRun(ServerPB::UserAddData *addData, uint32_t heroPos);
	void Test(UserSession, SharedMsg);
	
	CSocket *m_socket;
	CRsaCode *m_rsa;
	COnlineUser *m_onlineUser;

	std::list<std::string> m_badWords;

	std::unordered_map<std::string, uint32_t> m_userSessions;

	enum NewGuideStep
	{
		NGSelectCity = 2,
		NGSelectHero = 3,
	};
};
