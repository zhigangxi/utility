#ifndef _MONSTER_H_
#define _MONSTER_H_
#include "hash_table.h"
#include "self_typedef.h"
#include "item.h"
#include <string>
#include <boost/shared_ptr.hpp>
using namespace std;

struct SDropItem
{
    uint16 itemId;
    uint16 begin;
    uint16 end;
};

/*
品种系数=当前增幅总值/最大增幅值
当品种系数>=0.8时，为 顶级
当品种系数>=0.6且<0.8时，为 稀有
当品种系数>=0.4且<0.6时，为 优秀
当品种系数>=0.2且<0.4时，为 普通
当品种系数<0.2时，为 平庸
*/
enum EPetQuality
{
    EPQpingyong = 1,
    EPQputong,
    EPQyouxiu,
    EPQxiyou,
    EPQdingji
};

//技能：乾坤罩90级，10%概率对自己施放。
//属性：反击100，连击100，全法术抵抗+300
const uint16 CE_LUE_BA_XIA = 1;

//技能：相应向性3阶、4阶法术，法术等级60级，分神术60级，10%概率对攻击目标施放
//水攻击技能： 11，12
//火攻击技能：15，16
//分神术：53
//属性：法术反射100，法反增幅100
const uint16 CE_LUE_CHAO_FENG = 2;

//技能
//15	90级				
//53，57	90级	20%概率施放其一			
//112	1000级	50%血以下和30%血时各对自己施放一次。（共2次）			
//属性：全法术抵抗500，法术反射100，法反增幅100					
const uint16 CE_LUE_QI_LING = 3;

//技能：幻影术。（进入战斗只有一个怪，从第二回合开始，会释放5只幻影，幻影血无限，攻击低，
//被攻击之掉1点血，每回合6只怪均变换位置。释放技能和变换位置前，使用台词“看我移形换位。”）
const uint16 CE_HUAN_YING = 4;

const uint16 CE_FANZHENG    = 5;

//逃跑，有怪物逃跑成功战斗失败
const uint16 CL_TAO_PAO     = 6;

//自爆，目标单位失去40%血。但目标单位处于防御姿态时，只掉1点血。
const uint16 CL_ZI_BAO      = 7;

//全法术抵挡+2000,法术爆击+300,法爆强化+1000
const uint16 CL_LAN_RUO_BOS = 8;

//全法术抵挡+2000,法术闪避+150
const uint16 CL_LAN_RUO_BOS_FU = 9;

//全法术抵挡+3000,法术反射+500,反伤+500
const uint16 CL_LAN_RUO_LAOLAO  = 10;

//只用下毒
const uint16 CE_MEI_YING = 11;

//先攻击葫芦,葫芦死亡战斗结束
const uint16 FIRST_HULUO = 12;

//葫芦死亡战斗结束
const uint16 CL_HL_DIE_END = 13;

//山桃宝宝战斗可以捕捉，捕捉成功，任务数目加1，
//任务17 状态为6|num 触发战斗 捕捉成功num+1 直到num=30
//出现时第一回合说话"我是山桃宝宝"，逃跑术，第一回合就施放，100%逃跑成功。
const uint16 CL_SHAN_TAN = 14;

//易筋锻骨（112），LV3000 （在HP小于20%时连续对自己使用）
//召唤术（202），当树妖王的HP降至30%时，一次性召唤仙葫芦宝宝1只。
//葫芦死亡战斗失败
const uint16 CL_SHUYAO = 15;

//从第2回合开始，每2回合增援亲卫雄狼1只。直到增援满5只。狼王被打死后不再增援。
const uint16 CL_FENG_YA_LANG = 16;

//寒梅先集中攻击狼王，主角要保护狼王并消灭寒梅。狼王死亡则战斗强制结束并提示战斗失败。
const uint16 CL_MEI_HAN = 17;

//攻击防御最低的
const uint16 CL_FIRST_MIN_FANG = 18;

//只有狼攻击才有效果,100%逃跑。
const uint16 CL_ONLY_LANG = 19;

//在HP小于10%时连续对自己使用
const uint16 CL_XIAN_HU_LU = 20;

//舍命一击（153），LV100（只对人使用）
//乾坤罩（155），LV100，增援后第一回合对狼王使用。
const uint16 CL_FY_XIONG_LANG = 21;

