#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
#include "self_typedef.h"
#include <string>
#include <list>
#include <vector>
#include <boost/thread.hpp>
#include "hash_table.h"
#include "user.h"
#include "fight.h"
#include "net_msg.h"
using namespace std;

class CNpcManager;
class CUser;
class CUserTeam;
class CNetMessage;
class CSocketServer;
class COnlineUser;
class COnlineUser;
class CMonsterManager;
class CFightManager;
class CCallScript;

struct SJumpTo
{
    uint8 x;
    uint8 y;
    uint8 face;
    uint16 sceneId;
};
struct SPoint
{
    uint8 x;
    uint8 y;
};

struct SVisibleMonster
{
    uint8 x;
    uint8 y;
    uint8 face;
    uint8 type;
    uint16 id;
    uint16 pic;
    uint8 flag;//0正常，1战斗中
};

struct SJiFenUser
{
    uint32 roleId;
    string name;
    short jifen;
};
    
const uint16 SCENE_JIANYU   = 299;
const uint16 BANG_PAI_SCENE_ID  = 300;
const uint16 LAN_RUO_DI_GONG_ID       = 302;
const uint16 GUIYU_SCENE_ID = 310;
const int MAX_GUI_YU_LAYER  = 3;
const int MAX_GUI_YU_NUM    = 80;
const uint16 FUBEN_ID_BEGIN = 1000;//副本id从1000起始
const int GUI_YU_MAX_MONSTER_NUM = 10;
const int LANRUO_MONSTER_NUM = 5;

const uint16 MATCH_SCENE_ID = 320;
const int MATCH_SCENE_NUM   = 4;

const int MAX_TEAM_MEMBER = 3;

const int MATCH_BEGIN_TIME  = 22;//个人擂台开始时间
const int TEAM_MATCH_BEGIN_TIME  = 23;//组队擂台开始时间
const int MATCH_TIME        =  3300;
//const int MATCH_END_TIME    = 12;//23;//比赛结束时间
const int BANG_ZHAN_BEGIN_TIME = 21;
const int BANG_ZHAN_END_TIME = 22;

class CScene
{
public:
    CScene(uint16 id,uint16 mapId,const char *name,char *monsters);
    CScene(CScene&);
        
    ~CScene();
    bool GetJumpPoint(uint8 x,uint8 y,SJumpTo*&);
    bool HaveNpc(uint8 x,uint8 y);
    
    void Exit(CUser*);
    list<uint16> *GetNpcList()
    {
        return &m_npcList;
    }
    void SetFightType(uint8 type)
    {
        m_fightType = type;
    }
    uint8 GetFightType()
    {
        return m_fightType;
    }
    void SetId(int id)
    {
        m_id = id;
    }
    int GetId()
    {
        return m_id;
    }
    uint16 GetMapId()
    {
        return m_mapId;
    }
    void SetMapId(int mapId)
    {
        m_mapId = mapId;
    }
    const char *GetName()
    {
        return m_name.c_str();
    }
    uint8 *GetMonsters()
    {
        return m_monsters;
    }
    uint8 GetMonsterNum()
    {
        return m_monsterNum;
    }
    bool CreateTeam(CUser*,uint32 request = 0);
    void AskForJoinTeam(CUser*,uint32 headId);
    void GetTeamList(CUser*,uint8 page);
    void AllowJoinTeam(CUser*,uint32 member);
    void GetAskForUserList(CUser*);
    void GetTeamMembers(CUser*);
    void LeaveTeam(CUser*);
    void TempLeaveTeam(CUser *pUser);
    void ReturnTeam(CUser *pUser);
    void NotAllowJoin(CUser*,uint32 member);
    void DelTeamMember(CUser*,uint32 member);
    
    void UserMove(CUser *,uint8 dir,uint8 step);
    
    void ChangeScene(CUser *pUser,CScene *pOldScene);
    void SendUserList(CUser *pUser);
    void MeetEnemy(ShareUserPtr pUser);
    void PlayerPk(ShareUserPtr pUser,uint32 roleId,bool yaoqing);
    void MakeNearPlayerList(CUser *pUser,uint8 page,CNetMessage &msg);
    void MakeCloseUser(CUser *pUser,CNetMessage &msg,int bId,int xzBangId);
    
    void PlayerAskForMatch(ShareUserPtr pUser,uint32 roleId);
    bool FindFacePlayer(CUser *pUser,ShareUserPtr &find);
    void AcceptAskForMatch(ShareUserPtr pUser,bool accept,uint32 roleId);
    
