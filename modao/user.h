#ifndef _USER_H_
#define _USER_H_
#include "self_typedef.h"
#include "protocol.h"
#include "item.h"
#include "utility.h"
#include "skill.h"
#include "monster.h"
#include "npc_manager.h"
#include <string.h>
#include <string>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <bitset>
#include <map>
using namespace std;

const int MAX_PK_VAL = 1000;
const int SAVE_DATA_SPACE = 3600;
const int CHAT_SPACE = 120;
const int ANSWER_QUESTION_TIME = 60;
const int ADD_TILI_TIME_SPACE = 180;

const uint8 HOT_LIST_VERSION    = 0xf1;
const uint8 MAX_HOT_NUM = 80;

const uint8 MAX_EXT_PACKAGE_NUM = 18;
const uint8 MAX_EXT_BANK_NUM = 3*18;
const uint8 MAX_EXT_PET_BANK_NUM = 5;

class CScene;
class CNetMessage;
class CDatabaseSql;

//const int MAX_EXP = 10061438;
const int MAX_LEVEL = 90;

enum
{
    SSITItem = 1,
    ESITPet = 2,    
};

enum EEquipmentType
{
    EETMaoZi      = 0,// 帽子
    EETKuiJia     = 1,// 盔甲
    EETYaoDai     = 2,// 腰带
    EETXieZi      = 3,// 鞋子
    EETWuQi       = 4,// 武器
    EETXiangLian  = 5,// 项链
    EETYuPei      = 6,// 玉佩
    EETShouZhuo1  = 7,// 手镯
    EETShouZhuo2  = 8,// 手镯
};

enum EUserTitle
{
    EUTZhuangYuan   = 1,//状元
    EUTBangYan      = 2,// 榜眼
    EUTTanHua       = 3,// 探花
    EUTShiFu        = 4,// 师父
    EUT5            = 5,// 文状元
    EUT6            = 6,// 文榜眼
    EUT7            = 7,// 文探花
    EUT8            = 8,// 文举人
    EUT9            = 9,//TID=9 英雄 TID=10 MM  TID=11 GG
    EUT10            = 10,
    EUT11            = 11,
};

struct HotInfo
{
    uint32 hotId;
    uint16 hotVal;
};

bool CanAddShuXing(SItemTemplate *pItem,SItemInstance *pInst);

class CUser
{
public:
    CUser();
    ~CUser();
    void SetSock(int);
    int GetSock();
    void SetRoleId(uint32 id);
    void SetPos(uint8 x,uint8 y);
    uint32 GetUserId();
    uint32 GetRoleId();
    void SetUserId(uint32);
    void Move(uint8 direction,uint8 ste);//DIR=2 向上 DIR=4 向左 DIR=6 向右 DIR=8 向下
    void SetFace(uint8 face);
    uint8 GetFace();
    void GetPos(uint8 &x,uint8 &y);
    void GetFacePos(uint8 &x,uint8 &y);
    void EnterScene(CScene*);
    void EnterFuBen(uint16 sceneId);
    
    CScene *GetScene();
    /*CScene *GetOldScene()
    {
        return m_pOleScene;
    }*/
    uint16 GetSceneId();
    uint16 GetMapId();
    
    void SetRole(uint32 *roles);
    void AddRole(uint32 roleId)
    {
        for(uint8 i = 0; i < MAX_ROLE_NUM;i++)
        {
            if(m_role[i] == 0)
            {
                m_role[i] = roleId;
                return;
            }
        }
    }
    bool HaveRole(uint32 roleId)
    {
        for (int i = 0; i < MAX_ROLE_NUM; i++)
        {
            if(m_role[i] == roleId)
                return true;
        }
        return false;
    }
    
    //根据公式初始化人物属性
    void Init();
    
    uint32 GetFightId();
    uint8 GetFightPos();
    
    const char *GetName();
    uint8 GetSex();
    uint8 GetHead();
    uint8 GetXiang();
    uint8 GetX();
    uint8 GetY();
    uint8 GetLevel();
    uint32 GetExp();
    uint16 GetTiZhi();
    uint16 GetLiLiang();
    uint16 GetMinJie();
    uint16 GetLingLi();
    uint16 GetNaiLi();
    uint16 GetShuXinDian();
    uint16 GetJin();
    uint16 GetMu();
    uint16 GetShui();
    uint16 GetHuo();
    uint16 GetTu();
    uint16 GetXiangXinDian();
    int GetHp();
    int GetMp();
    int GetMaxHp();
    int GetMaxMp();
    
    uint32 GetStep();
    
    void SetName(const char *name);
    void SetSex(uint8 sex);
    void SetHead(uint8 head);
    void SetXiang(uint8 xiang);
    void SetLevel(uint8 level);
    void SetExp(uint32 exp);
    void SetTiZhi(uint16 tizhi);
    void SetLiLiang(uint16 liliang);
    void SetMinJie(uint16 minjie);
    void SetLingLi(uint16 lingli);
    void SetNaiLi(uint16 naili);
    void SetShuXinDian(uint16 shuxindian);
    void SetJin(uint16 jin);
    void SetMu(uint16 mu);
    void SetShui(uint16 shui);
    void SetHuo(uint16 huo);
    void SetTu(uint16 tu);
    void SetXiangXinDian(uint16 xiangxindian);
    void SetHp(int hp);
    void SetMp(int mp);
    //void SetStep(uint32 step);
    void SetFight(uint32 fightId,uint8 pos);
    bool CanMeetEnemy();
    
    void SetEquip(char *pEquip);
    void SetPackage(char *pPack);
    void SetBankItem(char *pBankItem);
    void GetBankItem(string &str);
    void MakeBankItemList(CNetMessage &msg);
    bool MoveItemToBank(uint8 packPos,uint8 num);
    bool MoveItemToPack(uint8 bankPos,uint8 num);
    
    void SetCall(int script,const char *call);    
    void SetCallScript(int script);
    void SetCallFun(const char *call);
    
    void MakeEquip(CNetMessage &msg);
    void MakeOtherEquip(CNetMessage &msg);
    void MakePack(CNetMessage &msg);
    
    void AddLevel(bool callScript = true);         //等级
    void AddExp(int exp,bool callScript = false);            //经验
    void CallScriptLevelUp();
    
    void AddTiZhi(int tizhi);        //体质
    void AddLiLiang(int liliang);    //力量
    void AddMinJie(int minjie);      //敏捷
    void AddLingLi(int lingli);      //灵力
    void AddNaiLi(int naili);        //耐力
    void AddShuXinDian(int shuxindian);//未分配属性点
    void AddJin(int jin);            //金相性
    void AddMu(int mu);              //木相性
    void AddShui(int shui);          //水相性
    void AddHuo(int huo);            //火相性
    void AddTu(int tu);              //土相性
    void AddXiangXinDian(int xiangxindian);  //未分配相性点
    void AddHp(int hp);              //气血
    void AddMp(int mp);              //法力
    void AddStep(int step);
    void SetMoveTime(uint64 t);
    uint64 GetMoveTime();
    void SetErrMoveTimes(uint8 t);
    uint8 GetMoveErrTimes();
    