//30%概率召唤石猴，当HP底于20%时连续使用妙手回春，当HP超过20%时依然30%概率召唤石猴。
const uint16 CL_LIU_ER_MI_HOUH = 22;

//可捕捉，捕捉后任务计数加一
const uint16 CL_YOU_LANG = 23;

//任务计数
const uint16 CL_DU_E_DA_XIAN = 24;

//死亡战斗结束
const uint16 CL_MONSTER_DIE_END = 25;

//可捕捉捕捉后任务计数增加
const uint16 CL_DU_MU = 26;

//用狼攻击，人攻击时候伤害/5
const uint16 CL_GU_MU_TIAN_LU = 27;

//召唤
const uint16 CL_GU_MU = 28;

//只有宠物才可对它有效的伤血。人体对其造成的伤害为真实伤害的1/10
const uint16 CL_ONLY_PET = 29;

//每2回合增援 蟹将王/牛头妖王/骷髅王 之一个。
const uint16 CL_ZHAO_HUA_NIU = 30;

/********************
BOSS出现时战斗中对话：[打倒我能得到更多奖励]
[BOSS战胜利场合]队长得到5种内丹里随机一个。
[BOSS战失败或逃跑]队长得到4种残骸里随机一个。
2038,10	//九尾狐妖残骸    
2039,15 	//九尾狐妖精魄    
2040,18 	//九尾狐妖内丹    
2041,28	//凤凰残骸	    
2042,33 	//凤凰精魄	    
2043,36 	//凤凰内丹	    
2044,56	//玄武残骸	    
2045,66	//玄武精魄	    
2046,71 	//玄武内丹	    
2047,86	//青龙残骸	    
2048,96	//青龙精魄	    
2049,99 	//青龙内丹	    
2050,100 	//年兽内丹
**************************/
const uint16 CL_MISSION21_BOSS = 32;

//捕捉到即可按以上概率得到任务品。如果杀死，则获得相应的残骸。
const uint16 CL_MISSION21_BAOBAO = 33;

//只使用混乱术60
const uint16 ONLY_HUN_LUAN = 34;

//只有宠物才可对它有效的伤血。人体对其造成的伤害为真实伤害的1
const uint16 ONLY_PET = 35;

//攻击随机消耗一件装备50点耐久
const uint16 SUN_SHI_NAIJIU = 36;

//只使用下毒57
const uint16 ONLY_DU_57 = 37;

const uint16 ONLY_SKILL_22 = 38;

//被攻击时命中率低
const uint16 DI_MING_ZHONG = 39;

//乾坤神龙轮换使用
const uint16 ONLY_QIANKUN_SHENLONG = 40;

const uint16 ONLY_QIANKUN_SHENLONG_YX = 41;

const uint16 USE_SKILL_111 = 42;

//每五回合使用53或者61
const uint16 SHI_YONG_53_61 = 43;

//优先攻击麒麟
const uint16 FIRST_QI_LING = 44;

//麒麟对冥妖王物理伤害加倍，且必定命中。
const uint16 DI_MING_ZHONG_NOT_QILING = 45;

//十回合后战斗结束(自身死亡)
const uint16 FIRST_QI_LING_MONSTER = 46;

//十回合后战斗结束(自身死亡)
const uint16 FIRST_XIAN_REN_MONSTER = 47;

struct SMonsterTmpl
{
    uint32 id;
    string name;
    uint8 minLevel;
    uint8 maxLevel;
    //一下均是最大最小成长值
    int minHp;
    int maxHp;
    int minMp;//法力
    int maxMp;
    int minSpeed;
    int maxSpeed;
    int minAttack;//物攻成长min 
    int maxAttack;//物攻成长max	
    int minSkillAttack;//技能成长 min
    int maxSkillAttack;//技能成长 max	
    uint8 xiang;
    int exp;
    static const int MAX_SKILL_NUM = 2;
    
    uint16 petSkillId[MAX_SKILL_NUM];
    
    SDropItem *pDropItem;
    uint16 dropNum;
    
    SDropItem *pHeadDropItem;//头领掉落
    uint16 headDropNum;
    
    SDropItem *pHuoDongDrop;
    uint16 huoDongDropNum;
    
