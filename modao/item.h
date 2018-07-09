#ifndef _ITEM_H_
#define _ITEM_H_
#include <string>
#include "self_typedef.h"
#include "hash_table.h"
using namespace std;
class CCallScript;
class CNetMessage;

enum EAddAttrType//附加属性类型
{
	EAAliliang = 1,//力量(固定值)
	EAAnaili,//耐力(固定值)
	EAAtizhi,//体质(固定值)
	EAAminjie,//敏捷(固定值)
	EAAlingxing,//灵性(固定值)
	
	EAAquanshuxing,//全属性(固定值)
	
	EAAshanghai,//伤害
	EAAfangyu,//防御
	EAAfali,//法力
	EAAqixue,//气血
	EAAsudu,//速度
	
	EAAjin,//金相
	EAAmu,//木相
	EAAshui,//水相
	EAAhuo,//火相
	EAAtu,//土相
	EAAquanxiangxing,//全相性
	
	EAAmingzhong,//命中率
	EAAhuibiwuli,//回避物理攻击
	EAAlianjilv,//连击率
	EAAlianjishu,//连击数
	EAAbishalv,//必杀率
	EAAfanjilv,//反击率
	EAAfanzhenlv,//反震率
	EAAgongjijiangdimp,//攻击技能MP消耗降低
	EAAfuzhujiangdimp,//辅助技能MP消耗降低
	EAAzhangaijiangdimp,//障碍技能MP消耗降低
	EAAquanjiangdimp,//全技能MP消耗降低
	EAAhuibifashugongji,//回避法术攻击
	EAAjinkang,//金抗性
	EAAmukang,//木抗性
	EAAshuikang,//水抗性
	EAAhuokang,//火抗性
	EAAtukang,//土抗性
	EAAquankang,//全抗性
	EAAkangyiwang,//抗遗忘
	EAAkangzhongdu,//抗中毒
	EAAkangbingdong,//抗冰冻
	EAAkanghuishui,//抗昏睡
	EAAkanghunluan,//抗混乱
	EAAquankangyichang,//全抗异常
	EAAquanjineng,//全技能（增加技能等级）
	EAAhushikangxing,//忽视所有抗性
	EAAhushikangyichang,//忽视所有抗异常
	
	//物理爆击基础1.5
	//法术爆击基础1.2
	EAAbaojizhuijia,//爆击时追加伤害	
	EAAaddbaojiweili,//增加爆击威力	增加百分比，在爆击
	EAAlianjizhuijia,//连击时追加伤害	
	EAAfanjizhuijia,//反击时追加伤害	
	EAAfanzhendu,//反震度	
	EAAfashubaoji,//法术爆击	
	EAAfashubaojizhuijia,//法术爆击追加	
	EAAfashufantan,//法术反弹
    EAAFashufantanlv,//法术反弹率
    
    EAddAttrTypeEnd,
	EAddAttrTypeNum = EAddAttrTypeEnd -1
};

const uint8 BLUE_EQUIP[] = 
{
    EAAliliang,//力量(固定值)
	EAAnaili,//耐力(固定值)
	EAAtizhi,//体质(固定值)
	EAAminjie,//敏捷(固定值)
	EAAlingxing,//灵性(固定值)
	
    EAAshanghai,//伤害
	EAAfangyu,//防御
	EAAfali,//法力
	EAAqixue,//气血
	EAAsudu,//速度
	
	EAAjin,//金相
	EAAmu,//木相
	EAAshui,//水相
	EAAhuo,//火相
	EAAtu,//土相
	
	EAAmingzhong,//命中率
	EAAhuibiwuli,//回避物理攻击
	EAAlianjilv,//连击率
	
    EAAbishalv,//必杀率
	EAAfanjilv,//反击率
	EAAfanzhenlv//反震率
};

inline bool InBlueEquip(uint8 attr)
{
    for(uint8 i = 0; i < sizeof(BLUE_EQUIP); i++)
    {
        if(BLUE_EQUIP[i] == attr)
            return true;
    }
    return false;
}