    int GetDamage();//物理伤害
    int GetSkillDamage(int skillId);//攻击技能伤害
    int GetRecovery();//防御
    int GetSpeed();//速度
    
    void AddDamage(int damage);
    void AddSkillDamage(int skillDamage);
    void AddRecovery(int recovery);
    void AddSpeed(int speed);
    
    bool AddPackage(SItemInstance &item,const char *name = NULL);
    bool AddPackage(int itemId,uint8 num = 1,const char *name = NULL);
    bool DelPackage(uint8 pos,uint8 num = 1);
    
    int GetMoney()
    {
        return m_money;
    }
    int GetTili()
    {
        return m_tili;
    }
    void SetMoney(uint32 money)
    {
        m_money = money;
        SendUpdateInfo(26,m_money);
    }
    void SetTili(int tili);
    
    void AddTili(int add);
    
    //pos包裹中的位置，返回装备中的位置，< 0表示装备失败
    int EquipItem(uint8 pos,string &errMsg);
    //pos装备中的位置，返回包裹中的位置，< 0表示卸装备失败
    int UnEquipItem(uint8 pos);
    
    int GetItemfangYu();//防御
    int GetItemGongji();//攻击力
    int GetItemFaLi();//增加法力
    int GetItemQiXue();//增加气血
    int GetItemSudu();//增加速度
    
    int GetItemAddAttr(EAddAttrType type);
    
    int GetItemLiliang();//力量(固定值)
    int GetItemLingli();//灵性(固定值)
    int GetItemMinjie();//敏捷(固定值)
    int GetItemTizhi();//体质(固定值)
    int GetItemNaili();//耐力(固定值)	
    
    int GetItemJin();//金相
    int GetItemMu();//木相
    int GetItemShui();//水相
    int GetItemHuo();//火相
    int GetItemTu();//土相	
    
    /*
    连击率=力量*0.00006+土相*0.0007+土系附加（土系天生1%）+装备附加
    物理爆击率=耐力*0.00006+木相*0.0007+木系附加（木系天生1%）+装备附加
    法术爆击率=灵性*0.00006+金相*0.0007+金系附加（金系天生1%）+装备附加
    反击率=体质*0.00006+水相*0.0007+水系附加（水系天生1%）+装备附加
    反震率=敏捷*0.00006+火相*0.0007+火系附加（火系天生1%）+装备附加
    */

    int GetItemMingzhong();//命中率
    int GetItemHuibi();//回避物理攻击
    int GetItemLianjiLv();//连击率
    int GetItemLianjiShu();//连击数
    int GetItemBaoJiLv();//必杀率
    int GetItemFanJiLv();//反击率
    int GetItemFanZhenLv();//反震率
    
    int GetItemGongjiMpJiangDi();//攻击技能MP消耗降低
    int GetItemFuZhuMpJiangDi();//辅助技能MP消耗降低
    int GetItemZhangAiMpJiangDi();//障碍技能MP消耗降低
    
    int GetItemHuiBiFaGong();//回避法术攻击
    
    //攻击伤害减少
    int GetItemKangJin();//金抗性
    int GetItemKangMu();//木抗性
    int GetItemKangShui();//水抗性
    int GetItemKangHuo();//火抗性
    int GetItemKangTu();//土抗性
    
    //攻击命率中降低
    int GetItemKangYiWang();//抗遗忘
    int GetItemKangZhongDu();//抗中毒
    int GetItemKangBingDong();//抗冰冻
    int GetItemKangHunShui();//抗昏睡
    int GetItemKangHunLuan();//抗混乱
    
    int GetItemJiNeng();//全技能
    int GetItemHuShiAllKang();//忽视所有抗性
    int GetItemHuShiAllKangYiChang();//忽视所有抗异常	

    int GetItemBaoJiZhuiJIa();
    int GetAddBaoJiWeiLi();
    int GetLianJiAddShangHai();
    int GetFanJiAddShangHai();
    int GetFanZhenDu();
    int GetFaShuBaoJi();
    int GetFaShuBaoJiAdd();
    int GetFaShuFanTan();//百分比
    int GetFaShuFanTanLv();
    
    bool MakeEquitInfo(uint8 pos,CNetMessage &msg);
    bool MakePackInfo(uint8 pos,CNetMessage &msg);
    void MakePlayerInfo(CNetMessage &msg,CUser *pUser);
    
    void Add1104ItemExp(int exp);
    
    uint32 GetTeam()
    {
        return m_teamId;
    }
    uint32 GetAskForJoinTeam()
    {
        return m_askForJoinTeam;
    }
    void SetTeam(uint32 id)
    {
        m_teamId = id;
    }
    void SetAskForJoinTeam(uint32 id)
    {
        m_askForJoinTeam = id;
    }
    
