#include "utility.h"
#include "monster.h"
#include "singleton.h"
#include <iostream>
using namespace std;

void SMonsterInst::Init(SMonsterTmpl *pTmpl,bool initXiang)
{
    pMonster = pTmpl;
    
    if(initXiang)
    {
        if(xiang == 0)
        {
            liliang = 5 * level;
            lingxing = 3 * level;
        }
        else
        {
            liliang = 3 * level;
            lingxing = 5 * level;
        }
    }
    
    int addHp;
    if(type == EMTTongLing)
        addHp = pTmpl->maxHp - 40;
    else
        addHp = Random(pTmpl->maxHp-50,pTmpl->maxHp - 40);
    
    //气血：	(等级*5+(体质-等级))*(40*血成长/100)	
    maxHp = (int)((level*5+(tizhi-level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    recovery = (int)((level*5+(naili-level))*(4*addHp/100.0) + 60);
        
    hp = maxHp;
    
    int addMp;
    if(type == EMTTongLing)
        addMp = pTmpl->maxMp - 40;
    else
        addMp = Random(pTmpl->maxMp-50,pTmpl->maxMp - 40);
    //法力：	(等级*5+(灵性-等级))*(12*法力成长/100)	
    maxMp = (int)((level*5+(lingxing-level)) * (12*addMp/100.0) + 80);
    
    mp = maxHp;
    
    int addSpeed;
    if(type == EMTTongLing)
        addSpeed = pTmpl->maxSpeed-40;
    else
        addSpeed = Random(pTmpl->maxSpeed-50,pTmpl->maxSpeed-40);
        
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    speed = (int)(level*(2.2+minjie*0.05)+(addSpeed-level)*1.5+10);
    
    int addAttack;
    if(type == EMTTongLing)
        addAttack = pTmpl->maxAttack-40;
    else
        addAttack = Random(pTmpl->maxAttack-50,pTmpl->maxAttack-40);
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    attack = (int)((level*5+(liliang-level))*(12*addAttack/100.0)+80);
    
    if(type == EMTTongLing)
        addSkillAttack = pTmpl->minSkillAttack + 10;
    else
        addSkillAttack = Random(pTmpl->maxSkillAttack-50,pTmpl->maxSkillAttack-40);
    daohang = level*level*level;
}

ShareMonsterPtr CMonsterManager::CreateMonster(uint32 tmplId,uint8 level,EMonsterType type,const char *name)
{
    ShareMonsterPtr ptr;
    SMonsterTmpl *pTmpl = GetTmpl(tmplId);
    if(pTmpl == NULL)
    {
        return ptr;
    }
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = type;
    pInst->tmplId = tmplId;
    pInst->xiang = pTmpl->xiang;
    
    pInst->name = pTmpl->name;
    pInst->level = level;
    if(type == EMTBaoBao)
    {
        pInst->level = 1;
        pInst->name.append("宝宝");
    }
        
    pInst->exp = pTmpl->exp;
    
    pInst->liliang = pInst->level;
    pInst->lingxing = pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    
    pInst->Init(pTmpl);
    
    if(type == EMTTongLing)
    {
        pInst->maxHp *= 2;
        pInst->hp = pInst->maxHp;
        pInst->name.append("头领");
    }            
    if(name != NULL)
    {
        pInst->name = name;
    }
    
    return ptr;
}

ShareMonsterPtr CMonsterManager::CreateMonster(uint32 tmplId,EMonsterType type,bool useMaxLevel,bool useMinLevel)
{
    ShareMonsterPtr ptr;
    SMonsterTmpl *pTmpl = GetTmpl(tmplId);
    if(pTmpl == NULL)
    {
        return ptr;
    }
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = type;
    pInst->tmplId = tmplId;
    pInst->xiang = pTmpl->xiang;
    
    pInst->name = pTmpl->name;
    if(useMaxLevel)
        pInst->level = pTmpl->maxLevel;
    else if(useMinLevel)
        pInst->level = pTmpl->minLevel;
    else
        pInst->level = Random(pTmpl->minLevel,pTmpl->maxLevel);
    if(type == EMTBaoBao)
    {
        pInst->level = 1;
        pInst->name.append("宝宝");
    }
    else if(type == EMTTongLing)
    {
        pInst->level = pTmpl->maxLevel;
    }
    
    pInst->exp = pTmpl->exp;
    /**********************
    int tol = 4*pInst->level;
    int ave = tol/5;
    int dian[4] = {0};
    int ave10 = CalculateRate(ave,10,100);
    dian[0] = ave - 2*ave10;
    dian[1] = ave - 1*ave10;
    dian[2] = ave;
    dian[3] = ave + 1*ave10;
    pInst->liliang = RandSelect(dian,4);
    pInst->lingxing = RandSelect(dian,4);
    pInst->tizhi = RandSelect(dian,4);
    pInst->naili = RandSelect(dian,4);
    pInst->minjie = tol - pInst->liliang - pInst->lingxing - pInst->tizhi - pInst->naili;
    ***********************/
    pInst->liliang = pInst->level;
    pInst->lingxing = pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    
    //pInst->qinmi = 0;
    
    pInst->Init(pTmpl);
    
    if(type == EMTTongLing)
    {
        pInst->maxHp *= 2;
        pInst->hp = pInst->maxHp;
        pInst->name.append("头领");
    }
    
    return ptr;
}

void SPet::AddLevel()
{
    level++;
    tizhi++;
    liliang++;
    minjie++;
    lingxing++;
    naili++;
    shuxingdian +=5;
    zhongcheng = 100;
    
    SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(tmplId);
    if(pTmpl != NULL)
        Init();//(pTmpl);
    hp = maxHp;
    mp = maxMp;    
}

void SMonsterInst::AddSkill(uint16 skillId,uint16 level,uint8 useTimes)
{
    for(uint8 i = 0; i < MAX_SKILL_NUM; i++)
    {
        if(skillIds[i] == 0)
        {
            skillIds[i] = skillId;
            skillLevels[i] = level;
            skillUseTimes[i] = useTimes;
            skillAttackNum[i] = 4;
            return;
        }
    }
}

uint16 SMonsterInst::GetSkillLevel(uint16 skillId)
{
    for(uint8 i = 0; i < MAX_SKILL_NUM; i++)
    {
        if(skillIds[i] == skillId)
        {
            return skillLevels[i];
        }
    }
    return 0;
}
uint8 SMonsterInst::UseSkill(uint16 skillId)
{
    for(uint8 i = 0; i < MAX_SKILL_NUM; i++)
    {
        if(skillIds[i] == skillId)
        {
            if(skillUseTimes[i] > 0)
            {
                skillUseTimes[i]--;
                //cout<<(int)skillUseTimes[i]<<endl;
                return skillUseTimes[i];
            }
            return 0;
        }
    }
    return 0;
}

uint8 SMonsterInst::GetSkillNum()
{
    uint8 num = 0;
    for(uint8 i = 0; i < MAX_SKILL_NUM; i++)
    {
        if(skillIds[i] != 0)
        {
            num++;
        }
    }
    return num;
}
uint8 SMonsterInst::GetSkillAttackNum(uint16 skillId)
{
    for(uint8 i = 0; i < MAX_SKILL_NUM; i++)
    {
        if(skillIds[i] == skillId)
        {
            return skillAttackNum[i];
        }
    }
    return 0;
}
uint16 SMonsterInst::RandSelSkill()
{
    uint8 num = GetSkillNum();
    return skillIds[Random(0,num-1)];
}

int SMonsterInst::GetSkillDamage(int skillId)
{
    int base = 0;
    uint8 jie = GetSkillJie(skillId);
    switch(jie)
    {
    case 1:
        base = 40;
        break;
    case 2:
        base = 30;
        break;
    case 3:
        base = 40;
        break;
    case 4:
        base = 60;
        break;
    }
    uint8 skillLevel = GetSkillLevel(skillId);
    if(skillLevel == 0)
        skillLevel = level;
    //(等级*4+(灵性-等级)*1.2))*(法攻成长/10)+80+法术基础伤害*法术等级/3
    return (int)((skillLevel*4+(lingxing-level)*1.2)*addSkillAttack/10.0+80+base*skillLevel/3.0);   
    //return (int)((skillLevel*4+(GetLingXing()-level)*1.2)*addSkillAttack/10.0+80+base*skillLevel/3.0);   
}

void SMonsterInst::SetCeLue(uint16 cl)
{
    celue = cl;
}

uint16 SMonsterInst::GetCeLue()
{
    return celue;
}

//1 40,2 30,3 40,4 60
int SPet::GetSkillDamage(int skillId)
{
    int base = 0;
    uint8 jie = GetSkillJie(skillId);
    switch(jie)
    {
    case 1:
        base = 40;
        break;
    case 2:
        base = 30;
        break;
    case 3:
        base = 40;
        break;
    case 4:
        base = 60;
        break;
    }
    int skillAttackCZTmp = skillAttackCZ + (GetItemAddAttr(EAAjin) + GetItemAddAttr(EAAquanxiangxing))/2;
    if(type == EMTBaoBao)
        //宝宝：(等级*4+(灵性-等级)*1.2))*(法攻成长*0.13)+80+法术基础伤害*法术等级/3
        return (int)((level*4+(GetLingXing()-level)*1.2)*skillAttackCZTmp*0.13+80+base*level/3.0);
    else
        return (int)((level*4+(GetLingXing()-level)*1.2)*skillAttackCZTmp/10.0+80+base*level/3.0);
}

uint8 SPet::GetSkillNum()
{
    uint8 num = 0;
    for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
    {
        if(skill[i] != 0)
        {
            num++;
        }
    }
    return num;
}

uint8 SPet::GetSkillLevel(uint16 id)
{
    for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
    {
        if(skill[i] == id)
        {
            return skillLevel[i];
        }
    }
    return 0;
}

void SPet::DelSkill(uint16 id)
{
    for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
    {
        if(skill[i] == id)
        {
            skill[i] = 0;
            skillLevel[i] = 0;
            return;
        }
    }
}

uint8 SPet::AddSkill(uint16 id,bool fuGai,int *pFugaiId)
{
    if(fuGai)
    {
        //gailv skillId
        int skillFuGai[] = {10,160,  40,161,  80,162,  80,163,  10,164,  50,165,  70,166,  80,167,  20,168,  60,169,  60,170,  10,171,  90,172,  10,173,  40,174,  80,175,  80,176,  10,177,  50,178,  70,179,  80,180,  20,181,  60,182,  60,183,  10,184,  90,185,  60,51,  20,55,  90,59,  20,63,  20,67,  20,151,  80,152,  60,153,  20,154,  20,158,  20,101,  20,105,  50,109,  20,113,  50,117,  95,155,  95,156,  95,157,  95,3,  95,7,  95,11,  95,15,  95,19};
        int size = sizeof(skillFuGai)/sizeof(int);
        int tol = 0;
        int gailv[MAX_SKILL_NUM] = {0};
        for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
        {
            if(skill[i] != 0)
            {
                for(int j = 1; j < size; j += 2)
                {
                    if(skill[i] == skillFuGai[j])
                    {
                        tol += skillFuGai[j-1];
                        gailv[i] = tol;
                        break;
                    }
                }
            }
        }
        if(tol == 0)
        {
            if(pFugaiId != NULL)
                *pFugaiId = skill[3];
            skill[3] = id;
            skillLevel[3] = 1;
            return 1;
        }
        int r = Random(0,tol);
        for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
        {
            if((skill[i] != 0) && (r <= gailv[i]))
            {
                if(pFugaiId != NULL)
                    *pFugaiId = skill[i];
                skill[i] = id;
                skillLevel[i] = 1;
                return 1;
            }
        }
    }
    for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
    {
        if(skill[i] == id)
        {
            if(skillLevel[i] >= MAX_SKILL_LEVEL)
                return 0;
            skillLevel[i]++;
            return skillLevel[i];
        }
    }
    for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
    {
        if(skill[i] == 0)
        {
            skill[i] = id;
            skillLevel[i] = 1;
            return skillLevel[i];
        }
    }
    return 0;
}

int SPet::GetItemAddAttr(EAddAttrType type)
{
    int temp = 0;
    if((kaiJia.tmplId != 0) && (kaiJia.naijiu > 0))
    {
        for(uint8 j = 0; j < SItemInstance::MAX_KAIJIA_ATTR_NUM; j++)
        {
            if(kaiJia.addAttrType[j] == type)
            {
                temp += kaiJia.addAttrVal[j];
            }
        }
    }
    return temp;
}

uint16 SPet::GetTiZhi(bool view)
{
    if(view)
        return tizhi + GetItemAddAttr(EAAtizhi) + GetItemAddAttr(EAAquanshuxing)
            + (int)(GetSkillLevel(174)*5);
    return tizhi + GetItemAddAttr(EAAtizhi)/2 + GetItemAddAttr(EAAquanshuxing)/2
        + (int)(GetSkillLevel(174)*5);
}
    
uint16 SPet::GetLiLiang(bool view)//力量
{
    if(view)
        return liliang + GetItemAddAttr(EAAliliang) + GetItemAddAttr(EAAquanshuxing);
    return liliang + GetItemAddAttr(EAAliliang)/2 + GetItemAddAttr(EAAquanshuxing)/2;
}

uint16 SPet::GetMinJie(bool view)//敏捷
{
    if(view)
        return minjie + GetItemAddAttr(EAAminjie) + GetItemAddAttr(EAAquanshuxing)
        + (int)(GetSkillLevel(175)*5);
    return minjie + GetItemAddAttr(EAAminjie)/2 + GetItemAddAttr(EAAquanshuxing)/2
        + (int)(GetSkillLevel(175)*5);
}
    
uint16 SPet::GetLingXing(bool view)//灵性    
{
    if(view)
        return lingxing + GetItemAddAttr(EAAlingxing) + GetItemAddAttr(EAAquanshuxing);
    return lingxing + GetItemAddAttr(EAAlingxing)/2 + GetItemAddAttr(EAAquanshuxing)/2;
}

uint16 SPet::GetNaiLi(bool view)//耐力    
{
    int level = GetSkillLevel(173);
    if(view)
        return naili + GetItemAddAttr(EAAnaili) + GetItemAddAttr(EAAquanshuxing)
            + (int)(level*5);
    return naili + GetItemAddAttr(EAAnaili)/2 + GetItemAddAttr(EAAquanshuxing)/2 
        + (int)(level*5);
}
int SPet::GetSkillAttack()
{
    //(等级*4+(灵性-等级)*1.2))*(法攻成长*0.13)+90
    int skillAttackCZTmp = skillAttackCZ + (GetItemAddAttr(EAAjin) + GetItemAddAttr(EAAquanxiangxing))/2;
    return (int)((level*4+(GetLingXing()-level)*1.2)*(skillAttackCZTmp*0.13)+90)
        + GetSkillLevel(177)*50 + GetItemAddAttr(EAAshanghai);
}

int SPet::GetItemLianjiLv()
{
    return GetItemAddAttr(EAAlianjilv);
}

int SPet::GetLianJiAddShangHai()
{
    return GetItemAddAttr(EAAlianjizhuijia);//连击时追加伤害
}

int SPet::GetItemBaoJiLv()//必杀率
{
    return GetItemAddAttr(EAAbishalv);
}

int SPet::GetItemBaoJiZhuiJIa()
{
    return GetItemAddAttr(EAAbaojizhuijia);//爆击时追加伤害	
}
int SPet::GetAddBaoJiWeiLi()
{
    return GetItemAddAttr(EAAaddbaojiweili);//增加爆击威力	
}

int SPet::GetItemLianjiShu()
{
    return 2+GetItemAddAttr(EAAlianjishu);
}

int SPet::GetItemFanZhenLv()
{
    return GetItemAddAttr(EAAfanzhenlv);
}

int SPet::GetFanZhenDu()
{
    return 10+GetItemAddAttr(EAAfanzhendu);//反震度	
}

int SPet::GetRecovery()
{
    int recovery = (int)((level*5+(GetNaiLi()-level))*(6*(hpCZ+GetItemAddAttr(EAAshui)/2)/100.0) + 60)
        + GetSkillLevel(176)*30;    
    if((kaiJia.tmplId != 0) && (kaiJia.naijiu > 0))
    {
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pItem = itemMgr.GetItem(kaiJia.tmplId);
        if(pItem != NULL)
        {
            if(kaiJia.level > 0)
            {
                recovery += (int)(GetQiangHuaBeiLv(kaiJia.level-1) * pItem->fangYu);
            }
            else
            {
                recovery += pItem->fangYu;
            }
        }
    }
    recovery += GetItemAddAttr(EAAfangyu);
    return recovery;
}

void SPet::Init()//(SMonsterTmpl *pTmpl)
{
    /***************
    173,1.5,//宠物耐力强化
    174,1.5,//宠物体质强化
    175,1.5,//宠物敏捷强化
    176,9  ,//宠物提升防御
    177,15 ,//宠物提升法力
    178,15 ,//宠物提升气血
    179,1.5,//宠物提升速度
    180,9  ,//宠物金法抵挡
    181,9  ,//宠物木法抵挡
    182,9  ,//宠物水法抵挡
    183,9  ,//宠物火法抵挡
    184,9  ,//宠物土法抵挡
    185,9  ,//宠物物理抵挡
    **********************/
    
    int tizhiTmp = GetTiZhi();
    //int nailiTmp = GetNaiLi();
    int lingxingTmp = GetLingXing();
    int minjieTmp = GetMinJie();
    
    
    int hpCZTmp = hpCZ + (GetItemAddAttr(EAAshui) + GetItemAddAttr(EAAquanxiangxing))/2;
    int mpCZTmp = mpCZ + (GetItemAddAttr(EAAmu) + GetItemAddAttr(EAAquanxiangxing))/2;
    int speedCZTmp = speedCZ + (GetItemAddAttr(EAAhuo) + GetItemAddAttr(EAAquanxiangxing))/2;
    
    //int skillAttackCZTmp = skillAttackCZ + (GetItemAddAttr(EAAjin) + GetItemAddAttr(EAAquanxiangxing))/2;
    shouMingTime = GetSysTime();
    //气血：	(等级*5+(体质-等级))*(40*血成长/100)						
    maxHp = (int)((level*5+(tizhiTmp-level)) * 40*hpCZTmp/100.0) + GetSkillLevel(178)*50;
        
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    //          (等级*5+(耐力-等级))*(6*血成长/100)+60
    //recovery = (int)((level*5+(nailiTmp-level))*(6*hpCZ/100.0) + 60)
        //+ GetSkillLevel(176)*9;
    
    //法力：	(等级*5+(灵性-等级))*(15*法力成长/100)	
    maxMp = (int)((level*5+(lingxingTmp-level)) * (15*mpCZTmp/100.0))
        + GetSkillLevel(177)*50;
        
    if(hp > maxHp)
        hp = maxHp;
    if(mp > maxMp)
        mp = maxMp;
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    speed = (int)(level*(2.2+minjieTmp*0.05)+(speedCZTmp-level)*1.5+10)
        + (int)(GetSkillLevel(179)*5);
        
    
    if((kaiJia.tmplId != 0) && (kaiJia.naijiu > 0))
    {
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pItem = itemMgr.GetItem(kaiJia.tmplId);
        if(pItem != NULL)
        {
            speed += pItem->addSudu;
            speed += GetItemAddAttr(EAAsudu);
            maxMp += GetItemAddAttr(EAAfali);
            speed += GetItemAddAttr(EAAsudu);
            maxHp += GetItemAddAttr(EAAqixue);
        }
    }
    
    uint16 skillId1 = 0,skillId2 = 0,skillId3 = 0;
    switch(xiang)
    {
    case EXJinXiang:
        skillId1 = 1;
        skillId2 = 2;
        skillId3 = 4;
        break;
    case EXMuXiang:
        skillId1 = 5;
        skillId2 = 6;
        skillId3 = 8;
        break;
    case EXShuiXiang:
        skillId1 = 9;
        skillId2 = 10;
        skillId3 = 12;
        break;
    case EXHuoXiang:
        skillId1 = 13;
        skillId2 = 14;
        skillId3 = 16;
        break;
    case EXTuXiang:
        skillId1 = 17;
        skillId2 = 18;
        skillId3 = 20;
        break;
    }
        
    skill[0] = skillId1;
    skillLevel[0] = level;
    if(level >= 30)
    {
        skill[1] = skillId2;
        skillLevel[1] = level;
    }
    if(level >= 60)
    {
        skill[2] = skillId3;
        skillLevel[2] = level;
    }
    //kaiJia.tmplId = 0;
}

static uint16 GetHuoDongAddAttr(int attr)
{
    int r = Random(0,100000);
    const int SELECT_ATTR[] = 
    {
        /*
        0	,10	,40000,
        10	,20	,70000,
        20	,30	,90000,
        30	,40	,99944,
        40	,50	,99994,
        50	,60	,99999,
        60	,70	,100000
        */
        0, 10,	20000,
        10,20,	60000,
        20,30,	85000,
        30,40,	98888,
        40,50,	99988,
        50,60,	99998,
        60,70,	100000
    };
    uint8 i;
    for(i = 0; i < sizeof(SELECT_ATTR)/sizeof(int)/3; i++)
    {
        if(r <= SELECT_ATTR[3*i+2])
        {
            if(attr >= SELECT_ATTR[3*i])
            {
                int max = std::min(attr,SELECT_ATTR[3*i+1]);
                return Random(SELECT_ATTR[3*i],max);
            }
            break;
        }
    }
    
    for(i = 0; i < sizeof(SELECT_ATTR)/sizeof(int)/3; i++)
    {
        //cout<<(int)j<<endl;
        if((attr <= SELECT_ATTR[3*i+1]) && (attr >= SELECT_ATTR[3*i]))
        {
            int max = std::min(attr,SELECT_ATTR[3*i+1]);
            return Random(SELECT_ATTR[3*i],max);//SELECT_ATTR[3*i+1]);
        }
    }
    return 1;
}

static uint16 GetAddAttr(uint16 attr)
{
    int r = Random(0,10000);
    const uint16 SELECT_ATTR[] = 
    {
#ifdef QQ
        0,10,100,
        10,20,2600,
        20,30,5352,
        30,40,8352,
        40,50,9622,
        50,60,9814,
        60,70,9910,
        70,80,9958,
        80,90,9982,
        90,100,9994,
        100,110,10000
#else
        0  ,10 ,100  ,
        10 ,20 ,2600 ,
        20 ,30 ,5100 ,
        30 ,40 ,8100 ,
        40 ,50 ,9370 ,
        50 ,60 ,9690 ,
        60 ,70 ,9850 ,
        70 ,80 ,9930 ,
        80 ,90 ,9970 ,
        90 ,100, 9990,
        100, 110, 10000
#endif
    };
    uint8 i;
    for(i = 0; i < sizeof(SELECT_ATTR)/sizeof(uint16)/3; i++)
    {
        if(r <= SELECT_ATTR[3*i+2])
        {
            if(attr >= SELECT_ATTR[3*i])
            {
                int max = std::min(attr,SELECT_ATTR[3*i+1]);
                return Random(SELECT_ATTR[3*i],max);
            }
            break;
        }
    }
    
    for(i = 0; i < sizeof(SELECT_ATTR)/sizeof(uint16)/3; i++)
    {
        //cout<<(int)j<<endl;
        if((attr <= SELECT_ATTR[3*i+1]) && (attr >= SELECT_ATTR[3*i]))
        {
            int max = std::min(attr,SELECT_ATTR[3*i+1]);
            return Random(SELECT_ATTR[3*i],max);//SELECT_ATTR[3*i+1]);
        }
    }
    return 1;
}

SharePetPtr CMonsterManager::CreatePet(SMonsterInst *pMonster,bool useItem,uint8 *pType)
{
    SMonsterTmpl *pTmpl = GetTmpl(pMonster->tmplId);
    if(pTmpl == NULL)
    {
        SharePetPtr ptr;
        return ptr;
    }
    
    SPet *pPet = new SPet;
    memset(pPet,0,sizeof(SPet));
    
    strncpy(pPet->name,pMonster->name.c_str(),SPet::MAX_NAME_LEN);
    pPet->level = pMonster->level;
    pPet->xiang = pMonster->xiang;
    pPet->type = pMonster->type;
    
    
    /*pPet->skillAttack1 = pMonster->skillAttack1;//技能攻击1
    pPet->skillAttack2 = pMonster->skillAttack2;//技能攻击2
    pPet->skillAttack4 = pMonster->skillAttack4;//技能攻击4*/
    pPet->tmplId = pMonster->tmplId;
    
    pPet->qinmi = 0;
        
    pPet->exp = 0;
    pPet->shouming = Random(9000,11000);
   
    if(pPet->type == EMTBaoBao)
    {
        pPet->shuxingdian = 5;
        pPet->zhongcheng = 100;
        if(useItem)
        {
            int add = pTmpl->maxHp-pTmpl->minHp;
            pPet->hpCZ = pTmpl->minHp+GetAddAttr(add);
            add = pTmpl->maxMp-pTmpl->minMp;
            pPet->mpCZ = pTmpl->minMp+GetAddAttr(add);
            add = pTmpl->maxSpeed-pTmpl->minSpeed;
            pPet->speedCZ = pTmpl->minSpeed+GetAddAttr(add);
            add = pTmpl->maxAttack-pTmpl->minAttack;
            pPet->attackCZ = pTmpl->minAttack+GetAddAttr(add);
            add = pTmpl->maxSkillAttack-pTmpl->minSkillAttack;
            pPet->skillAttackCZ = pTmpl->minSkillAttack+GetAddAttr(add);
            
            pPet->ClearSkill();
            uint16 r = Random(0,100);
            if(r < 3)
            {//出现一个技能
                pPet->AddSkill(pTmpl->petSkillId[0]);
            }
            else if(r < 6)
            {//两个技能
                pPet->AddSkill(pTmpl->petSkillId[0]);
                if(pTmpl->petSkillId[1] != 0)
                    pPet->AddSkill(pTmpl->petSkillId[1]);
            }
        }
        else
        {
            time_t t = GetSysTime();
            struct tm *pTm = localtime(&t);
            if((pTm != NULL) && (pTm->tm_hour == CHATCH_PET_HUO_DONG))
            {
                int add = pTmpl->maxHp-pTmpl->minHp;
                pPet->hpCZ = pTmpl->minHp+GetHuoDongAddAttr(add);
                add = pTmpl->maxMp-pTmpl->minMp;
                pPet->mpCZ = pTmpl->minMp+GetHuoDongAddAttr(add);
                add = pTmpl->maxSpeed-pTmpl->minSpeed;
                pPet->speedCZ = pTmpl->minSpeed+GetHuoDongAddAttr(add);
                add = pTmpl->maxAttack-pTmpl->minAttack;
                pPet->attackCZ = pTmpl->minAttack+GetHuoDongAddAttr(add);
                add = pTmpl->maxSkillAttack-pTmpl->minSkillAttack;
                pPet->skillAttackCZ = pTmpl->minSkillAttack+GetHuoDongAddAttr(add);
                
                pPet->ClearSkill();
                uint16 r = Random(0,100);
                if(r < 1)
                {//出现一个技能
                    pPet->AddSkill(pTmpl->petSkillId[0]);
                }
            }
            else
            {
                int add = (pTmpl->maxHp-pTmpl->minHp)/10;
                pPet->hpCZ = Random(pTmpl->minHp+add,pTmpl->minHp+3*add);
                add = (pTmpl->maxHp-pTmpl->minMp)/10;
                pPet->mpCZ = Random(pTmpl->minMp+add,pTmpl->minMp+3*add);
                add = (pTmpl->maxSpeed-pTmpl->minSpeed)/10;
                pPet->speedCZ = Random(pTmpl->minSpeed+add,pTmpl->minSpeed+3*add);
                add = (pTmpl->maxAttack-pTmpl->minAttack)/10;
                pPet->attackCZ = Random(pTmpl->minAttack+add,pTmpl->minAttack+3*add);
                add = (pTmpl->maxSkillAttack-pTmpl->minSkillAttack)/10;
                pPet->skillAttackCZ = Random(pTmpl->minSkillAttack+add,pTmpl->minSkillAttack+3*add);
            }
        }
        pPet->liliang = 1; 
        pPet->lingxing = 1;
        pPet->tizhi = 1;
        pPet->naili = 1;
        pPet->minjie = 1;
    }
    else
    {
        pPet->zhongcheng = 50;
        pPet->shuxingdian = 0;
        pPet->hpCZ = Random(pTmpl->minHp,pTmpl->minHp+(pTmpl->maxHp-pTmpl->minHp)/10);
        pPet->mpCZ = Random(pTmpl->minMp,pTmpl->minMp+(pTmpl->maxHp-pTmpl->minMp)/10);
        pPet->speedCZ = Random(pTmpl->minSpeed,pTmpl->minSpeed+(pTmpl->maxSpeed-pTmpl->minSpeed)/10);
        pPet->attackCZ = Random(pTmpl->minAttack,pTmpl->minAttack+(pTmpl->maxAttack-pTmpl->minAttack)/10);
        pPet->skillAttackCZ = Random(pTmpl->minSkillAttack,
                            pTmpl->minSkillAttack+(pTmpl->maxSkillAttack-pTmpl->minSkillAttack)/10);
        pPet->liliang = pPet->level;
        pPet->lingxing = pPet->level;
        pPet->tizhi = pPet->level;
        pPet->naili = pPet->level;
        pPet->minjie = pPet->level;
        
        int tol = 4*pPet->level;
        int ave = tol/5;
        int dian[4] = {0};
        int add = CalculateRate(ave,10,100);
        if(add <= 0)
            add = 1;
        dian[0] = ave - add;
        dian[1] = ave - 2*add;
        dian[2] = ave;
        dian[3] = ave + add;
        for(uint8 i = 0; i < 4; i++)
        {
            if(dian[i] < 0)
                dian[i] = 0;
        }
        pPet->liliang += RandSelect(dian,4);
        pPet->lingxing += RandSelect(dian,4);
        pPet->tizhi += RandSelect(dian,4);
        pPet->naili += RandSelect(dian,4);
        pPet->minjie += tol - dian[0] - dian[1] - dian[2] - dian[3];
    }
    if(pPet->tmplId < 100)
    {
        if(pPet->xiang > 0)
        {//法宠
            pPet->attackCZ = 10;
        }
        else
        {//物攻宠
            pPet->skillAttackCZ = 10;
        }
    }
    pPet->Init();//(pTmpl);
    pPet->hp = pPet->maxHp;
    pPet->mp = pPet->maxMp;
    
    //(等级*4+(灵性-等级)*1.2))*(法攻成长/10)+80+法术基础伤害*法术等级/3
    //pPet->skillAttack = (int)((pPet->level*4+(pPet->lingxing-pPet->level)*1.2)*pPet->skillAttackCZ/10.0+80+40*pPet->level/3.0);
    
    double rate; 
    if(pPet->xiang > 0)
    {//法宠
        rate = pPet->hpCZ-pTmpl->minHp;//pTmpl->minHp,pTmpl->maxHp
        rate += pPet->mpCZ-pTmpl->minMp;// = Random(pTmpl->minMp,pTmpl->minMp+()/10);
        rate += pPet->speedCZ-pTmpl->minSpeed;// = Random(pTmpl->minSpeed,pTmpl->minSpeed+(pTmpl->maxSpeed-pTmpl->minSpeed)/10);
        //rate += pPet->attackCZ;// = Random(pTmpl->minAttack,pTmpl->minAttack+(pTmpl->maxAttack-pTmpl->minAttack)/10);
        rate += pPet->skillAttackCZ-pTmpl->minSkillAttack;
        int maxCZ = pTmpl->maxHp - pTmpl->minHp
                    + pTmpl->maxHp-pTmpl->minMp
                    + pTmpl->maxSpeed-pTmpl->minSpeed
                    + pTmpl->maxSkillAttack-pTmpl->minSkillAttack;
        if(maxCZ <= 0)
            maxCZ = 1;
        rate = rate/maxCZ;
    }
    else
    {//物攻宠
        rate = pPet->hpCZ-pTmpl->minHp;//pTmpl->minHp,pTmpl->maxHp
        rate += pPet->mpCZ-pTmpl->minMp;// = Random(pTmpl->minMp,pTmpl->minMp+()/10);
        rate += pPet->speedCZ-pTmpl->minSpeed;// = Random(pTmpl->minSpeed,pTmpl->minSpeed+(pTmpl->maxSpeed-pTmpl->minSpeed)/10);
        rate += pPet->attackCZ-pTmpl->minAttack;// = Random(pTmpl->minAttack,pTmpl->minAttack+(pTmpl->maxAttack-pTmpl->minAttack)/10);
        //rate += pPet->skillAttackCZ;
        int maxCZ = pTmpl->maxHp - pTmpl->minHp
                    + pTmpl->maxMp-pTmpl->minMp
                    + pTmpl->maxSpeed-pTmpl->minSpeed
                    + pTmpl->maxAttack-pTmpl->minAttack;
        if(maxCZ <= 0)
            maxCZ = 1;
        rate = rate/maxCZ;
    }
    if(rate >= 0.8)
        pPet->quality = EPQdingji;
    else if(rate >= 0.6)
        pPet->quality = EPQxiyou;
    else if(rate >= 0.4)
        pPet->quality = EPQyouxiu;
    else if(rate >= 0.2)
        pPet->quality= EPQputong;
    else
        pPet->quality = EPQpingyong;
    if(pType != NULL)
    {
        *pType = pPet->quality;
    }
    pPet->kaiJia.tmplId = 0;
    SharePetPtr ptr(pPet);
    return ptr;
}

int SPet::GetDamage()
{
    int attack = 0;
    int attackCZTmp = attackCZ + (GetItemAddAttr(EAAtu) + GetItemAddAttr(EAAquanxiangxing))/2;
    int liliangTmp = GetLiLiang();
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    if(type == EMTBaoBao)
        //宝宝：(等级*5+(力量-等级))*(15*物攻成长/100)+80
        attack = (int)((level*5+(liliangTmp-level))*(15*(float)attackCZTmp/100.0)+80);
    else
        attack = (int)((level*5+(liliangTmp-level))*(12*(float)attackCZTmp/100.0)+80);
    attack += GetItemAddAttr(EAAshanghai);
    return attack;
}

void InitHuanYingMonster(SMonsterInst *pInst,uint8 monsterLevel,bool flag)
{
    pInst->type = EMTNormal;
    pInst->tmplId = 45;
    pInst->name = "试炼天兵";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = 0xffffff;//pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CE_HUAN_YING);
    pInst->AddSkill(56,30);
    pInst->xiang = 0;
    /*if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
        pInst->AddSkill(11,60);
        pInst->AddSkill(12,60);
    }
    else
    {
        pInst->xiang = 4;
        pInst->AddSkill(15,60);
        pInst->AddSkill(16,60);
    }*/
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    /*if(flag)
        pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    else
        pInst->maxHp = MAX_INT;*/
    pInst->maxHp = 10;
        
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    if(flag)
        pInst->recovery = MAX_INT-1;//(int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    else
        pInst->recovery = MAX_INT;
        
    pInst->hp = pInst->maxHp;
    
    //pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = 2000;//(int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    //pInst->maxHp *= 10;
    
    pInst->hp = pInst->maxHp;
}