    SMonsterTmpl()
    {
        pDropItem = NULL;
        pHeadDropItem = NULL;
        pHuoDongDrop = NULL;
        dropNum = 0;
        headDropNum = 0;
        huoDongDropNum = 0;
    }
    ~SMonsterTmpl()
    {
        delete pDropItem;
        delete pHeadDropItem;
        delete pHuoDongDrop;
    }
};

enum EMonsterType
{
    EMTNormal,//普通怪
    EMTBaoBao,//宝宝
    EMTTongLing,//统领
    //EMTQiPet,//可骑宠物
};

struct SMonsterInst
{
    SMonsterInst()
    {
        memset(skillIds,0,sizeof(skillIds));
        memset(skillLevels,0,sizeof(skillLevels));
        memset(skillAttackNum,0,sizeof(skillAttackNum));
        celue = 0;
        lianji = 0;//连击概率
        lianjiAdd = 0;//连击追加
        lianjiShu = 0;//连击数
        mingzhong = 0;
        
        fashubaojilv = 0;//法术爆击率
        fashubaojiadd = 0;//法术爆击追加
        
        fanshang = 0;//
        fanshangadd = 0;//
        
        baojilv = 0;
        baojiadd = 0;
        exp = 0;
        addData = 0;
        quanKang = 0;
        faShuFanTanLv = 0;
        faShuFanTan = 0;
        onlySkill = false;
        noAdd = false;
        fanJiLv = 0;
    }
    string name;
    string chatMsg;
    uint8 level;
    uint8 xiang;
    uint8 type;
    
    int maxHp;
    int maxMp;
    int hp;
    int mp;
    int speed;
    int attack;//物理攻击
    int recovery;
    
    int tmplId;
    int exp;//打怪掉落经验
    int addSkillAttack;
    int daohang;
    
    uint16 liliang;//力量
    uint16 lingxing;//灵性
    uint16 tizhi;//体质
    uint16 naili;//耐力
    uint16 minjie;//敏捷
    
    uint16 celue;//战斗策略
    
    //怪身上的绿属性
    uint8 lianji;//连击概率
    uint16 lianjiAdd;//连击追加
    uint8 lianjiShu;//连击数
    uint8 fanJiLv;//反击率
    uint16 mingzhong;
    
    uint16 fashubaojilv;//法术爆击率
    uint16 fashubaojiadd;//法术爆击追加
    
    uint16 faShuFanTanLv;
    uint16 faShuFanTan;
    
    int fanshang;//
    int fanshangadd;//
    
    int baojilv;
    int baojiadd;
    int addData;
    int quanKang;//全抗性
    
    bool onlySkill;
    bool noAdd;
    
    void SetCeLue(uint16 cl);
    uint16 GetCeLue();
    
    void Init(SMonsterTmpl*,bool initXiang=true);
    SMonsterTmpl *pMonster;
    int GetSkillDamage(int skillId);
    const static int MAX_SKILL_NUM = 5;
    uint16 skillIds[MAX_SKILL_NUM];
    uint16 skillLevels[MAX_SKILL_NUM];
    uint8 skillAttackNum[MAX_SKILL_NUM];
    uint8 skillUseTimes[MAX_SKILL_NUM];
    
    void AddSkill(uint16 skillId,uint16 level,uint8 useTimes = 255);
    uint16 GetSkillLevel(uint16 skillId);
    uint8 GetSkillNum();
    uint16 RandSelSkill();
    uint8 UseSkill(uint16 skillId);
    uint8 GetSkillAttackNum(uint16 skillId);
};

struct SPet
{
    const static int MAX_NAME_LEN = 16;
    uint8 level;
    uint8 type:4;
    uint8 bangDing:4;
    uint8 xiang:4;
    uint8 qiCheng:4;
    uint8 zhongcheng;//忠诚度最高100
    uint8 quality;
    char name[MAX_NAME_LEN];
    
    uint16 tizhi;//体质
    uint16 liliang;//力量
    uint16 minjie;//敏捷
    uint16 lingxing;//灵性    
    uint16 naili;//耐力    
    
    uint16 shuxingdian;//可分配属性点
    uint16 shouming;//
    
    uint16 hpCZ;//气血成长
    uint16 mpCZ;//法术成长
    uint16 speedCZ;//速度成长
    uint16 attackCZ;//物攻成长
    uint16 skillAttackCZ;//技能功能成长
    