    void AddAskForMatchUser(uint32 id)//邀请者id
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        m_askForMatchUser.push_back(id);
    }
    
    void ClearAskForMatchUser()
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        m_askForMatchUser.clear();
    }
    
    void DelAskForMatchUser(uint32 id)
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        m_askForMatchUser.remove(id);
    }
    
    bool InAskForMatchUser(uint32 id)
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        list<uint32>::iterator i = m_askForMatchUser.begin();
        for(; i != m_askForMatchUser.end(); i++)
        {
            if(*i == id)
                return true;
        }
        return false;
    }
    
    bool AddPet(SharePetPtr &pPet);
    void SetPet(char *pPet);
    
    void MakePet(CNetMessage &msg);
    
    void AssignPetAttr(uint8 pos,uint8 attr);
    void AssignPetKangXing(uint8 pos,uint8 attr);
    void SetGuanKanPet(uint8 pos);
    void SetChuZhanPet(uint8 pos);
    void SetPetIdle(uint8 pos);
    bool UseItemToPet(uint8 petPos,uint8 itemPos,int *pAddHp = NULL,int *pAddMp = NULL,int val = 0);
    void UseItem(uint8 pos,int *pAddHp = NULL,int *pAddMp = NULL,uint8 val = 0,uint8 val1 = 0,uint8 num = 1);
    
    //战斗中只能使用药品，此函数内不做药品实际效果，返回后根据实际情况选择加给谁
    void FightUseItem(uint8 pos,int &addHp,int &addMp,int maxHp,int maxMp,int hp,int mp,bool pkQieCuo);
    
    void XunYangPet(uint8 pos);
    void DelPet(uint8 pos);
    bool GetChuZhanPet(SharePetPtr &pet);
    void MakeUpdateInfo(CNetMessage &msg,CUser *pUser);
    
    void SetPkVal(uint16 pk)
    {
        m_pkVal = pk;
    }
    uint16 GetPkVal()
    {
        return m_pkVal;
    }
    void AddPkVal(int val);
    
    void AddTitle(uint16 title);
    void SetTitle(uint16 title);
    void GetTitleMsg(CNetMessage &msg);
    void ReadTitle(char *);
    void GetTitleStr(string &str);
    uint16 GetTitle()
    {
        return m_title;
    }
    /*void AddTitle(int val)
    {
        atomic_exchange_and_add((int*)&m_title,val);
    }*/
    
    void SetDaoHang(int daohang)
    {
        m_daohang = daohang;
    }
    int GetDaoHang()
    {
        return m_daohang;
    }
    int AddDaoHang(int val)
    {
        if(val == 0)
            return 0;
        if((val > 0) && (m_daohang > (int)4*m_level*m_level*m_level))
            val /= 10;
        m_daohang += val;
        SendUpdateInfo(7,m_daohang);
        return val;
    }
    void SetQianNeng(int qianNeng)
    {
        m_qianneng = qianNeng;
    }
    int GetQianNeng()
    {
        return m_qianneng;
    }
    void AddQianNeng(int qianNeng)
    {
        m_qianneng += qianNeng;
        SendUpdateInfo(6,m_qianneng);
    }
    uint8 GetChuZhanPet()
    {
        return m_chuZhanPet;
    }
    
    const char *GetCall(int &script);
    
    const char *GetMission(int id);
    bool AddMission(int id,const char *pMiss); //任务
    void UpdateMission(int id,const char *pMiss);//更新任务
    void DelMission(int id);
    void MakeMission(CNetMessage &msg);
    
    void GetHotList(list<uint32> &hotList);
    
    void GetHotList(list<HotInfo> &hotList);
    bool GetHotVal(uint32 id,uint16 &val);
    bool SetHotVal(uint32 id,uint16 val);
    
    bool AddHot(uint32 id);
    bool AddHot(uint32 id,uint16 hotVal);
    
    void DelHot(uint32 id);
    
    SharePetPtr GetPet(uint8 pos);
    SPet GetPetByPos(uint8 pos);
    
    void SetShop(char*);
    
    void SetBitSet(char*);
    
    //188秒杀、189充值100反通宝
    void SetBitSet(int ind)
    {
        if(ind >= MAX_BITSET)
            return;
        m_bitset.set(ind);
        if(ind == 0)
        {
            ClearAnswer();
        }
    }
    void ClearBitSet(int ind)
    {
        if(ind >= MAX_BITSET)
            return;
        m_bitset.set(ind,false);
    }
    
    bool HaveBitSet(int ind)
    {
        if(ind >= MAX_BITSET)
            return false;
        return m_bitset.test(ind);
    }
    
    bool TakeDownShopItem(uint8 pos);
    uint16 PutItemToShop(uint8 type,uint8 &pos,uint8 num,int money);//返回物品、宠物id
    void MakeShopItemList(CNetMessage &msg);
    bool BuyUserShopItem(CUser *pUser,uint8 pos,uint32 shopItemId,string &str);
    void MakeShopItemInfo(uint8 pos,CNetMessage &msg);
    
    uint8 GetMenPai()
    {
        return m_menpai;
    }
    void SetMenPai(uint8 menpai);
    
    uint16 GetGenSuiPetId();
    void SetBangPai(uint32 bangpai)
    {
        m_bangpai = bangpai;
        if(bangpai == 0)
            SetData32(5,0);
    }
    uint32 GetBangPai()
    {
        return m_bangpai;
    }
    int GetBangState();
    int GetBangRank();
    void DismissBang();// 解散帮派
    void UndismissBang();// 解除解散状态

    void SetTradeUser(uint32 id)
    {
        m_tradeUserId = id;
        memset(m_tradeType,2,sizeof(m_tradeType));
        memset(m_tradeItemPos,0xff,sizeof(m_tradeItemPos));
        memset(m_tradeMD5,0,sizeof(m_tradeMD5));
        m_tradeMoney = 0;
        m_tradeOk = false;
        m_inTrade = false;
    }
    uint32 GetTradeUser()
    {
        return m_tradeUserId;
    }
    bool SetTradeItem(CNetMessage &msg);
    void GetTradeInfo(CNetMessage &msg);
    void TradeOk()
    {
        m_tradeOk = true;
    }
    bool TradeIsOk()
    {
        return m_tradeOk;
    }
    bool TradeItem(CUser*);
    void SetInTrade()
    {
        m_inTrade = true;
    }
    bool IsInTrade()
    {
        return m_inTrade;
    }
    void SetVal(int id,int val);
    int GetVal(int id);
    /***************
   +------+----+-------+
   | TYPE | OP | VALUE |
   +------+----+-------+
   |  1   |  1 |   4   |
   +------+----+-------+
    TYPE=1 声望
    TYPE=2 战绩
    TYPE=3 帮贡
    TYPE=4 属性点
    TYPE=5 相性点
    TYPE=6 潜能
    TYPE=7 道行
    TYPE=9 气血
    TYPE=10 最大气血
    TYPE=11 mp
    TYPE=12 最大mp
    TYPE=13 伤害
    TYPE=14 速度
    TYPE=15 防御
    TYPE=16 体质
    TYPE=17 力量
    TYPE=18 敏捷
    TYPE=19 灵力
    TYPE=20 耐力
    TYPE=21 金
    TYPE=22 木
    TYPE=23 水
    TYPE=24 火
    TYPE=25 土
    TYPE=26 金钱
    TYPE=27 通宝
    TYPE=28 法攻
    TYPE=29 命中 
    TYPE=30 回避
    TYPE=31 命中率提升
    TYPE=32 闪避率提升
    TYPE=33 连击率
    TYPE=34 爆击率
    TYPE=35 反击率
    TYPE=36 反震率
    TYPE=37 法反率
    TYPE=38 法爆率
    TYPE=40 魔障
    TYPE=42 体力
    TYPE=43 开启第四背包背包
    TYPE=44 状态
    TYPE=45 val：0背包失效，1仓库失效，2宠物仓库失效
    TYPE=46 更新绑定通宝
    TYPE=47 下一次领取礼盒时间
    *****************/
    void SendUpdateInfo(uint8 type,int val,bool add = false);
    
    bool OpenPack(uint8 type,SItemInstance *pItem);
    
    SItemInstance *GetItem(uint8 pos)
    {
        if((pos >= MAX_PACKAGE_NUM) || (m_package[pos].tmplId == 0))
            return NULL;
        return m_package + pos;
    }
    
    SItemInstance *GetItemById(int id);
    
    SItemInstance *GetEquip(uint8 pos)
    {
        if((pos >= EQUIPMENT_NUM) || (m_equipment[pos].tmplId == 0))
            return NULL;
        return m_equipment + pos;
    }
    void AddMoney(int add);
    
    uint8 GetWuQiType();
    void LoadMission();
    void UpdatePetToBaby(uint8 pos);
    uint8 GetGenSuiPet()
    {
        return m_gensuiPet;
    }
    void SaveData(CDatabaseSql *pDb,bool lock = true);
    
    //查询装备强化结果
    bool MakeQiangHuaInfo(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num,CNetMessage &msg);
    
    //0成功，1，失败，2不能装备
    int QiangHuaPackage(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num);
    
    //0成功，1，失败，2不能炼化,stone1,stone2,stone3为黑水晶位置
    int LianHuaPackage(uint8 itemPos,uint8 stone1,uint8 stone2,uint8 stone3);
    
    //0成功，1，失败，2不能炼化，tianHuanShi天换石(623)为位置(),lianHuaShi炼化石位置(611)
    int TianHuanPackage(uint8 itemPos,uint8 tianHuanShi);//,uint8 lianHuaShi);
    
    //0成功，1，失败，2不能制作
    int MakeGreenItem(uint8 item,uint8 stone);
    
    //0成功，1，失败，2不能装备
    int SelectGreenItemAttr(uint8 item,uint8 attrInd,uint8 shuijing,uint8 stone,uint8 num);
    
    bool IsAutoFight()
    {
        return m_autoFightTurn > 0;
    }
    void GetAutoFightOp(uint8 &userOp,
                        int  &userPara,
                        uint8 &target,
                        uint8 &petOp,
                        int &petPara,
                        uint8 &petTar)
    {
        userOp = m_userOp;
        userPara = m_userPara;
        target = m_target;
        petOp = m_petOp;
        petPara = m_petPara;
        petTar = m_petTar;
    }
    void SetAutoFightTurn(int turn)
    {
        m_autoFightTurn = turn;
    }
    int GetAutoFightTurn()
    {
        return m_autoFightTurn;
    }
    void SaveAutoFight(uint8 userOp,
                        int  userPara,
                        uint8 target,
                        uint8 petOp,
                        int petPara,
                        uint8 petTar)
    {
        m_userOp =      userOp;
        m_userPara =  userPara;
        m_target =      target;
        m_petOp =        petOp;
        m_petPara =    petPara; 
        m_petTar =      petTar;
    }
    
    bool ModifyPetName(uint8 pos,string &name,string &errMsg);
    void UpdatePet(uint8 pos);
    void DecreaseWuQiNaiJiu(int naijiu);
    void DecreaseFangJuNaiJiu(int naijiu);
    void DecreaseAllNaiJiu(int naijiu = 0);
    bool HaveZeroEquip();
    //0成功，1，失败，2不能修理
    int RepairItem(uint8 pos);
    //耐久修理费=取整（装备等级*要维修的耐久*0.1）+1
    //获得修理所有装备费用
    int GetRepairFee();
    void RepairTotel();
    
    void SetSaveVal(uint8 index,int val);
    int GetSaveVal(uint8 index);
    void SetSaveVal(char *msg);
    
    
    bool DelPackageById(int id,int num);
    int GetItemNum(int id);
    
    //设置进入场景是否调用脚本
    /*void SetEnterSceneCall(int id)
    {
        m_enterSceneCall = id;
    }*/
    
    int AddNpcInfo(int sceneId,CNetMessage &msg);
    void AddNpc(int scecseId,SNpcInstance &npc);
    uint16 DelNpc(int npcId,SNpcInstance &npc);
    bool FindNpcNear(SNpcInstance &npc);
    void MakeNpc(int ncpId,CNetMessage &msg);
    
    void SetNpc(char *row);
    void UserJump(bool inJump)
    {
        m_inJump = inJump;
    }
    bool InJump()
    {
        return m_inJump;
    }
    void TimeOut();
    void Clear();
    void SetMPGongXian(int val)
    {
        m_menPaiGongXian = val;
    }
    int GetMPGongXian()
    {
        return m_menPaiGongXian;
    }
    const char *GetNpcName(int npcId);
    uint32 GetHumanNcpRoleId(int npcId);
    bool ReadData(uint32 roleId,uint16 *pSceneId = NULL);
    void SetSkill(int skillId,int level);
    int GetSkillLevel(int skillId);
    int GetSrcSkillLevel(int skillId);
    void UpdateSkill();
    void MakeUserSkill(CNetMessage&);
    void MakePetSkill(uint8 pos,CNetMessage &msg);
    
    void SetPetZhongCheng(uint8 pos,int zhongcheng);
    void PetRestore(uint8 pos);
    
    //false不能鉴定
    int JianDingItem(uint8 pos);
    
    //鉴定饰品
    bool JianDingShiPin(uint8 pos);
    
    //饰品镶嵌
    bool XiangQian(uint8 pos,uint8 stone);
    
    bool UserDouble()
    {
        return time(NULL) < m_userDoubleEnd;
    }
    bool PetDouble()
    {
        return time(NULL) < m_petDoubleEnd;
    }
    bool NoExperiencePunish()
    {
        return time(NULL) < m_noPunishEnd;
    }
    bool HaveItem(int id);
    uint8 HaveEmptyPack();
    
    bool HavePet(int id);
    bool HaveBaobaoPet(int id);
    //waitTime单位为分钟
    void AddTimer(int id,int waitTime);
    void DelTimer(int id);
    
    bool CompoundStone(uint8 pos,uint8 num);
    void GetViewPara(int &fagong,uint16 &mingzhong,uint16 &huibi);
    
    //合成物品，返回合成物品id，0时表示合成失败
    int CompoundItem(uint8 pos1,uint8 pos2,uint8 pos3);
    int CompoundItem1(int id);
    
    //掉落未绑定装备
    void DropItem(string &name);
    void GetDropItem(string &item);
    void SetChatChannel(uint8 val)
    {
        m_chatChannel = val;
    }
    uint8 GetChatChannel()
    {
        return m_chatChannel;
    }
    void SetMaxHp(int maxHp)
    {
        m_maxHp = maxHp;
    }
    void UpdateEquip(uint8 pos);
    int GetBankMoney()
    {
        return m_bankMoney;
    }
    void SetBankMoney(int money)
    {
        m_bankMoney = money;
    }
    void UpdatePetInfo(uint8 petPos,uint8 type,int val);
    /***************
    TYPE=1 状态
   TYPE=2 属性点
   TYPE=3 武学
   TYPE=4 经验
   TYPE=5 气血
   TYPE=6 法力
   TYPE=7 体质
   TYPE=8 力量
   TYPE=9 敏捷
   TYPE=10 灵力
   TYPE=11 耐力
   TYPE=12 寿命
   TYPE=13 忠诚度
   TYPE=14 亲密度
   
   TYPE=15 最大气血
   TYPE=16 最大法力
   TYPE=17 物攻
   TYPE=18 法攻
   TYPE=19 速度
   TYPE=20 防御
   TYPE=21 移动速度
   TYPE=22 铠甲耐久
   TYPE=23 修为
    ****************/
    
    void UseAddHpItem();
    void UserAddMpItem();
    void MoveItem(uint8 srcPos,uint8 tarPos);
    void BangDingItem(uint8 pos);
    
    CCallScript *FindNpcScript(int npcId);
    void UpdateInfo();
    bool MakePetInfo(uint8 pos,CNetMessage &msg);
    
    SPet GetCZPet();
    void SetCZPetWuXue(int wuxue);
    bool MissionFull();
    
    bool IsFewMonster()
    {
        return m_fewMonsterEnd > time(NULL);//遇敌步数增加
    }
    bool IsMaxMonster()
    {
        return m_maxMonsterEnd > time(NULL);//遇敌最大(数量）
    }
    bool IsMinMonster()
    {
        return m_minMonsterEnd > time(NULL);//遇敌最少(
    }
    bool IsUpMonster()
    {
        return m_upMonsterEnd > time(NULL);//只遇到>=人物等级的怪
    }
    //type 0 通宝，1绑定通宝
    int GetTongBao(uint8 type = 0)
    {
        if(type == 1)
            return m_bdTongBao;
        return m_tongBao;
    }
    void SetTongBao(int tongbao,uint8 type = 0)
    {
        if(type == 1)
            m_bdTongBao = tongbao;
        else
            m_tongBao = tongbao;
    }
    void AddTongBao(int tongbao,uint8 type = 0);
    bool IsLogout()
    {
        return m_logout;
    }
    void UserLogout(bool flag)
    {
        m_logout = flag;
    }
    void AddMonsterScript(int monsterId,int scriptId);
    void DelMonsterScript(int monsterId,int scriptId);
    int FindMonsterScript(int monsterId,list<int> *pList);
    void GetRoles(uint32 *roles)
    {
        memcpy(roles,m_role,sizeof(m_role));//MAX_ROLE_NUM];
    }
    void MakeSavePosInfo(CNetMessage &msg);
    bool CanDelPackage(uint8 pos);
    void SendItemTimeOut();    
    void UpdateBangPai();
    
    //给予绑定物品
    bool AddBangDingPackage(int itemId,int num = 1,const char *name = NULL);
    //给予蓝水晶          属性      属性值
    bool AddBlueCrystal(int attr,int attrVal);
    //给予炼化石
    bool AddLianHuaStone(int level,int num = 1);
    //给予指定等级强化装备
    bool AddLevelPackage(int itemId,int level);
    
    bool AddAttrPackage(int itemId,int attr,int attrVal,bool bang = true);
    
    void SetScene(CScene *p)
    {
        m_pScene = p;
    }
    int EmptyPackage();
    void SaveSellItem(uint8 pos,uint8 num);
    void SaveDelItem(uint8 pos);
    //void operator=(CUser&);
    void MakeSaveShuXing(string &shuxing);
    void MakeSaveEquip(string &equip);
    bool CanChat();
    void SetChatTime(time_t t)
    {
        m_chatTime = t;
    }
    uint32 GetChatTime()
    {
        return m_chatTime;
    }
    void ClearAnswer();
    uint16 GetAnswerSpace();
    
    void SetAskTime(time_t t)
    {
        m_askTime = t;
    }
    time_t GetAskTime()
    {
        return m_askTime;
    }
    void SetRightAns(uint8 pos)
    {
        m_rightAnswer = pos;
    }
    uint8 GetRightAns()
    {
        return m_rightAnswer;
    }
    uint8 GetAnswerTimes()
    {
        return m_answerTimes;
    }
    void SendMsgToTeamMember(const char *msg);
    void SetAnswerTimes(uint8 times)
    {
        m_answerTimes = times;
    }
    uint8 AdminLevel()
    {
        return m_admin;
    }
    void SetPkTime(time_t t)
    {
        m_pkTime = t;
    }
    bool CanPk()
    {
        //cout<<GetSysTime() - m_pkTime<<endl;
        return GetSysTime() - m_pkTime > 5*60;
    }
    bool UserInfoIsOpen()
    {
        return (m_chatChannel & 32) != 0;
    }
    bool CanFightHuoDong();
    void SetHuoDongFightTime(time_t t)
    {
        m_huodongTime = t;
    }
    time_t GetLastHeartTime()
    {
        return m_lastHeartTime;
    }
    void SetLastHeartTime(time_t t)
    {
        m_lastHeartTime = t;
    }
    void SetHeartTimes(uint8 t)
    {
        m_heartTimes = t;
    }
    uint8 GetHeartTimes()
    {
        return m_heartTimes;
    }
    void SetHBErrTimes(uint8 t)
    {
        m_heartErrTimes = t;
    }
    uint8 GetHBErrTimes()
    {
        return m_heartErrTimes;
    }
    //0解绑成功，1 没有宠物解绑符，2 宠物非绑定、3 解绑时间未到
    int PetJieBang(uint8 petPos);
    
    int GetLeftDoubleTime();
    void SetDouble(int hour);
    
    //打开背包，1第三个、2第四个
    void OpenPackage(int tab)
    {
        m_openPack = tab;
        if((m_openPack > 0) && (m_openPack <= 1))
            MAX_PACKAGE_NUM = (2+m_openPack)*18;
        SendUpdateInfo(43,m_openPack);
    }
    uint8 GetOpenPack()
    {
        return m_openPack;
    }
    bool ScriptDouble()
    {
        return m_sDoubleEnd > GetSysTime();
    }
    //0 师傅等级
    //1 比赛死亡次数
    //领取礼盒剩余时间
    //3出师徒弟数量
    //4,5,8脚本使用
    //6 婚礼礼服状态
    //7,8脚本使用
    //9 被pk次数
    //10,11,12,13,14世界大战金木水火土
    //15脚本使用
    void SetData8(uint8 pos,uint8 data)
    {
        if(pos < UINT8_NUM)
            m_save8[pos] = data;
    }
    uint8 GetData8(uint8 pos)
    {
        if(pos < UINT8_NUM)
            return m_save8[pos];
        return 0;
    }
    int GetChuShiNum()
    {
        return GetData8(3);
    }
    //0 声望（善恶）
    //1 比赛积分
    //2,5 脚本使用
    //3 世界大战积分
    //4 每天在线时间
    //6 周积分
    //7 每日积分
    //8 挑战十妖使用时间 英雄
    //10,11脚本使用
    //12 端午活动积分
    void SetData16(uint8 pos,uint16 data)
    {
        if(pos < UINT16_NUM)
        {
        	if((pos == 0) && (data > 30000))
        		data = 30000;
            m_save16[pos] = data;
        }
    }
    int GetJiFen()
    {
        return (short)GetData16(1);
    }
    int GetDieTimes()
    {
        return GetData8(1);
    }
    uint16 GetData16(uint8 pos)
    {
        if(pos < UINT16_NUM)
        {
        	if((pos == 0) && (m_save16[pos] > 30000))
        		m_save16[pos] = 30000;
            return m_save16[pos];
        }
        return 0;
    }
    //0 解散师徒关系时间
    //1 状元、探花、榜眼title时间
    //2 离开帮派时间
    //3 副本id
    //4 偷菜时间
    //5 帮贡
    //6 结婚roleid
    //7 离婚时间
    //8 结婚时间
    //9 种菜积分（帮贡）
    //10 恩爱值
    //11 进入副本306时间
    //12 魅力值
    //13结拜解散时间
    void SetData32(uint8 pos,uint32 data)
    {
        if(pos < UINT32_NUM)
        {
            m_save32[pos] = data;
            if(pos == 6)
                m_save32[10] = 0;
            if((pos == 12) && (m_save32[12] >= 500))
            {
                if(m_sex == 0)
                    AddTitle(11);
                else
                    AddTitle(10);
            }
        }
    }
    uint32 GetData32(uint8 pos)
    {
        if(pos < UINT32_NUM)
            return m_save32[pos];
        return 0;
    }
    int GetShengWang()
    {
        return GetData16(0);
    }
    void SetShengWang(int sw)
    {
        SetData16(0,sw);
        SendUpdateInfo(1,GetData16(0));
    }
    //拆分,0成功，1 拆分石不符 
    int ChaiFen(uint8 weaponPos,uint8 attrPos,uint8 stonePos);
    //融合，0成功，1 石头不符 
    int RongHe(uint8 weaponPos,uint8 stonePos);
    
    int ChaiFenSX(uint8 weaponPos,uint8 attrPos);
    void SetShiFu();
    
    int GetHeChengVal(uint8 pos1,uint8 pos2);
    bool HeChengLanShuiJing(uint8 pos1,uint8 pos2,uint8 hechengfu);
    
    //合成宠物铠甲
    int HeChengKaiJia(int target ,uint8 kaijia);//,uint8 pos1,uint8 pos2,uint8 pos3);
    
    void SetBangZhanScore(int score)
    {
        m_bangZhanScore = score;
    }
    int GetBangZhanScene()
    {
        return m_bangZhanScore;
    }
    const static uint8 UINT8_NUM = 16;
    const static uint8 UINT16_NUM = 16;
    const static uint8 UINT32_NUM = 16;
    
    bool HaveShop()
    {
        for(uint8 i = 0; i < MAX_SHOP_ITEM_NUM; i++)
        {
            if(m_shopItemId[i] != 0)
                return true;
        }
        return false;
    }
    int GetXZBangId()
    {
        return m_XZBangId;
    }
    void SetXZBangId(int id)
    {
        m_XZBangId = id;
    }
    /*void SetPkUser(uint32 roleId)
    {
        m_pkUser = roleId;
    }
    uint32 GetPkUser()
    {
        return m_pkUser;
    }*/
    void ReadSaveData(char *);
    const static int MAX_BITSET = 1024 * 8;
    void GetBitSet(string &str);
    void WriteSaveData(string&);
    bool KaiJiaXiangQian(uint8 kaijiaPos,uint8 kongPos,uint8 stonePos,string &msg);
    
    //宠物学习技能
    bool PetStudySkill(uint8 petPos,uint16 skillId);
    
    //装备宠物铠甲
    bool PetKaiJia(uint8 petPos,uint8 kaiJiaPos);
    
    //得到宠物位置
    int GetPetById(int id);
    
    void SetQiPet(uint8 pos);
    
    void AddPetQinMi(uint8 pos,int qinmi);
    
    void DesKaiJiaNaijiu(int val);
    
    uint8 GetQiPet()
    {
        return m_qiPet;
    }
    int GetQiPetKangWuLi();//9
    int DuiHuanTB();
    bool HaveNameItem(uint16 itemId,const char *name);
    
    //type = 3表示可骑乘
    void SetPetType(uint8 pos,uint8 type);
    uint32 TempLeaveTeam()
    {
        return m_tempLeaveTeam;
    }
    void SetTempLeaveTeam(uint32 teamId)
    {
        m_tempLeaveTeam = teamId;
    }
    time_t GetActivityTime()
    {
        return m_activityTime;
    }
    void SetActivityTime(time_t t)
    {
        m_activityTime = t;
    }
    bool HaveIgnore(uint32 roleId);
    bool AddIgnore(uint32 roleId);
    void DelIgnore(uint32 roleId);
    void GetIgnoreList(list<HotInfo> &ignoreList);
    void ReadIgnore();
    int GetPkMiss(uint32 roleId);
    
    void XiShouPet(uint8 ind);
    bool AddQiLing();
    
    void GetEquipment(string &str);
    void GetPackage(string &str);
    void GetPet(string &str);
    void GetShop(string &str);
    void SetGuanZhan(uint32 fightId)
    {
        m_guanFight = fightId;
    }
    uint32 GetGuanZhan()
    {
        return m_guanFight;
    }
    //战斗结束后使用自动补血补篮药
    void UseFightEnd();
    
    //0第四背包扩容、1 仓库扩容，2 宠物仓库扩容
    //3端午活动使用，每日清
    uint8 GetExtData8(uint16 pos);
    void SetExtData8(uint16 pos,uint8 val);
    
    //0礼盒物品
    uint16 GetExtData16(uint16 pos);
    void SetExtData16(uint16 pos,uint16 val);
    
    //0第四背包扩容、1 仓库扩容，2 宠物仓库扩容结束时间
    uint32 GetExtData32(uint16 pos);
    void SetExtData32(uint16 pos,uint32 val);
    
    void MakeBankPet(CNetMessage &msg);
    
    //pos宠物位置
    bool SaveBankPet(uint8 pos);
    
    //pos在宠物商店中的位置
    bool GetBankPet(uint8 pos);
    
    void QueryTextTitle(CNetMessage &msg);
    
    void AddTextTitle(const char *pTitle);
    void DelTextTitle(const char *pTitle);
    void UseTextTitle(const char *pTitle);
    bool HaveAllWanMei();
    bool HaveLevel12Equip();
    void GetGift(uint8 type=0);
    
    time_t m_yewaiShiYao;//野外十妖时间
