#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <stdio.h>
#include <stdint.h>
#include <google/protobuf/message.h>
#include <string>
#include "Award.h"
#include "NetMsg.pb.h"
#include <vector>
#include "session.h"

class CUser;
class CSocket;
class COnlineUser;
class CHeroInst;
class CItemInst;
class CUserMail;
class CResourceInst;

class CUtility
{
public:
	static void Init();

	static size_t StrlenUtf8(const char *);

	static int SplitLine(char **templa, int templatecount, char *pkt,char space);

	static int64_t GetMicroseconds();

	static void MsgToStr(google::protobuf::Message *msg, std::string &str);

	static bool StrToMsg(const char *str, google::protobuf::Message *msg);

	//字符串转换十六进制
	static int UnHexify(unsigned char *obuf, const char *ibuf);

	//十六进制转化字符串
	static void Hexify(unsigned char *obuf, const unsigned char *ibuf, int len);

	template<typename Type>
	static Type RandomSel(const std::vector<Type>& vecs)
	{
		uint32_t size = vecs.size();
		assert(size > 0);
		uint32_t pos = rand() % size;
		return vecs[pos];
	}

	static int RandomInt(int min, int max);

	static uint32_t RandomChance(const std::vector<uint32_t> &changes);//return sel pos

	static void CopyAwards(const std::vector<CAward::AwardItem> &inAwards, NetMsg::Award *outAward);
	static void CopyAwards(const NetMsg::Award *inAwards, std::vector<CAward::AwardItem> &outAward);

	static void GetAward(uint32_t awardId, CUser *user, NetMsg::Award *netAward);

	//升到本级需要的经验
	static void AddExp(std::vector<uint32_t> &leveupExp,uint32_t &exp,uint32_t &level);

	static void UpdateUserInfo(CUser *user, uint64_t mask);

	static CUser *GetOnlineUser(uint32_t userId);

	static uint32_t RandOtherCity(CUser *user);

	static void CopyHero(CHeroInst * in, NetMsg::HeroAck * out);
	static void CopyResource(CResourceInst * in, NetMsg::ResourceInfoAck * out);

	static CHeroInst *AddUserHero(CUser *user, uint32_t heroId, bool isExchangeGeneralSoul = false);
	static CResourceInst *AddUserResourceField(CUser *user, uint32_t fieldId);

	static void SendMsg(CUser *user,SharedMsg &msg);

	static void UseBoxItem(CUser *user, CItemInst *item, uint32_t num);
	static bool HaveUserAward(std::vector<CAward::AwardItem> &awards);
	static bool HaveHeroAward(std::vector<CAward::AwardItem> &awards);
	static void AddUserAwards(CUser *user, std::vector<CAward::AwardItem> &awards,uint32_t heroClass = 0);
	static void AddUserAwards(CUser *user, const NetMsg::Award *netAward);
	static void AddHeroAwards(CUser *user, CHeroInst *hero, std::vector<CAward::AwardItem> &awards);
	static bool IsHeroAward(int type);

	static bool AddCityItem(uint32_t cityId, uint32_t itemId, uint32_t num);
	static bool AddUserItem(CUser *user, uint32_t itemId, uint32_t num);
	static bool AddCityHero(uint32_t cityId, const std::string & heroName, uint32_t heroLevel);
	static void AddAwards(CUser *user, std::vector<uint32_t> &awardIds);
	static void GetAwards(CUser *user, std::vector<uint32_t> &awardIds, std::vector<CAward::AwardItem> &awards);
	static void GetRandomVec(std::vector<uint32_t> & val);
	static uint32_t GetRandomOneFromVector(std::vector<uint32_t> & vec);

	static void SendResTeamRunPath(CUser* user, uint32_t teamPos);

	static uint32_t MakeMask(uint32_t h, uint32_t l)
	{
		return (h << 16) | l;
	}

	enum CurrencyType {
		COIN = 0,
		GOLD,
		REPUTATION,
		CONTRIBUTION,
		PROVISIONS,
	};

	static bool DelPrice(CUser *user, uint32_t priceType, uint32_t price);

	static void AddExploit(CUser *user, int exploit);
	static void AddCoin(CUser *user, int coin);
	static void AddGold(CUser *user, int add);
	static void AddExp(CUser *user, int exp);
	static void AddStamina(CUser *user, int stamina);

	static std::vector<uint32_t> ConvertToArray(char *str);

	static void SendSysMail(CUser *user,const char *name, const char *title, const char *context, uint32_t awardId, NetMsg::MailType type);
	static void SendOflineSysMail(uint32_t userId, const char *name,const char *title, const char *context, uint32_t awardId, NetMsg::MailType type);
	static void SendUserAllResource(CUser *user);

	static void CopyMail(CUser *user,CUserMail *from, NetMsg::MailData *to);

	static void SendWorldWarStatus(CUser *user, SharedMsg &ackMsg);

	static std::string GetNick(const char *str);
	//static bool CUtility::CalculateRunPosCommon(NetMsg::RunPathAck *runAck, int dt, int speed);
	//static void CUtility::SendResourceTeamRunPath(CUser* user);
private:
	static CSocket *m_socket;
	static COnlineUser *m_onlineUser;
};

#endif
