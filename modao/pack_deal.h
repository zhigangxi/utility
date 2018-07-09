#ifndef _PACK_DEAL_H_
#define _PACK_DEAL_H_
#include "hash_table.h"
#include "self_typedef.h"
#include "bangpai.h"
#include "item.h"
#include "database.h"
#include <list>
#include <vector>
#include <map>
#include <boost/thread.hpp>
using namespace std;

class CNetMessage;
class CSocketServer;
class COnlineUser;
class CNpcManager;
class CSceneManager;
class CUser;
class CFightManager;
class CMonsterManager;
class CScene;

const int ANSWER_QUESTION_SAPCE = 2*60*60;
const int USE_QIAN_NENG = 100;
const int USE_TILI_CHAT = 20;

class CPackageDeal
{
public:
    CPackageDeal();
    void UserLogout(CUser*);
    void SetVerInfo(string &version,string &forceUpdate,string &updateMsg,string &updateUrl);
private:
    void UserLogin(CNetMessage*,int sock);
#ifdef QQ
    string m_qqLoginServer;
    int m_qqLoginPort;
    int m_qqServerSocket;
    struct SQQLoginUser
    {
        string name;
        int sock;
        time_t loginTime;
    };
    list<SQQLoginUser> m_qqLoginUserList;
    boost::mutex       m_qqMutex;
    void AddQQLoginUser(string &name,int sock);
    void DelQQLoginUser(int sock);
    void QQLogin(string &name,string &password,int sock);
    int FindQQLoginUser(string &name);
    void QQServerLogin(CNetMessage*,int);
#endif
    void MDLogin(string &name,string &password,int sock);

    
    void RegUser(CNetMessage*,int sock);
    void CreateRole(CNetMessage *,int);
    void SelectRole(CNetMessage *,int);
    void GetEquipment(CNetMessage *,int);
    void GetPackage(CNetMessage *,int);
    void RoleMove(CNetMessage *,int);
    void OpenNpcInteract(CNetMessage *,int);
    void NpcInteract(CNetMessage *,int);
    void UpdateChar(CNetMessage *,int);//加属性点、向性点……
    void EquipItem(CNetMessage *,int);//装备物品
    void GetItemInfo(CNetMessage *,int);//获得物品信息
    void UserTeam(CNetMessage *,int);//组队
    void GetPlayerInfo(CNetMessage *,int);//查询玩家信息
    void PlayerPk(CNetMessage *,int);//玩家pk
    void PlayerMatch(CNetMessage *,int);//玩家切磋
    void NearPlayerList(CNetMessage *,int);//附近玩家列表
    void QueryPet(CNetMessage *,int);//查询宠物列表
    void PetOption(CNetMessage *,int);//宠物操作
    void GetMissionList(CNetMessage *,int);//任务列表
    void GetMissionInfo(CNetMessage *,int);//任务信息
    void GetUserSkill(CNetMessage *,int);//
    void GetPetSkill(CNetMessage *,int);//
    void UserChat(CNetMessage *,int);
    void GetOtherUserItemInfo(CNetMessage*msg,int sock);//获得其他玩家物品信息
    void GetHotList(CNetMessage*msg,int sock);
    void AddHot(CNetMessage*msg,int sock);
    void DelHot(CNetMessage*msg,int sock);
    void OnlineMail(CNetMessage*msg,int sock);
    void QueryMail(CNetMessage*msg,int sock);
    void UserShop(CNetMessage*msg,int sock);
    void BuyShopItem(CNetMessage*msg,int sock);
    void PutItemToShop(CNetMessage*msg,int sock);
    void ShopItemInfo(CNetMessage*msg,int sock);
    void GetUserDetail(CNetMessage*msg,int sock);
    void SafeTrade(CNetMessage*msg,int sock);
    void CloseUser(CNetMessage*msg,int sock);//附近玩家
    void BangPai(CNetMessage*msg,int sock);//帮派
    void UpdatePackage(CNetMessage*msg,int sock);//背包物品
    
    void GetEShopItem(CNetMessage*msg,int sock);//商城物品
    void BuyEShopItem(CNetMessage*msg,int sock);//购买商城物品
    void UserJumpOk(CNetMessage*msg,int sock);
    void DoObject(CNetMessage *pMsg,int sock);
    
    void GetNpcState(CNetMessage*msg,int sock);
    void QiangHuaEquip(CNetMessage*msg,int sock);
    void QueryQiangHua(CNetMessage*msg,int sock);
    void MakeBlueItem(CNetMessage*msg,int sock);
    void MakeGreenItem(CNetMessage*msg,int sock);
    void SelectGreenItemAttr(CNetMessage*msg,int sock);
    void XiangQian(CNetMessage*msg,int sock);
    
    void ChatChannel(CNetMessage*msg,int sock);
    