private: 
    bool NewUserGift();//新手宝箱
    bool NormalUserGift();//每日宝箱
    void NormalUserGift0(uint16 nextTime);
    void NormalUserGift1(uint16 nextTime);
    void NormalUserGift2(uint16 nextTime);
    void SelectUserGift(uint16 *gifts,uint16 num);
    
    uint8 GetCurMaxBankPetNum();
    uint8 GetCurMaxPackNum();
    
    uint8 NoLockGetExtData8(uint16 pos);
    void NoLockSetExtData8(uint16 pos,uint8 val);
    
    uint16 NoLockGetExtData16(uint16 pos);
    void NoLockSetExtData16(uint16 pos,uint16 val);
    
    uint32 NoLockGetExtData32(uint16 pos);
    void NoLockSetExtData32(uint16 pos,uint32 val);
    
    uint8 GetPingZhi();
    int GetQiPetNaiLi(); //1.5
    int GetQiPetTiZhi(); //1.5
    int GetQiPetMinJie();//1.5
    int GetQiPetFangYu();//9  
    int GetQiPetMp();    //15 
    int GetQiPetHp();    //15 
    int GetQiPetSpeed(); //1.5
    
    int GetQiPetKangJin();//9
    int GetQiPetKangMu();//9
    int GetQiPetKangShui();//9
    int GetQiPetKangHuo();//9
    int GetQiPetKangTu();//9
        
    void NoLockMakePetSkill(uint8 pos,CNetMessage &msg);
    void ReadHots(char*);
    
    void UpdateMission();
    bool GetHumanData(uint32 roleId,HumanData &human,string &name);
    void WriteHots(string &hots);
    bool CanSale(SItemInstance &);
    
    //uint8 GetItemPos(uint16 id);
    void UpdatePackage(uint8 pos);
    
    void SaveData();
    
    time_t m_huodongTime;
    time_t m_saveDataTime;
    time_t m_loginTime;
    time_t m_giftTime;//领取礼盒累计时间
    time_t m_chatTime;
    time_t m_answerTime;//回答问题时间
    time_t m_askTime;
    time_t m_lastHeartTime;
    time_t m_activityTime;
    bool m_readIgnore;
    
    int m_XZBangId;
    
    uint8 m_heartTimes;
    uint8 m_rightAnswer;
    uint8 m_answerTimes;//回答的次数
    uint8 m_admin;
    uint8 m_heartErrTimes;
    
    struct SUserGift
    {
        uint8 type;//1物品，2经验、3潜能，4道行
        uint16 val;
        uint8 num;
        uint8 pos;
    };
    SUserGift m_gift;
    uint16 m_giftBitset;
    
    uint8 m_openPack;
    
    uint16 m_answerTimeSpace;//
    
    uint16 m_jianYuTime;
        
    void NoLockSaveData(CDatabaseSql *pDb);
    
    bool m_callLevelScript;
    bool m_logout;
    //如果是使用脚本，返回脚本id
    CCallScript *NoLockUseItem(uint8 pos,int *pAddHp,int *pAddMp,uint8 val,uint8 val1,uint8 num);
    
    struct STimer
    {
        int id;
        time_t endTime;
    };
    
    const static int MAX_TIMER  = 3;
    STimer m_scriptTimer[MAX_TIMER];
    
    void SetSkills(char *row);
    
    CCallScript *GetTimeOutNpcScript();
    
    bool m_inJump;
    
    uint16 NoLOckGetGenSuiPetId();
    //void EquipmentAddAttr(bool add);
    
    //add = true为加，add为false为减
    //void EquipAddAttr(SItemInstance &pItem,bool add);
    
    bool NoLockAddBankItem(SItemInstance &item,uint8 &tolSaveNum);
    void NoLockDelBankItem(uint8 bankPos);
        
    bool NoLockAddPackage(SItemInstance &item,uint8 *pPos = NULL,bool trade=false,int *pAddQinMi = NULL);
    //bool NoLockAddPackage(int itemId,uint8 num = 1,const char *name=NULL);
    bool NoLockDelPackage(uint8 pos,uint8 num = 1);
    void NoLockDelPet(uint8 pos);
    bool NoLockAddPet(SharePetPtr &ptr); 
    
    bool NoLockMakePetInfo(uint8 pos,CNetMessage &msg);
    void MakePetInfo(SPet *pPet,uint8 state,CNetMessage &msg);
    
    void MakePack(SItemInstance &item,uint8 pos,CNetMessage &msg);
    
    const static int MAX_TRADE_NUM  = 3;
    
    bool GetTradeItem(SItemInstance *pItem[MAX_TRADE_NUM],uint8 &num);
    bool GetTradePet(SharePetPtr *pPet[MAX_TRADE_NUM],uint8 &num);
    
    bool CanSavePackage(SItemInstance **pItem,uint8 num);
    uint8 CanSavePetNum();
    bool CanGetPet(SharePetPtr *pPet[MAX_TRADE_NUM],uint8 num);
    
    void DelTradeItem();
    void ClearTimeoutTitle();
    
    uint8 m_xPos;
    uint8 m_yPos;
    uint8 m_face;
    uint8 m_sex;
    uint8 m_head;
    uint8 m_xiang;
    uint8 m_level;
    
    uint8 m_fightPos;
    uint8 m_menpai;
    
    uint8  m_autoFightTurn;
    
    int16 m_tizhi;
    int16 m_liliang;
    int16 m_minjie;
    int16 m_lingli;
    int16 m_naili;
    int16 m_shuxingdian;
    int16 m_jin;
    int16 m_mu;
    int16 m_shui;
    int16 m_huo;
    int16 m_tu;
    int16 m_xiangxingdian;
    int16 m_pkVal;
    
    uint16 m_title;//目前使用的title
    
    struct STitleData
    {
        uint16 id;
        time_t begin;//给予时间
    };
    const static int MAX_TITLE_NUM = 30;
    const static int TITLE_ARRAY_SIZE = 32;
    STitleData m_titleList[TITLE_ARRAY_SIZE];
    
    short m_menPaiGongXian;
    
    int m_daohang;
    int m_qianneng;
    
    uint32 m_userId;
    uint32 m_roleId;
    int m_sock;
    int m_exp;
    int m_hp;
    int m_mp;
    uint32 m_role[MAX_ROLE_NUM];
    int m_maxHp;
    int m_maxMp;
    uint32 m_fightId;
    int m_damage;
    int m_skillDamage;
    int m_recovery;
    int m_speed;
    uint16 m_tili;
    int m_money;
    int m_tongBao;
    int m_bdTongBao;
    int m_bangZhanScore;
    
    uint32 m_step;
    uint64 m_moveTime;
    uint8 m_moveErrTimes;
    
    uint32 m_askForJoinTeam;
    uint32 m_teamId;
    uint32 m_bangpai;
    //uint32 m_pkUser;
    
    //自动战斗保存数据
    uint8 m_userOp;
    int  m_userPara;
    uint8 m_target;
    uint8 m_petOp;
    int m_petPara;
    uint8 m_petTar;
    
    uint8 m_save8[UINT8_NUM];
    uint16 m_save16[UINT16_NUM];
    uint32 m_save32[UINT32_NUM];
    
    //0第四背包扩容、1 仓库扩容，2 宠物仓库扩容
    map<uint16,uint8> m_saveData8;
    
    map<uint16,uint16> m_saveData16;
    
    //0第四背包扩容、1 仓库扩容，2 宠物仓库扩容结束时间
    map<uint16,uint32> m_saveData32;
    
    uint32 m_guanFight;//观战的战斗
    
    CScene *m_pScene;
    //CScene *m_pOleScene;
    
    //string m_name;
    char m_name[MAX_NAME_LEN];
    
    struct SScriptCall
    {
        int scriptId;
        string func;
    };
    //list<SScriptCall> m_scriptHeap;
    string m_scriptCall;
    int m_script;
    
    const static int EQUIPMENT_NUM = 9;
    SItemInstance m_equipment[EQUIPMENT_NUM];
    //SItemTemplate *m_equipmentTmpl[EQUIPMENT_NUM];
    
    const static int MAX_PACKAGE_NUM2 = 72;
    uint8 MAX_PACKAGE_NUM;
    SItemInstance m_package[MAX_PACKAGE_NUM2];
    
    const static int MAX_BANK_ITEM_NUM  = 72;//18;
    SItemInstance m_bankItem[MAX_BANK_ITEM_NUM];
    
    vector<SPet> m_bankPet;
    
    const static int MAX_SHOP_ITEM_NUM = 6;
    struct SShopItem
    {
        uint8 type;
        int money;
        union
        {
            SPet pet;
            SItemInstance item;
        };
    };
    SShopItem m_shopItem[MAX_SHOP_ITEM_NUM];
    uint32 m_shopItemId[MAX_SHOP_ITEM_NUM];//存储shop物品的动态id
    
    uint8 m_petNum;//宠物数量
    uint8 m_chuZhanPet;//出战宠物
    uint8 m_gensuiPet;//跟随宠物
    uint8 m_qiPet;//骑宠
    
    struct SMission
    {
        int dbId;
        uint16 id;
        string mission;
    };
    
    const static int MAX_MISSION_NUM = 10;
    SMission m_mission[MAX_MISSION_NUM];
    
    uint32 m_tradeUserId;
    
    char m_tradeType[MAX_TRADE_NUM];
    char m_tradeItemPos[MAX_TRADE_NUM];
    char m_tradeNum[MAX_TRADE_NUM];
    const static int MD5_RESULT_SIZE    = 16;
    uint8 m_tradeMD5[MAX_TRADE_NUM][MD5_RESULT_SIZE];
    
    int   m_tradeMoney;
    bool m_tradeOk;
    bool m_inTrade;
    time_t m_userDoubleEnd;
    time_t m_petDoubleEnd;
    time_t m_noPunishEnd;
    
    time_t m_fewMonsterEnd;//遇敌步数增加
    time_t m_maxMonsterEnd;//遇敌最大
    time_t m_minMonsterEnd;//遇敌最少
    time_t m_upMonsterEnd;//只遇到>=人物等级的怪
    time_t m_fightEndTime;
    
    time_t m_addTiliTime;
    time_t m_pkTime;
    
    
    time_t m_sDoubleEnd;//脚本设置双倍时间
    uint32 m_tempLeaveTeam;//暂时离开队伍
    
    uint8 m_chatChannel;
    const static int MAX_SAVE_NUM = 12;
    
    uint32 m_shortArray[MAX_SAVE_NUM];
    //MAX_SAVE_NUM-2:为更新日期
    //MAX_SAVE_NUM-1,为记录每周一需要更新的变量
        
    int m_bankMoney;
    
    bitset<MAX_BITSET> m_bitset;
    //uint16 m_enterSceneCall;
    map<int,int> m_intMap;
    //monsterid scriptid
    multimap<int,int> m_monsterScript;
    list<SNpcInstance> m_npcList;
    list<uint32> m_askForMatchUser;
    
    list<HotInfo> m_hotList;
    list<HotInfo> m_ignoreList;
    
    const static uint8 MAX_PET_NUM = 8;
    SharePetPtr m_pet[MAX_PET_NUM];
    
    struct UserSkill
    {
        uint16 id;
        uint16 level;
    };
    list<UserSkill> m_userSkill;
    
    list<string> m_textTitle;
    string m_useTextTitle;
    
    struct SSavePos
    {
        uint16 sceneId;
        uint8 x;
        uint8 y;
    };
    const static uint8 MAX_SAVE_POS = 10;
    SSavePos m_savePos[MAX_SAVE_POS];
    string m_dropItem;
    boost::recursive_mutex m_mutex;
};