    void SceneChat(CNetMessage &msg);
    void TeamChat(uint32 teamId,CNetMessage &msg);
    
    void UpdateUserInfo(CUser *pUser);
    void EnterFight(CUser *pUser,int monsterId,int level);
    
    int GetTeamMemNum(uint32 temdId);
    CUser *GetTeamMember1(uint32 teamId);
    CUser *GetTeamMember2(uint32 teamId);
    void ZhaoTao1(CUser *pUser);
    void ZhaoTao2(CUser *pUser);
    void ZhaoTao3(CUser *pUser);
    
    void ThreeKuLou(CUser *pUser);
    
    void ShiMenFight(CUser *pUser);
    
    void HumanNpcFight(CUser *pUser,int npcId);
    void DengLongGuiFight(CUser *pUser);
    void BaiYueFight(CUser *pUser);
    
    void LouLuoFight(CUser *pUser);
    void WuNianFight(CUser *pUser);
    void WuXinFight(CUser *pUser);
    void ShuiGuiFight(CUser *pUser);
    
    CCallScript *GetScript()
    {
        return m_pScript;
    }
    
    void NotInTeamUser(uint8 page,CNetMessage &msg);
    
    void InitNpcPoint(SPoint *pPoint,uint8 num);
    void AddNpc(uint16 id,bool sendMsg = false);
    void AddNpc(uint16 tmplId,uint8 x,uint8 y,uint8 direct);
    void ModifyNpcPos(uint16 id,uint8 x,uint8 y);
    void DelNpc(uint16 id);
    
    void DelNpc(uint8 x,uint8 y);
    SNpcInstance *FindNpc(uint8 x,uint8 y);
    SNpcInstance *FindNpc(uint16 id);
    
    static const int ONE_PAGE_MAX_NUM = 20;
    static const int SEND_MAX_USER_NUM = 50;
    void SetFightStep(uint8 step)
    {
        m_fightStep = step;
    }
    uint8 GetFightStep()
    {
        return m_fightStep;
    }
    
    void EpisodeBattle1(CUser *pUser);
    void EpisodeBattle2(CUser *pUser);
    void EpisodeBattle3(CUser *pUser);
    void EpisodeBattle4(CUser *pUser);
    void EpisodeBattle5(CUser *pUser);
    void EpisodeBattle6(CUser *pUser);
    void EpisodeBattle7(CUser *pUser);
    void EpisodeBattle8(CUser *pUser);
    void EpisodeBattle9(CUser *pUser);
    void EpisodeBattle10(CUser *pUser);
    void EpisodeBattle11(CUser *pUser);
    void EpisodeBattle12(CUser *pUser);
    void EpisodeBattle13(CUser *pUser);
    void EpisodeBattle14(CUser *pUser);
    void EpisodeBattle15(CUser *pUser);
    void EpisodeBattle16(CUser *pUser);
    void EpisodeBattle17(CUser *pUser);
    void EpisodeBattle18(CUser *pUser);
    void EpisodeBattle19(CUser *pUser);
    void EpisodeBattle20(CUser *pUser);
    void EpisodeBattle21(CUser *pUser);
    void WenTianBattle(CUser *pUser);
    void OpenPackBattle1(CUser *pUser);
    void OpenPackBattle2(CUser *pUser);
    
    void FightCoupleTrial(CUser *pUser);
    
    ShareMonsterPtr CreateQCFightMonster(uint32 monsterId,EMonsterType type,uint8 level,const char *name,
        int xue,int liliang,int lingli,uint8 xiang,int minjie=0);
        
    void QiChongBattle1(CUser *pUser);
    void QiChongBattle2(CUser *pUser);
    void QiChongBattle3(CUser *pUser);
    void QiChongBattle4(CUser *pUser);
    void QiChongBattle5(CUser *pUser);
    void QiChongBattle6(CUser *pUser);
    void QiChongBattle7(CUser *pUser);
    void QiChongBattle8(CUser *pUser);
    void QiChongBattle9(CUser *pUser);
    void QiChongBattle10(CUser *pUser);
    void QiChongBattle11(CUser *pUser);
    void QiChongBattle12(CUser *pUser);
    void QiChongBattle13(CUser *pUser);
    void QiChongBattle14(CUser *pUser);
    
    void LangBattle1(CUser *pUser);
    void LangBattle2(CUser *pUser);
    void LangBattle3(CUser *pUser);
    void LangBattle4(CUser *pUser);
    void LangBattle5(CUser *pUser);
    void LangBattle6(CUser *pUser);
    void LangBattle7(CUser *pUser);
    void LangBattle8(CUser *pUser);
    void LangBattle9(CUser *pUser);
    void LangBattle10(CUser *pUser);
    void LangBattle11(CUser *pUser);
    void LangBattle12(CUser *pUser);
    
