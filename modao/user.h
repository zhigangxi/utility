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
    EETMaoZi      = 0,// ñ��
    EETKuiJia     = 1,// ����
    EETYaoDai     = 2,// ����
    EETXieZi      = 3,// Ь��
    EETWuQi       = 4,// ����
    EETXiangLian  = 5,// ����
    EETYuPei      = 6,// ����
    EETShouZhuo1  = 7,// ����
    EETShouZhuo2  = 8,// ����
};

enum EUserTitle
{
    EUTZhuangYuan   = 1,//״Ԫ
    EUTBangYan      = 2,// ����
    EUTTanHua       = 3,// ̽��
    EUTShiFu        = 4,// ʦ��
    EUT5            = 5,// ��״Ԫ
    EUT6            = 6,// �İ���
    EUT7            = 7,// ��̽��
    EUT8            = 8,// �ľ���
    EUT9            = 9,//TID=9 Ӣ�� TID=10 MM  TID=11 GG
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
    void Move(uint8 direction,uint8 ste);//DIR=2 ���� DIR=4 ���� DIR=6 ���� DIR=8 ����
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
    
    //���ݹ�ʽ��ʼ����������
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
    
    void AddLevel(bool callScript = true);         //�ȼ�
    void AddExp(int exp,bool callScript = false);            //����
    void CallScriptLevelUp();
    
    void AddTiZhi(int tizhi);        //����
    void AddLiLiang(int liliang);    //����
    void AddMinJie(int minjie);      //����
    void AddLingLi(int lingli);      //����
    void AddNaiLi(int naili);        //����
    void AddShuXinDian(int shuxindian);//δ�������Ե�
    void AddJin(int jin);            //������
    void AddMu(int mu);              //ľ����
    void AddShui(int shui);          //ˮ����
    void AddHuo(int huo);            //������
    void AddTu(int tu);              //������
    void AddXiangXinDian(int xiangxindian);  //δ�������Ե�
    void AddHp(int hp);              //��Ѫ
    void AddMp(int mp);              //����
    void AddStep(int step);
    void SetMoveTime(uint64 t);
    uint64 GetMoveTime();
    void SetErrMoveTimes(uint8 t);
    uint8 GetMoveErrTimes();
    
    int GetDamage();//�����˺�
    int GetSkillDamage(int skillId);//���������˺�
    int GetRecovery();//����
    int GetSpeed();//�ٶ�
    
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
    
    //pos�����е�λ�ã�����װ���е�λ�ã�< 0��ʾװ��ʧ��
    int EquipItem(uint8 pos,string &errMsg);
    //posװ���е�λ�ã����ذ����е�λ�ã�< 0��ʾжװ��ʧ��
    int UnEquipItem(uint8 pos);
    
    int GetItemfangYu();//����
    int GetItemGongji();//������
    int GetItemFaLi();//���ӷ���
    int GetItemQiXue();//������Ѫ
    int GetItemSudu();//�����ٶ�
    
    int GetItemAddAttr(EAddAttrType type);
    
    int GetItemLiliang();//����(�̶�ֵ)
    int GetItemLingli();//����(�̶�ֵ)
    int GetItemMinjie();//����(�̶�ֵ)
    int GetItemTizhi();//����(�̶�ֵ)
    int GetItemNaili();//����(�̶�ֵ)	
    
    int GetItemJin();//����
    int GetItemMu();//ľ��
    int GetItemShui();//ˮ��
    int GetItemHuo();//����
    int GetItemTu();//����	
    
    /*
    ������=����*0.00006+����*0.0007+��ϵ���ӣ���ϵ����1%��+װ������
    ��������=����*0.00006+ľ��*0.0007+ľϵ���ӣ�ľϵ����1%��+װ������
    ����������=����*0.00006+����*0.0007+��ϵ���ӣ���ϵ����1%��+װ������
    ������=����*0.00006+ˮ��*0.0007+ˮϵ���ӣ�ˮϵ����1%��+װ������
    ������=����*0.00006+����*0.0007+��ϵ���ӣ���ϵ����1%��+װ������
    */

    int GetItemMingzhong();//������
    int GetItemHuibi();//�ر�������
    int GetItemLianjiLv();//������
    int GetItemLianjiShu();//������
    int GetItemBaoJiLv();//��ɱ��
    int GetItemFanJiLv();//������
    int GetItemFanZhenLv();//������
    
    int GetItemGongjiMpJiangDi();//��������MP���Ľ���
    int GetItemFuZhuMpJiangDi();//��������MP���Ľ���
    int GetItemZhangAiMpJiangDi();//�ϰ�����MP���Ľ���
    