inline void CUser::SetRoleId(uint32 id)
{
    m_roleId = id;
}
inline void CUser::SetPos(uint8 x,uint8 y)
{
    m_xPos = x;
    m_yPos = y;
}
inline uint32 CUser::GetUserId()
{
    return m_userId;
}
inline uint32 CUser::GetRoleId()
{
    return m_roleId;
}
inline void CUser::SetUserId(uint32 id)
{
    m_userId = id;
}

inline uint8 CUser::GetFace()
{
    return m_face;
}
inline void CUser::GetPos(uint8 &x,uint8 &y)
{
    x = m_xPos;
    y = m_yPos;
}
inline CScene *CUser::GetScene()
{
    return m_pScene;
}

inline void CUser::SetSock(int sock)
{
    m_sock = sock;
}
inline int CUser::GetSock()
{
    return m_sock;
}

inline void CUser::SetRole(uint32 *roles)
{
    memcpy(m_role,roles,sizeof(m_role));
}

inline const char *CUser::GetName()
{
    return m_name;
}

inline uint8 CUser::GetSex()
{
    return m_sex;
}
inline uint8 CUser::GetXiang()
{
    return m_xiang;
}
inline uint8 CUser::GetX()
{
    return m_xPos;
}
inline uint8 CUser::GetY()
{
    return m_yPos;
}
inline uint8 CUser::GetLevel()
{
    return m_level;
}
inline uint32 CUser::GetExp()
{
    return m_exp;
}

