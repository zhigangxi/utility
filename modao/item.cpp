#include "item.h"
#include "self_typedef.h"
#include "utility.h"
#include "net_msg.h"
#include <iostream>
using namespace std;
//附加属性
const uint16 ADD_ATTR[] = {500,500,500,500,500,150,3000,3000,5000,5000,500,100,100,100,100,100,30,3000,1500,100,3,100,100,100,10,10,10,10,100,3000,3000,3000,3000,3000,1000,3000,3000,3000,3000,3000,1000,100,3000,3000,1500,100,200,1500,100,100,800,50,50,
25,25,25,25,25,8,150,150,250,250,25,5,5,5,5,5,2,150,75,5,1,5,5,5,1,1,1,1,5,150,150,150,150,150,50,150,150,150,150,150,50,5,150,150,75,5,10,75,5,5,40,3,3,
5,5,5,5,5,2,30,30,50,50,5,1,1,1,1,1,1,30,15,1,1,1,1,1,1,1,1,1,1,30,30,30,30,30,10,30,30,30,30,30,10,1,30,30,15,1,2,15,1,1,8,1,1,
};

double GetQiangHuaBeiLv(uint8 level)
{
    double beiLv[] = {1.2,1.4,1.6,1.9,2.2,2.6,3,3.5,4,4.5,5,6,7,8,9};
    if(level < sizeof(beiLv)/sizeof(double))
    {
        return beiLv[level];
    }
    return 1;
}

int GetQiangHuaGaiLv(uint8 level)
{
    int gaiLv[] = {70,60,50,40,30,30,30,30,30,20,20,20,20,20,20};
    if(level < sizeof(gaiLv)/sizeof(int))
    {
        return gaiLv[level];
    }
    return 100;
}

int GetQiangHuaReturn(uint8 level)
{
    int retLevel[] = {0 ,0 ,1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,9 ,10,11,12,13};
    if(level < sizeof(retLevel)/sizeof(int))
    {
        return retLevel[level];
    }
    return level;
}

double GetStoneChengBen(uint8 level)
{
#ifdef QQ
    double chengBen[] = {2,6,18,54,162,486,1458,4374,13122,39366,118098,354294,1062882,3188646,9565938};
#else
    double chengBen[] = {3,9,27,81,243,729,2187,6561,19683,59049,177147,531441,1594323,4782969,14348907};
#endif
    
    if((level < 1) || (level > sizeof(chengBen)/sizeof(double)))
        return 0.0;
    return chengBen[level-1];
}

//获得升级成本
int GetLeveUpChengBen(uint8 level)
{
    int chengBen[] = {2,4,6,15,40,100,250,700,1500,3200,6000,9000,12000,15000,17000};
    if(level >= sizeof(chengBen)/sizeof(int))
        return 0;
    return chengBen[level];
}

//获得选绿装属性概率，duan为0－9（0－100划分为10段）
double GetSelectGreenAttrGaiLv(uint8 duan,uint8 stoneLevel)
{
    double duanGaiLv[] = {88661,28661,8661,3661,1661,911,536,318,175,75};//{115406 ,29692 ,9692 ,3692 ,1692 ,942 ,567 ,345 ,195 ,86};
    if(duan > 9)
        return 0;
    return duanGaiLv[duan] * GetStoneChengBen(stoneLevel);
}

//选择绿装是否成功
bool SelectGreenAttr(uint8 &duan,uint8 stoneLevel,uint8 stoneNum)
{
    int r = Random(0,600000);
    
    for(int i = 9; i >= 0; i--)
    {
        if(r < stoneNum*GetSelectGreenAttrGaiLv(i,stoneLevel))
        {
            duan = i;
            return true;
        }
    }    
    return false;
}

void GetAddAttrVal(int type,uint16 &min,uint16 &middle,uint16 &max)
{
    if(type > EAddAttrTypeNum)
        return;
    min = ADD_ATTR[type-1+2*EAddAttrTypeNum];
    max = ADD_ATTR[type-1];
    middle = ADD_ATTR[type-1+EAddAttrTypeNum];
}

bool IsFangJu(uint8 type)
{
    return ((type >= EITMaoZi) && (type <= EITXieZi));
}

bool IsJie(uint8 type)
{
    return type == EITXiangLian;
}

bool IsPei(uint8 type)
{
    return type == EITYuPei;
}

bool IsHuWan(uint8 type)
{
    return type == EITShouZhuo;
}