    int GetItemHuiBiFaGong();//�رܷ�������
    
    //�����˺�����
    int GetItemKangJin();//����
    int GetItemKangMu();//ľ����
    int GetItemKangShui();//ˮ����
    int GetItemKangHuo();//����
    int GetItemKangTu();//������
    
    //���������н���
    int GetItemKangYiWang();//������
    int GetItemKangZhongDu();//���ж�
    int GetItemKangBingDong();//������
    int GetItemKangHunShui();//����˯
    int GetItemKangHunLuan();//������
    
    int GetItemJiNeng();//ȫ����
    int GetItemHuShiAllKang();//�������п���
    int GetItemHuShiAllKangYiChang();//�������п��쳣	

    int GetItemBaoJiZhuiJIa();
    int GetAddBaoJiWeiLi();
    int GetLianJiAddShangHai();
    int GetFanJiAddShangHai();
    int GetFanZhenDu();
    int GetFaShuBaoJi();
    int GetFaShuBaoJiAdd();
    int GetFaShuFanTan();//�ٷֱ�
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
    
    void AddAskForMatchUser(uint32 id)//������id
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
    
    //ս����ֻ��ʹ��ҩƷ���˺����ڲ���ҩƷʵ��Ч�������غ����ʵ�����ѡ��Ӹ�˭
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
    bool AddMission(int id,const char *pMiss); //����
    void UpdateMission(int id,const char *pMiss);//��������
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
    
    //188��ɱ��189��ֵ100��ͨ��
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
    uint16 PutItemToShop(uint8 type,uint8 &pos,uint8 num,int money);//������Ʒ������id
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
    void DismissBang();// ��ɢ����
    void UndismissBang();// �����ɢ״̬

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
    TYPE=1 ����
    TYPE=2 ս��
    TYPE=3 �ﹱ
    TYPE=4 ���Ե�
    TYPE=5 ���Ե�
    TYPE=6 Ǳ��
    TYPE=7 ����
    TYPE=9 ��Ѫ
    TYPE=10 �����Ѫ
    TYPE=11 mp
    TYPE=12 ���mp
    TYPE=13 �˺�
    TYPE=14 �ٶ�
    TYPE=15 ����
    TYPE=16 ����
    TYPE=17 ����
    TYPE=18 ����
    TYPE=19 ����
    TYPE=20 ����
    TYPE=21 ��
    TYPE=22 ľ
    TYPE=23 ˮ
    TYPE=24 ��
    TYPE=25 ��
    TYPE=26 ��Ǯ
    TYPE=27 ͨ��
    TYPE=28 ����
    TYPE=29 ���� 
    TYPE=30 �ر�
    TYPE=31 ����������
    TYPE=32 ����������
    TYPE=33 ������
    TYPE=34 ������
    TYPE=35 ������
    TYPE=36 ������
    TYPE=37 ������
    TYPE=38 ������
    TYPE=40 ħ��
    TYPE=42 ����
    TYPE=43 �������ı�������
    TYPE=44 ״̬
    TYPE=45 val��0����ʧЧ��1�ֿ�ʧЧ��2����ֿ�ʧЧ
    TYPE=46 ���°�ͨ��
    TYPE=47 ��һ����ȡ���ʱ��
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
    
    //��ѯװ��ǿ�����
    bool MakeQiangHuaInfo(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num,CNetMessage &msg);
    
    //0�ɹ���1��ʧ�ܣ�2����װ��
    int QiangHuaPackage(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num);
    
    //0�ɹ���1��ʧ�ܣ�2��������,stone1,stone2,stone3Ϊ��ˮ��λ��
    int LianHuaPackage(uint8 itemPos,uint8 stone1,uint8 stone2,uint8 stone3);
    
    //0�ɹ���1��ʧ�ܣ�2����������tianHuanShi�컻ʯ(623)Ϊλ��(),lianHuaShi����ʯλ��(611)
    int TianHuanPackage(uint8 itemPos,uint8 tianHuanShi);//,uint8 lianHuaShi);
    
    //0�ɹ���1��ʧ�ܣ�2��������
    int MakeGreenItem(uint8 item,uint8 stone);
    
    //0�ɹ���1��ʧ�ܣ�2����װ��
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
    //0�ɹ���1��ʧ�ܣ�2��������
    int RepairItem(uint8 pos);
    //�;������=ȡ����װ���ȼ�*Ҫά�޵��;�*0.1��+1
    //�����������װ������
    int GetRepairFee();
    void RepairTotel();
    
    void SetSaveVal(uint8 index,int val);
    int GetSaveVal(uint8 index);
    void SetSaveVal(char *msg);
    
    
    bool DelPackageById(int id,int num);
    int GetItemNum(int id);
    
