#ifndef _SKILL_H_
#define _SKILL_H_
#include "self_typedef.h"

enum EXiang
{
    EXJinXiang = 1,//-金
    EXMuXiang,//2－木
    EXShuiXiang,//3－水
    EXHuoXiang,//4－火
    EXTuXiang//5－土
};

enum ESkillType
{
    //咒术
    ESTDamageHp = 1,//伤HP。
    
    //遗忘
    //4回合内技能菜单显灰无法选取。感受此技能期间无法再次感受相同技能，但可感受其他负面技能并被覆盖。当技能效力结束、战斗结束后技能效果消失。
    ESTForget,
    
    //下毒
    //4回合内每回合轮到用户行动时损失20% HP值，当HP损失到0时判断死亡。感受此技能期间无法再次感受相同技能，但可感受其他负面技能并被覆盖。当技能效力结束、战斗结束后技能效果消失。
    ESTEnvenom,

    //混乱
    //目标感受此技能后，4回合内菜单可选但无法实施，轮到该玩家回合时，以平砍方式随机攻击敌我任何一个战斗单位。感受此技能期间无法再次感受相同技能，但可感受其他负面技能并被覆盖。当技能效力结束、战斗结束后技能效果消失。
    ESTConfusion,
    
    //昏睡
    //4回合内无法行动，菜单可选但无法实施。感受此技能期间无法再次感受相同技能，但可感受其他负面技能并被覆盖。当受到攻击、技能效力结束、战斗结束后技能效果消失。
    ESTDizzy,
    
    //冰冻
    //4回合内无法行动，菜单可选但无法实施，感受此技能期间无敌（咒术、平砍无效）。感受此技能期间无法再次感受相同技能，但可感受其他负面技能并被覆盖。当技能效力结束、战斗结束后技能效果消失。
    ESTFrost,
    
    //法攻提升
    //4回合内伤害提高10%。当技能效力结束、战斗结束后技能效果消失。
    ESTImproveDamage,

    //回避提升
    //目标感受此技能后，4回合内回避提高10%。当技能效力结束、战斗结束后技能效果消失。
    ESTImproveDodge,
    
    //血量提升
    //4回合内血量提高30%。
    ESTImproveHp,
    
    //速度提升
    //目标感受此技能后，4回合内速度提高10%。当技能效力结束、战斗结束后技能效果消失。
    ESTImproveSpeed,
    
    //物攻提升
    //4回合内防御提高10%。
    ESTImproveRecovery,
    
    //宠物天书技能
    //151	神圣之光 攻击技能	可以清除对方身上的辅助类状态
    ESTshenshengzhiguang,
    
    //152	漫天血舞 攻击技能	被法术击中目标的气血会有一定量被施法者吸收，以补充施法者的气血							
    ESTmantianxuewu,
    
    //153	舍命一击 攻击技能	被法术命中的对象的法力值会下降，而施法者气血降至1点
    ESTshemingyiji,
    
    //154	死亡缠绵 辅助技能	对己方对象使用后，对象使用物理攻击时，将必定出现连击效果							
    ESTsiwangchanmian,
    
    //155	乾坤罩 辅助技能	对己方对象使用后，被施法者在受到物理攻击时，可免疫对方物理攻击一次，并对攻击者造成一定数值的反弹伤害							
    ESTqiankunzhao,
    
    //156	如意圈 辅助技能	使用后，被施用对象在一定回合数内抵御一定伤害的法术攻击							
    ESTruyiquan,
    
    //157	神龙罩 辅助技能	使用后，被施用对象在一定回合数内抵御一定伤害的物理攻击							
    ESTshenlongzhao,
    
    //158	翻转乾坤 辅助技能	使用成功后，可改变目标相性							
    ESTfanzhuanqiankun
};

uint8 GetSkillType(uint8 id);
uint8 GetSkillXiang(uint8 id);
uint8 GetSkillAttackNum(uint8 id,uint8 level);

uint8 GetSkillJie(uint16 id);

#endif