    int qinmi;//亲密度
    int maxHp;
    int maxMp;
    int hp;//
    int mp;//法力
    int speed;//速度
    //int attack;//物理伤害
    //解绑时间
    time_t jbTime;
    //int recovery;//防御
    int xiuWei;//修为
    int wuxue;//同人物道行
    
    time_t shouMingTime;//上次扣寿命的时间
    const static int MAX_SKILL_NUM = 11;
    const static int MAX_SKILL_LEVEL = 120;
    uint16 skill[MAX_SKILL_NUM];
    uint8 skillLevel[MAX_SKILL_NUM];
    int exp;
    uint32 tmplId;
    
    SItemInstance kaiJia;
    
    int GetSkillDamage(int jie);
    int GetDamage();
    
    uint8 AddSkill(uint16 id,bool fuGai=false,int *pFugaiId=NULL);
    uint8 GetSkillNum();
    uint8 GetSkillLevel(uint16 id);
    void DelSkill(uint16 id);
    
    void ClearSkill()
    {
        for(uint8 i = 3; i < MAX_SKILL_NUM; i++)
        {
            skill[i] = 0;
            skillLevel[i] = 0;
        }
    }
    
    void AddLevel();
    void Init();//(SMonsterTmpl*);
    int GetItemAddAttr(EAddAttrType type);
    uint16 GetTiZhi(bool view = false);
    uint16 GetLiLiang(bool view = false);//力量
    uint16 GetMinJie(bool view = false);//敏捷
    uint16 GetLingXing(bool view = false);//灵性    
    uint16 GetNaiLi(bool view = false);//耐力    
    int GetRecovery();
    int GetItemLianjiLv();
    int GetLianJiAddShangHai();
    int GetItemLianjiShu();
    int GetItemBaoJiLv();
    int GetSkillAttack();
    int GetItemBaoJiZhuiJIa();
    int GetAddBaoJiWeiLi();
    int GetItemFanZhenLv();
    int GetFanZhenDu();
    /*void Assign(SPet &p)
    {
        level                  = p.level;
        xiang                  = p.xiang;
        zhongcheng             = p.zhongcheng;
        
        strcpy(name,p.name);
                               
        qinmi                  = p.qinmi;
        liliang                = p.liliang;
        lingxing               = p.lingxing;
        tizhi                  = p.tizhi;
        naili                  = p.naili;
        minjie                 = p.minjie;
        
        shuxingdian            = p.shuxingdian;
        
        wuxue                  = p.wuxue;
        shouming               = p.shouming;
        
        maxHp                  = p.maxHp;
        maxMp                  = p.maxMp;
        hp                     = p.hp;
        mp                     = p.mp;
        speed                  = p.speed;
        attack                 = p.attack;
        recovery               = p.recovery;
        tmplId                 = p.tmplId;
        shouMingTime           = p.shouMingTime;
        
        memcpy(skill,p.skill,sizeof(skill));
        memcpy(skillLevel,p.skillLevel,sizeof(skillLevel));
    }*/
};

typedef boost::shared_ptr<SMonsterInst> ShareMonsterPtr;
typedef boost::shared_ptr<SPet> SharePetPtr;

class CMonsterManager
{
public:
    //pTmpl必须是使用new分配出来的
    void AddMonster(SMonsterTmpl *pTmpl)
    {
        m_monsterTmpl.Insert(pTmpl->id,pTmpl);
    }
    SMonsterTmpl *GetTmpl(uint32 id)
    {
        SMonsterTmpl *pTmpl = NULL;
        m_monsterTmpl.Find(id,pTmpl);
        return pTmpl;
    }
    ShareMonsterPtr CreateMonster(uint32 tmplId,EMonsterType type,bool useMaxLevel = false,bool useMinLevel = false);
    ShareMonsterPtr CreateMonster(uint32 tmplId,uint8 level,EMonsterType type,const char *name = NULL);
                                                                        //EPetQuality
    SharePetPtr CreatePet(SMonsterInst *pMonster,bool useItem = false,uint8 *pType = NULL);
private:
    CHashTable<uint32,SMonsterTmpl*> m_monsterTmpl;
};

void InitHuanYingMonster(SMonsterInst *pInst,uint8 monsterLevel,bool flag);

#endif