    void EasyUseYaoPin(CNetMessage*msg,int sock);
    
    void QuerySkillDesc(CNetMessage*msg,int sock);
    
    void SwitchInfo(CNetMessage *msg,int sock);
    void QueryPetInfo(CNetMessage *msg,int sock);
    void MyBangPai(CNetMessage *msg,int sock);
    void ChangeUserFace(CNetMessage *msg,int sock);
    void CancelTask(CNetMessage *msg,int sock);
    void ItemDesc(CNetMessage *msg,int sock);
    void Charge(CNetMessage *msg,int sock);
    void QuerySavePos(CNetMessage *msg,int sock);
    void QueryPath(CNetMessage *msg,int sock);
    void AvailableTask(CNetMessage *msg,int sock);
    void GetScenePos(CNetMessage *msg,int sock);
    void SpecChat(CNetMessage *msg,int sock);
    void UserAnswer(CNetMessage *msg,int sock);
    void MonsterBattle(CNetMessage *msg,int sock);
    void QueryScene(CNetMessage *msg,int sock);
    void QueryItem(CNetMessage *msg,int sock);
    void HeartBeat(CNetMessage *msg,int sock);
    void GetServerList(CNetMessage *msg,int sock);
    void GetTitle(CNetMessage *msg,int sock);
    void UseTitle(CNetMessage *msg,int sock);
    void SysMail(CNetMessage *msg,int sock);
    void CXGongGao(CNetMessage *pMsg,int sock);
    void ShiTu(CNetMessage *pMsg,int sock);
    void KaiJiaXiangQian(CNetMessage *pMsg,int sock);
    void FuQi(CNetMessage *pMsg,int sock);
    void DelRole(CNetMessage *pMsg,int sock);
    void CancelDelRole(CNetMessage *pMsg,int sock);
    void IgnoreList(CNetMessage *pMsg,int sock);
    void AddIgnore(CNetMessage *pMsg,int sock);
    void DelIgnore(CNetMessage *pMsg,int sock);
    void WWAgreeYaoQing(CNetMessage *pMsg,int sock);
    
    void GuanZhan(CNetMessage *pMsg,int sock);
    void LeaveGuanZhan(CNetMessage *pMsg,int sock);
    
    void GetNewUserGift(CNetMessage *pMsg,int sock);
    void GetNormalGift(CNetMessage *pMsg,int sock);
        
    void LearnSkill(CNetMessage *pMsg,int sock);
    //信使功能
    void XinShi(CNetMessage *pMsg,int sock);
    
    void YeWaiShiYao(CNetMessage *pMsg,int sock);
    
    //查询称号
    void QueryTextTitle(CNetMessage *pMsg,int sock);
    
    void UseTextTitle(CNetMessage *pMsg,int sock);
    //提供后台管理功能
    void ServerMgr(CNetMessage *pMsg,int sock);
    void WriteAdminLog(uint32 roleId,const char *fmt, ...);
    
    void BroadcastChat(CUser *pUser,CNetMessage *msg);
    
    void OnSockClose(int sock);
    
    //void ReadHots(CUser*,char *hots);
    
    void AddNotifyUser(uint32 userId,uint32 hotId);
    void DelNotifyUser(uint32 userId,uint32 hotId);
    void NotifyUser(CUser *,bool online);//上下线通知
    
    void MD5(string &str);
    void SendQuestion(CUser *pUser);
    void AskQuestion(CUser *pUser);
    //在pvp中注册用户，true成功，false失败
    bool RegShareUser(string &name,string &passwd);//,string &trueName,string &rcode);
#ifndef QQ
    uint32 LoginShareUser(string &name,string &passwd);
#endif
    
    void UserMoveOneStep(CUser *pUser,CScene *pScene,ShareUserPtr &ptr,uint8 x,uint8 y);
    struct EShopItem
    {
        uint16 id;
        uint8 level;
        int money;
        uint8 page;
    };
    std::vector<EShopItem> m_eShopItemList;
    
    std::map<uint16,string> m_skillDescMap;
    
    CHashTable<uint32,list<uint32>*> m_inOutNotify;
    boost::mutex    m_mutex;
    boost::mutex    m_dbLock;//登录数据库锁
    
    time_t          m_readSLTime;//read server list time
    boost::mutex    m_slLock;//server list lock
    CNetMessage     *m_pSLMsg;//server list msg
    
    CSocketServer &m_socketServer;
    COnlineUser &m_onlineUser;
    CNpcManager &m_npcManager;
    CSceneManager &m_sceneManager;
    CMonsterManager &m_monsterManager;
    CBangPaiManager &m_bangPaiMgr;
    
    string m_version;
    uint8 m_forceUpdate;
    string m_updateMsg;
    string m_updateUrl;

    CDatabaseSql m_loginDb;

    
    //CCallScript *m_pAdminScript;
};
#endif