bool IsWuQi(uint8 type)
{
    return type <= EITGun;
}

bool CanAddAttr(uint8 attrType,uint8 itemType)
{
    switch(attrType)
    {
    case EAAliliang://力量(固定值)
	case EAAnaili://耐力(固定值)
	case EAAtizhi://体质(固定值)
	case EAAminjie://敏捷(固定值)
	case EAAlingxing://灵性(固定值)
	case EAAshanghai://伤害
	case EAAfangyu://防御
	case EAAfali://法力
	case EAAqixue://气血
	case EAAsudu://速度
	case EAAgongjijiangdimp://攻击技能MP消耗降低
	case EAAfuzhujiangdimp://辅助技能MP消耗降低
	case EAAzhangaijiangdimp://障碍技能MP消耗降低
	case EAAquanjiangdimp://全技能MP消耗降低
	case EAAjinkang://金抗性
	case EAAmukang://木抗性
	case EAAshuikang://水抗性
	case EAAhuokang://火抗性
	case EAAtukang://土抗性
	case EAAquankang://全抗性
	case EAAkangyiwang://抗遗忘
	case EAAkangzhongdu://抗中毒
	case EAAkangbingdong://抗冰冻
	case EAAkanghuishui://抗昏睡
	case EAAkanghunluan://抗混乱
	case EAAquankangyichang://全抗异常
	case EAAbaojizhuijia://爆击时追加伤害	
	case EAAfanjizhuijia://反击时追加伤害	
	case EAAfanzhendu://反震度	
	case EAAfashufantan://法术反弹
    case EAAFashufantanlv://法术反弹率
    case EAAlianjizhuijia://连击时追加伤害	
    case EAAfashubaojizhuijia://法术爆击追加	
    //防，佩，戒
    	if(IsFangJu(itemType) || IsPei(itemType) || IsJie(itemType))
    		return true;
        break;
    case EAAquanshuxing://全属性(固定值)
    case EAAjin://金相
	case EAAmu://木相
	case EAAshui://水相
	case EAAhuo://火相
	case EAAtu://土相
	case EAAquanxiangxing://全相性
	case EAAmingzhong://命中率
	case EAAhuibiwuli://回避物理攻击
	case EAAlianjilv://连击率
	case EAAbishalv://必杀率
	case EAAfanjilv://反击率
	case EAAfanzhenlv://反震率
	case EAAhuibifashugongji://回避法术攻击
	case EAAquanjineng://全技能（增加技能等级）
	case EAAhushikangxing://忽视所有抗性
	case EAAhushikangyichang://忽视所有抗异常
	case EAAaddbaojiweili://增加爆击威力	
	case EAAfashubaoji://法术爆击	
	case EAAlianjishu://连击数
    // 武，护腕
    	if(IsWuQi(itemType) || IsHuWan(itemType))
    		return true;
        break;

    }
    return false;
}

bool SItemInstance::AddKong(uint8 pos,uint8 type,uint16 val)
{
    for(uint8 i = 0; i < MAX_KAIJIA_ATTR_NUM; i++)
    {
        if(addAttrType[i] == type)
            return false;
    }
    if(pos < MAX_KAIJIA_ATTR_NUM)
    {
        addAttrType[pos] = type;
        addAttrVal[pos] = val;
        bangDing = 1;
        return true;
    }
    return false;
}

void SItemInstance::MakeKongInfo(CNetMessage &msg)
{
    uint8 kongNum[] = {
        1,0,0,
        1,1,0,
        2,1,0,
        2,2,0,
        2,2,1,
        3,2,1,
        3,2,2,
        3,3,2,
        3,3,3,
        3,3,3,
        3,3,3};
    int minId = 800;
    uint8 pos = tmplId - minId;
    if(pos >= sizeof(kongNum))
        return;
    pos *= 3;
    msg<<kongNum[pos];
    for(uint8 i = 0; i < kongNum[pos]; i++)
    {
        msg<<addAttrType[i]<<addAttrVal[i];
    }
    msg<<kongNum[pos+1];
    for(uint8 i = 0; i < kongNum[pos+1]; i++)
    {
        msg<<addAttrType[i+3]<<addAttrVal[i+3];
    }
    msg<<kongNum[pos+2];
    for(uint8 i = 0; i < kongNum[pos+2]; i++)
    {
        msg<<addAttrType[i+6]<<addAttrVal[i+6];
    }
}