const uint8 GREEN_EQUIP[] = 
{
    EAAquanshuxing,//全属性(固定值)
    EAAquanxiangxing,//全相性
    EAAlianjishu,//连击数
    EAAgongjijiangdimp,//攻击技能MP消耗降低
	EAAfuzhujiangdimp,//辅助技能MP消耗降低
	EAAzhangaijiangdimp,//障碍技能MP消耗降低
	EAAquanjiangdimp,//全技能MP消耗降低
	//EAAhuibifashugongji,//回避法术攻击
	EAAjinkang,//金抗性
	EAAmukang,//木抗性
	EAAshuikang,//水抗性
	EAAhuokang,//火抗性
	EAAtukang,//土抗性
	EAAquankang,//全抗性
	EAAkangyiwang,//抗遗忘
	EAAkangzhongdu,//抗中毒
	EAAkangbingdong,//抗冰冻
	EAAkanghuishui,//抗昏睡
	EAAkanghunluan,//抗混乱
	EAAquankangyichang,//全抗异常
	//EAAquanjineng,//全技能（增加技能等级）
	EAAhushikangxing,//忽视所有抗性
	EAAhushikangyichang,//忽视所有抗异常
	
	EAAbaojizhuijia,//爆击时追加伤害	
	EAAaddbaojiweili,//增加爆击威力	
	EAAlianjizhuijia,//连击时追加伤害	
	EAAfanjizhuijia,//反击时追加伤害	
	EAAfanzhendu,//反震度	
	EAAfashubaoji,//法术爆击	
	EAAfashubaojizhuijia,//法术爆击追加	
	EAAfashufantan,//法术反弹
    EAAFashufantanlv//法术反弹率
};

enum EItemType
{
    EITQiang = 1	,//枪
    EITZhua = 2	,//爪
    EITJian = 3	,//剑
    EITDao = 4	,//刀
    EITGun = 5	,//棍
    
    EITMaoZi = 6	,//帽子
    EITYiFu = 7	,//衣服
    EITYaoDai = 8,//腰带
    EITXieZi = 9,//鞋子
    
    EITXiangLian = 10,//戒,增加法力
    EITYuPei = 11,//佩，增加气血
    EITShouZhuo = 12,//护腕，增加攻击
    
    EITAddAttr = 13,//带等级石头13
    EITNormalYaoPin = 14,//非pk药
    EITNvWaShi = 15,//女娲石15
    EITPKYaoPin = 16,//pk药
    
    EITTeShu = 17,//特殊物品17
    
    EITMission = 18,//任务物品
    
    EITChargeItem = 19,//收费道具,可叠加
    
    EITMissionCanSave = 20,//
    
    EITPetBook = 21,//宠物天书
    EITPetKaiJia = 22,//宠物铠甲
    
    EITCanDelMiss = 23,//可丢弃任务物品
    
    EItemDieJiaNum = 20,//物品叠加数量
    
    EBankItemDieJIaNum = 99//
};

//品质类型
enum EQualityType
{
    EQTWhite,//白装
    EQTBlue,//蓝装
    EQTGreen,//绿装
};

struct SItemTemplate
{
    SItemTemplate():
        type(0),//种类
        level(0),//需求等级	
        mod(0),//模式
        sex(0),//性别
        //canDieJia(false),
        fangYu(0),//防御
        id(0),
        gongji(0),//攻击力
        naijiu(0),//耐久
        addFaLi(0),//增加法力
        addQiXue(0),//增加气血
        addSudu(0),//增加速度
        pic(0),//图片
        addXue(0),//增加血量
        jiage(0),//价格
        pScript(NULL)
    {
    }
    
    uint8 type;//种类
    uint8 level;//需求等级	
    uint8 mod;//模式
    uint8 sex;//性别
    //bool canDieJia;//是否可叠加
    uint16 fangYu;//防御
    uint16 id;
    uint16 gongji;//攻击力
    uint16 naijiu;//耐久
    uint16 addFaLi;//增加法力
    uint16 addQiXue;//增加气血
    uint16 addSudu;//增加速度
    uint16 pic;//图片
    