inline uint16 CUser::GetShuXinDian()
{
    return m_shuxingdian;
}

inline uint16 CUser::GetXiangXinDian()
{
    return m_xiangxingdian;
}
inline int CUser::GetHp()
{
    return m_hp;
}

inline void CUser::SetSex(uint8 sex)
{
    m_sex = sex;
}
inline void CUser::SetXiang(uint8 xiang)
{
    m_xiang = xiang;
}
inline void CUser::SetLevel(uint8 level)
{
    m_level = level;
}
inline void CUser::SetExp(uint32 exp)
{
    m_exp = exp;
}
inline void CUser::SetTiZhi(uint16 tizhi)
{
    m_tizhi = tizhi;
}
inline void CUser::SetLiLiang(uint16 liliang)
{
    m_liliang = liliang;
}
inline void CUser::SetMinJie(uint16 minjie)
{
    m_minjie = minjie;
}
inline void CUser::SetLingLi(uint16 lingli)
{
    m_lingli = lingli;
}
inline void CUser::SetNaiLi(uint16 naili)
{
    m_naili = naili;
}
inline void CUser::SetShuXinDian(uint16 shuxindian)
{
    m_shuxingdian = shuxindian;
}
inline void CUser::SetJin(uint16 jin)
{
    m_jin = jin;
}
inline void CUser::SetMu(uint16 mu)
{
    m_mu = mu;
}
inline void CUser::SetShui(uint16 shui)
{
    m_shui = shui;
}
inline void CUser::SetHuo(uint16 huo)
{
    m_huo = huo;
}
inline void CUser::SetTu(uint16 tu)
{
    m_tu = tu;
}
inline void CUser::SetXiangXinDian(uint16 xiangxindian)
{
    m_xiangxingdian = xiangxindian;
}
inline void CUser::SetHp(int hp)
{
    m_hp = hp;
}
inline void CUser::SetMp(int mp)
{
    m_mp = mp;
}

inline uint8 CUser::GetHead()
{
    return m_head;
}
inline void CUser::SetHead(uint8 head)
{
    m_head = head;
}
    
inline const char *CUser::GetMission(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == id)
        {
            return m_mission[i].mission.c_str();
        }
    }
    return NULL;
}
    
/*inline void CUser::SetStep(uint32 step)
{
    m_step = step;
}*/

inline uint32 CUser::GetStep()
{
    return m_step;
}

inline void CUser::AddStep(int step)
{
    atomic_exchange_and_add((int*)&m_step,step);
}

inline uint32 CUser::GetFightId()
{
    return m_fightId;
}

inline uint8 CUser::GetFightPos()
{
    return m_fightPos;
}

#endif