    //���ý��볡���Ƿ���ýű�
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
    
    //false���ܼ���
    int JianDingItem(uint8 pos);
    
    //������Ʒ
    bool JianDingShiPin(uint8 pos);
    
    //��Ʒ��Ƕ
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
    //waitTime��λΪ����
    void AddTimer(int id,int waitTime);
    void DelTimer(int id);
    
    bool CompoundStone(uint8 pos,uint8 num);
    void GetViewPara(int &fagong,uint16 &mingzhong,uint16 &huibi);
    
    //�ϳ���Ʒ�����غϳ���Ʒid��0ʱ��ʾ�ϳ�ʧ��
    int CompoundItem(uint8 pos1,uint8 pos2,uint8 pos3);
    int CompoundItem1(int id);
    
    //����δ��װ��
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
    TYPE=1 ״̬
   TYPE=2 ���Ե�
   TYPE=3 ��ѧ
   TYPE=4 ����
   TYPE=5 ��Ѫ
   TYPE=6 ����
   TYPE=7 ����
   TYPE=8 ����
   TYPE=9 ����
   TYPE=10 ����
   TYPE=11 ����
   TYPE=12 ����
   TYPE=13 �ҳ϶�
   TYPE=14 ���ܶ�
   
   TYPE=15 �����Ѫ
   TYPE=16 �����
   TYPE=17 �﹥
   TYPE=18 ����
   TYPE=19 �ٶ�
   TYPE=20 ����
   TYPE=21 �ƶ��ٶ�
   TYPE=22 �����;�
   TYPE=23 ��Ϊ
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
        return m_fewMonsterEnd > time(NULL);//���в�������
    }
    bool IsMaxMonster()
    {
        return m_maxMonsterEnd > time(NULL);//�������(������
    }
    bool IsMinMonster()
    {
        return m_minMonsterEnd > time(NULL);//��������(
    }
    bool IsUpMonster()
    {
        return m_upMonsterEnd > time(NULL);//ֻ����>=����ȼ��Ĺ�
    }
    //type 0 ͨ����1��ͨ��
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
    
    //�������Ʒ
    bool AddBangDingPackage(int itemId,int num = 1,const char *name = NULL);
    //������ˮ��          ����      ����ֵ
    bool AddBlueCrystal(int attr,int attrVal);
    //��������ʯ
    bool AddLianHuaStone(int level,int num = 1);
    //����ָ���ȼ�ǿ��װ��
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
    //0���ɹ���1 û�г��������2 ����ǰ󶨡�3 ���ʱ��δ��
    int PetJieBang(uint8 petPos);
    
    int GetLeftDoubleTime();
    void SetDouble(int hour);
    
    //�򿪱�����1��������2���ĸ�
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
    //0 ʦ���ȼ�
    //1 ������������
    //��ȡ���ʣ��ʱ��
    //3��ʦͽ������
    //4,5,8�ű�ʹ��
    //6 �������״̬
    //7,8�ű�ʹ��
    //9 ��pk����
    //10,11,12,13,14�����ս��ľˮ����
    //15�ű�ʹ��
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
    //0 �������ƶ�
    //1 ��������
    //2,5 �ű�ʹ��
    //3 �����ս����
    //4 ÿ������ʱ��
    //6 �ܻ���
    //7 ÿ�ջ���
    //8 ��սʮ��ʹ��ʱ�� Ӣ��
    //10,11�ű�ʹ��
    //12 ��������
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
    //0 ��ɢʦͽ��ϵʱ��
    //1 ״Ԫ��̽��������titleʱ��
    //2 �뿪����ʱ��
    //3 ����id
    //4 ͵��ʱ��
    //5 �ﹱ
    //6 ���roleid
    //7 ���ʱ��
    //8 ���ʱ��
    //9 �ֲ˻��֣��ﹱ��
    //10 ����ֵ
    //11 ���븱��306ʱ��
    //12 ����ֵ
    //13��ݽ�ɢʱ��
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
    //���,0�ɹ���1 ���ʯ���� 
    int ChaiFen(uint8 weaponPos,uint8 attrPos,uint8 stonePos);
    //�ںϣ�0�ɹ���1 ʯͷ���� 
    int RongHe(uint8 weaponPos,uint8 stonePos);
    
    int ChaiFenSX(uint8 weaponPos,uint8 attrPos);
    void SetShiFu();
    
    int GetHeChengVal(uint8 pos1,uint8 pos2);
    bool HeChengLanShuiJing(uint8 pos1,uint8 pos2,uint8 hechengfu);
    
    //�ϳɳ�������
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
    
    //����ѧϰ����
    bool PetStudySkill(uint8 petPos,uint16 skillId);
    
    //װ����������
    bool PetKaiJia(uint8 petPos,uint8 kaiJiaPos);
    
    //�õ�����λ��
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
    
    //type = 3��ʾ�����
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
    //ս��������ʹ���Զ���Ѫ����ҩ
    void UseFightEnd();
    
    //0���ı������ݡ�1 �ֿ����ݣ�2 ����ֿ�����
    //3����ʹ�ã�ÿ����
    uint8 GetExtData8(uint16 pos);
    void SetExtData8(uint16 pos,uint8 val);
    
    //0�����Ʒ
    uint16 GetExtData16(uint16 pos);
    void SetExtData16(uint16 pos,uint16 val);
    
    //0���ı������ݡ�1 �ֿ����ݣ�2 ����ֿ����ݽ���ʱ��
    uint32 GetExtData32(uint16 pos);
    void SetExtData32(uint16 pos,uint32 val);
    
    void MakeBankPet(CNetMessage &msg);
    
    //pos����λ��
    bool SaveBankPet(uint8 pos);
    
    //pos�ڳ����̵��е�λ��
    bool GetBankPet(uint8 pos);
    
    void QueryTextTitle(CNetMessage &msg);
    
    void AddTextTitle(const char *pTitle);
    void DelTextTitle(const char *pTitle);
    void UseTextTitle(const char *pTitle);
    bool HaveAllWanMei();
    bool HaveLevel12Equip();
    void GetGift(uint8 type=0);
    
    time_t m_yewaiShiYao;//Ұ��ʮ��ʱ��
private: 
    bool NewUserGift();//���ֱ���
    bool NormalUserGift();//ÿ�ձ���
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
    time_t m_giftTime;//��ȡ����ۼ�ʱ��
    time_t m_chatTime;
    time_t m_answerTime;//�ش�����ʱ��
    time_t m_askTime;
    time_t m_lastHeartTime;
    time_t m_activityTime;
    bool m_readIgnore;
    
    int m_XZBangId;
    
    uint8 m_heartTimes;
    uint8 m_rightAnswer;
    uint8 m_answerTimes;//�ش�Ĵ���
    uint8 m_admin;
    uint8 m_heartErrTimes;
    
    struct SUserGift
    {
        uint8 type;//1��Ʒ��2���顢3Ǳ�ܣ�4����
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
    //�����ʹ�ýű������ؽű�id
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
    
    //add = trueΪ�ӣ�addΪfalseΪ��
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
    
    uint16 m_title;//Ŀǰʹ�õ�title
    
    struct STitleData
    {
        uint16 id;
        time_t begin;//����ʱ��
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
    
    //�Զ�ս����������
    uint8 m_userOp;
    int  m_userPara;
    uint8 m_target;
    uint8 m_petOp;
    int m_petPara;
    uint8 m_petTar;
    
    uint8 m_save8[UINT8_NUM];
    uint16 m_save16[UINT16_NUM];
    uint32 m_save32[UINT32_NUM];
    
    //0���ı������ݡ�1 �ֿ����ݣ�2 ����ֿ�����
    map<uint16,uint8> m_saveData8;
    
    map<uint16,uint16> m_saveData16;
    
    //0���ı������ݡ�1 �ֿ����ݣ�2 ����ֿ����ݽ���ʱ��
    map<uint16,uint32> m_saveData32;
    
    uint32 m_guanFight;//��ս��ս��
    
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
    uint32 m_shopItemId[MAX_SHOP_ITEM_NUM];//�洢shop��Ʒ�Ķ�̬id
    
    uint8 m_petNum;//��������
    uint8 m_chuZhanPet;//��ս����
    uint8 m_gensuiPet;//�������
    uint8 m_qiPet;//���
    
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
    
    time_t m_fewMonsterEnd;//���в�������
    time_t m_maxMonsterEnd;//�������
    time_t m_minMonsterEnd;//��������
    time_t m_upMonsterEnd;//ֻ����>=����ȼ��Ĺ�
    time_t m_fightEndTime;
    
    time_t m_addTiliTime;
    time_t m_pkTime;
    
    
    time_t m_sDoubleEnd;//�ű�����˫��ʱ��
    uint32 m_tempLeaveTeam;//��ʱ�뿪����
    
    uint8 m_chatChannel;
    const static int MAX_SAVE_NUM = 12;
    
    uint32 m_shortArray[MAX_SAVE_NUM];
    //MAX_SAVE_NUM-2:Ϊ��������
    //MAX_SAVE_NUM-1,Ϊ��¼ÿ��һ��Ҫ���µı���
        
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