    //当物品属性为EITAddAttr时，addXue按位表示拆分武器种类
    int addXue;//增加血量
    
    int jiage;//价格    
    string name;//名字
    string describe;//说明
    CCallScript *pScript;//特殊物品脚本
};

/***********
0x1 hp百分比
0x2 hp固定值
0x4 mp百分比
0x8 mp百分比
************/
enum EItemModType
{
    EIMTHpPer = 1,
    EIMTHpFix = 2,
    EIMTMpPer = 4,
    EIMTMpFix = 8,
    EIMTZhongcheng = 16,//恢复宠物忠诚度
    EIMTShouming = 32,//恢复宠物寿命
    EIMTJiaoYiBang = 64,//交易后绑定		
    EIMTName = 128,//带名字物品		
};

/********
1-体质
2-力量
3-敏捷
4-灵力
5-耐力
*********/
enum EReqAttrType//属性需求类型
{
    ERATtizhi = 1,
    ERATliliang,
    ERATminjie,
    ERATlingli,
    ERATnaili,
};


struct SItemInstance
{
    const static int MAX_ADD_ATTR_NUM  =  6;
    const static int MAX_KAIJIA_ATTR_NUM = 9;
    const static char CAN_NOT_XIE_XIA = 'n';//不可卸下装备
    uint8   num;
    uint8   level;//强化等级
    uint8   strengthenLevel;//强化需求
    uint8   quality:4;//品质
    uint8   bangDing:4;//是否绑定，0：不绑定，1：绑定
    uint8   addAttrNum;//附加属性数量
    uint8   addAttrType[MAX_KAIJIA_ATTR_NUM];//附加属性类型
    uint8   reqAttrType;//属性需求类型
    uint16  reqAttrVal;//属性需求值
    uint16  addAttrVal[MAX_KAIJIA_ATTR_NUM];//附加属性值
    uint16  naijiu;//耐久
    uint16 tmplId;
    char name[MAX_NAME_LEN];
    
    bool operator == (const SItemInstance &item)
    {
        return memcmp(&level,&(item.level),sizeof(SItemInstance)-sizeof(num)) == 0;
    }
    bool AddKong(uint8 pos,uint8 type,uint16 val);
    void MakeKongInfo(CNetMessage &msg);
    
    int GetAddAttrType(uint8 pos)
    {
        if(pos < MAX_KAIJIA_ATTR_NUM)
            return addAttrType[pos];
        return 0;
    }
    int GetAddAttrVal(uint8 pos)
    {
        if(pos < MAX_KAIJIA_ATTR_NUM)
            return addAttrVal[pos];
        return 0;
    }
    int GetItemValue()
    {
        int *p = (int*)addAttrVal;
        return *p;
    }
};


class CItemTemplateManager
{
public:
    //pItem必须是使用new分配出来的
    void AddItem(SItemTemplate *pItem)
    {
        m_itemTemplate.Insert(pItem->id,pItem);
    }
    SItemTemplate *GetItem(uint16 id)
    {
        SItemTemplate *pItem = NULL;
        m_itemTemplate.Find(id,pItem);
        return pItem;
    }
private:
    CHashTable<uint16,SItemTemplate*> m_itemTemplate;
};

double GetQiangHuaBeiLv(uint8 level);
int GetQiangHuaGaiLv(uint8 level);
int GetQiangHuaReturn(uint8 level);
//int GetLi
void GetAddAttrVal(int type,uint16 &min,uint16 &middle,uint16 &max);

//判断附加属性是否可以加到此类型物品
bool CanAddAttr(uint8 attrType,uint8 itemType);

//选择绿装是否成功
bool SelectGreenAttr(uint8 &duan,uint8 stoneLevel,uint8 stoneNum);

//获得炼化石成本
double GetStoneChengBen(uint8 level);

//获得升级成本
int GetLeveUpChengBen(uint8 level);

const int MAX_QIANGHUA_LEVEL = 15;

#endif