    bool YeWaiShiYao(CUser *pUser);
    void ShiYaoYW1(CUser *pUser);
    void ShiYaoYW2(CUser *pUser);
    void ShiYaoYW3(CUser *pUser);
    void ShiYaoYW4(CUser *pUser);
    void ShiYaoYW5(CUser *pUser);
    void ShiYaoYW6(CUser *pUser);
    void ShiYaoYW7(CUser *pUser);
    void ShiYaoYW8(CUser *pUser);
    void ShiYaoYW9(CUser *pUser);
    void ShiYaoYW10(CUser *pUser);
    
    //英雄
    void ShiYaoYX1(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX2(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX3(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX4(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX5(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX6(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX7(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX8(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX9(CUser *pUser,int type = CFight::EFTScript);
    void ShiYaoYX10(CUser *pUser,int type = CFight::EFTScript);
    
    //普通
    void ShiYaoPT1(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT2(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT3(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT4(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT5(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT6(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT7(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT8(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT9(CUser *pUser,int type = CFight::EFTScript); 
    void ShiYaoPT10(CUser *pUser,int type = CFight::EFTScript);
    
    void NuYanQiLing(CUser *pUser);
    
    void XianBing1(CUser *pUser);
    void XianBing2(CUser *pUser);
    
    void YaoBing1(CUser *pUser);
    void YaoBing2(CUser *pUser);
    
    void MenPaiChuanSong(CUser *pUser);
    void QingLongBaoBao(CUser *pUser);
    void KuLouDaWang(CUser *pUser);
    //void LouLuo(CUser *pUser);
    
    //仙人（剧情）
    void JuQingXianRen(CUser *pUser);
    
    //麒麟（剧情）
    void JuQingQiLing(CUser *pUser);
    
    //冥妖王（剧情）
    void JuQingMingYao(CUser *pUser);
    
    void Mission21Boss(CUser *pU,uint16 monsterId);
    //void LangBattleDuE(CUser *pUser);
    
    //找出妖孽
    void FindYaoNieBattle(CUser *pUser);
    //击败狂兽
    void JiBaiKSBattle(CUser *pUser);
    
    //寻宝战斗
    void XunBaoFight(CUser *pUser);
    
    void ChuShiFight(CUser *pUser);
    void QiLinFightYao(CUser*);
    void QiLinFightXian(CUser*);
    
    void InitPathInfo();
    void SendPathInfo(int sock);
    void GetUserList(list<uint32> &userList);
    void SetX(uint8 x)
    {
        m_x = x;
    }
    void SetY(uint8 y)
    {
        m_y = y;
    }
    uint8 GetX()
    {
        return m_x;
    }
    uint8 GetY()
    {
        return m_y;
    }
    uint8 GetTeamMem(uint32 teamId,uint32 members[MAX_TEAM_MEMBER]);
    
    void AddVisibleMonster(SVisibleMonster &);
    void ClearVisibleMonster();
    bool FindVisibleMonster(uint16 id,SVisibleMonster &,uint8 flag);
    void DelVisibleMonster(uint16 id);
    void NianShouBattle(CUser *,uint16 monsterId);
    void ShiYaoFight(CUser*,uint16 monsterId);
    //年兽卷土重来
    void NianShouBattle(ShareUserPtr);
    
    //鬼域战斗
    void GuiYuFight(ShareUserPtr pUser,uint16 monsterId);
    
    void YeZhuFight(ShareUserPtr pUser,uint16 monsterId);
    void QianNianJiangShiFight(ShareUserPtr pUser,uint16 monsterId);
    void ThreeSister(ShareUserPtr pUser,uint16 monsterId);
    
    int GetUserNum();
    void AddGuiYuMonster();
    void AddLanRuoMonster(bool killMonster = false);
    void AddShiYao(bool killMonster = true);
    
    bool InGuiYu();
    bool InMatchScene();
    
    bool GetCanWalkPos(uint8 &x,uint8 &y);
    void Match();
    void SetUserJiFen(uint32 roleId,char *name,short jifen);
    
    void AddJumpPoint(uint8 x,uint8 y,uint8 toX,uint8 toY,uint16 sceneId);
    
    
    //二阶段
    void LanRuoBattle(CUser *pUser);
    //三阶段
    void LanRuoBattle3(CUser *pUser);
    //四阶段
    void LanRuoBattle4(CUser *pUser);
    //五阶段
    void LanRuoBattle5(CUser *pUser);
    
    void DiaoXiangFight(CUser *pUser,int id);
    
    void Clear();
    int GetState();
    void SetState(int state);
    
    bool m_usedFuBen;
    time_t m_startTime;
    
    void SetGroupId(int gId)
    {
        m_groupId = gId;
    }
    int GetGroupId()
    {
        return m_groupId;
    }
    int GetJiFen(int bId);
    int GetKuang(int bId);
    int GetOtherJiFen(int bId);
    int GetOtherKuang(int bId);
    void SetJiFen(int bId,int jifen);
    void SetKuang(int bId,int kuang);
    void SetOtherJiFen(int bId,int jifen);
    void SetOtherKuang(int bId,int kuang);
                        //魅影数量，持续时间
    void StartMeiYing(int bId,int num,int timeOut);
    int GetOtherLeftMeiYing(int bId);
    int GetMeYingLeftTime(int bId);
    void SetDiaoXiangBang(int bId,int diaoxiangId);
    int GetDiaoXiangBang(int id)
    {
        if(m_diaoXiangBang.find(id) != m_diaoXiangBang.end())
            return m_diaoXiangBang[id];
        return 0;
    }
    time_t GetDiaoXiangTime(int id)
    {
        if(m_zhanDiaoXiangTime.find(id) != m_zhanDiaoXiangTime.end())
            return m_zhanDiaoXiangTime[id];
        return 0;
    }
    void BangZhanJiFen();
    void SetNewHead(CUser *pUser,uint32 newHead);
    char *GetMatchPaiMing();
    void SetShiYaoLevel(int state)
    {
        m_state = state;//1普通，2英雄
    }
    //void ThieveObject(CUser *pUser,CNetMessage &msg);
    //拦路虎战斗
    void LanLuoHu(CUser *pUser,uint16 tmplId);
    
    void DiaoYuFight(CUser *pUser);
    
    int GetUserJiFen(uint32 roleId);
private:
    //暴走任务
    void BaZouMission(CUser *pUser);
    
    bool LeaveSceneTeam(uint32 teamId,CUser *pUser);
    
    void GroupClear(int sceneId,uint8 x,uint8 y);
    void GroupSetState(int state);
    
    void Init();
    void MeiYingFight(ShareUserPtr &pUser);
    void AddMeiYingMonster(ShareFightPtr &pFight,uint8 num,uint8 begin,uint8 level);
    ShareMonsterPtr CreateMeiYing(uint8 level);
    ShareMonsterPtr CreateDiaoXiangMonster(uint8 level);
    
    void SetOffLineTitle(uint32 roleId,uint8 title);
    
    //bool FindMatchTeam(uint32 teamId,CUserTeam *,vector<uint32> *pTeamList);
    void FindMatchUser(vector<uint32> &userList);
    
    void AddGuYuFightMonster(ShareFightPtr pFight,uint8 num,uint8 level);
    
    void NoLockChangeScene(CUser *pUser,CScene *pOldScene);
    
    bool InitEpisodeBattle(CUser *p,ShareFightPtr &pFight,ShareUserPtr &pUser);
    uint8 AddEpisodeBattleMonster(ShareFightPtr &fight,uint8 ind,uint8 pos,uint8 level = 0,SMonsterInst **ppMonster = NULL);
    uint8 BeginEpisodeBattle(ShareFightPtr &fight,ShareUserPtr &pUser);
    
    ShareMonsterPtr CreateMonster(CUser *pUser,bool &baobao);
    
    void AddMonsterToFight(ShareFightPtr &pFight,uint8 num,uint8 begin,CUser *pUser);
    uint8 AddTeamToFight(ShareFightPtr &pFight,CUserTeam *pTeam,uint8 begin);
    uint8 AddUserToFight(ShareFightPtr &pFight,ShareUserPtr &pUser,uint8 *pTeamMemNum = NULL);
    
    //fightType=0 召讨使任务，1 师门任务
    void AddScriptFight(ShareFightPtr &pFight,uint8 num,uint8 begin,uint8 level,
                        uint8 toMuBeiLv,uint8 louLuoBeiLv,int touMuId,int louLuoId,
                        string touMuName = "",uint8 fightType=0);
    
    void ForEachTeamMember(uint32 teamId,boost::function<void(ShareUserPtr)> f);
    void ForEachUser(boost::function<void(ShareUserPtr)> f);
    
    void Enter(CUser*);
    void InsertJumpPoint(uint8 x,uint8 y,SJumpTo*);
    bool MakeTeamList(uint32 id,CUserTeam *pTeam,uint8 page,CNetMessage *msg,uint8 *teamNum,uint8 *tolNum);
    void BroadcastMsg(CNetMessage &msg,bool chatMsg = false);
    void BroadcastMsgExcept(CNetMessage &msg,CUser *pUser);
    void BroadcastMsgExceptSameTeam(CUser *pUser,CNetMessage &msg);
    
    
    uint8 m_x;
    uint8 m_y;
    
    uint8 m_fightStep;
    int m_id;
    uint16 m_mapId;
    int m_groupId;
    
    string m_name;
    uint8 m_fightType;
    uint8 m_killMonsterNum;
    bool m_addJump;
    SPoint m_jumpPoint;
    SJumpTo m_jumpToPoint;
        
    bool m_inBangZhan;
    bool m_isPaiMing;
    time_t m_matchBegin;
    time_t m_jsBangZhanJF;//计算帮战积分时间
    time_t m_sendJFGongGao;
    int m_state;
    
    const static int MAX_JIFEN_USER_NUM = 3;
    
    const static int MAX_PAIMING_INFO = 128;
    int m_jifens[2];
    int m_kuangs[2];
    int m_bangpais[2];
    int m_meiYingNum[2];//魅影数量
    time_t m_meiYingStartTime[2];//魅影开始时间
    int m_meiYingTimeOut[2];//魅影持续时间
    map<int,int> m_diaoXiangBang;
    map<int,time_t> m_zhanDiaoXiangTime;
    
    //SJiFenUser m_jifenUsers[MAX_JIFEN_USER_NUM];
    vector<SJiFenUser> m_jifenUsers;
    
    list<uint16> m_npcList;
    list<uint32> m_userList;
    vector<SPoint> m_canWalkPos;
    
    CNetMessage m_pathInfo;
    
    list<SNpcInstance*> m_dynamicNpc;
    vector<SPoint> m_dynamicNpcPoint;
    
    list<SVisibleMonster> m_visibleMonsters;
    
    const static int MAX_MONSTER_NUM = 10;
    uint8 m_monsters[MAX_MONSTER_NUM];
    uint8 m_monsterNum;
    //索引为x<<8|y
    CHashTable<int,SJumpTo*> m_jumpTo;
    CHashTable<uint32,CUserTeam*> m_userTeams;
    
    boost::recursive_mutex m_mutex;
    
    CSocketServer &m_socketServer;
    COnlineUser &m_onlineUser;
    CMonsterManager &m_monsterManager;
    CFightManager &m_fightManager;
    
    uint16 m_curVisibleId;
    CCallScript *m_pScript;
};

class CSceneManager
{
public:
    CSceneManager();
    //~CSceneManager();
    CScene *FindScene(int id);
    CScene *FindScene(int mapId,int groupId);
    bool Init();
    
    CScene *FindMarryHall(int id);
    
    CScene *GetMarryHall(int id);
    //获得帮派场景
    CScene *GetBangPaiScene(int id,int bangPaiId);
    
    //兰若地宫（场景id302）
    CScene *GetLanRuoDiGong();
    CScene *GetFuBen(int scendId);
    CScene *GetGuiYu(int level);
    CScene *GetBangZhanScene(int bId,CScene **ppScene);
    //CScene *Get
    //删除帮派场景
    void DelBangPaiScene(int bangPaiId);
    void Timer();
    void GetGroupScene(int groupId,list<int> &sceneList);
private:
    bool FindGroupScene(int id,CScene *pScene,int groupId,list<int> *sceneList);
    
    bool FindMapGroupScene(int id,CScene *pScene,int mapId,int groupId,
                                CScene **ppScene);
    bool FindMapSceneList(int id,CScene *pScene,int mapId,list<CScene*> *pSceneList);
    time_t m_matchRunTime;
    const static int RUN_MATCH_SPACE = 120;
    
    int m_curFuBenId;
    list<CScene*>   m_fuBenScene;
    list<CScene*>   m_gcLanRuo;
    list<CScene*>   m_marryHall;
    
    CHashTable<int,CScene*> m_sceneList;
    
    CHashTable<int,CScene*> m_bangPaiScene;
    boost::mutex m_bpMutex;
    
    bool m_isInit;
    CNpcManager &m_npcManager;
};

void GetChengZhang(uint8 level,int &xue,int &fali,int &sudu,int &wugong,int &fagong);

#endif
