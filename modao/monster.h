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
Ʒ��ϵ��=��ǰ������ֵ/�������ֵ
��Ʒ��ϵ��>=0.8ʱ��Ϊ ����
��Ʒ��ϵ��>=0.6��<0.8ʱ��Ϊ ϡ��
��Ʒ��ϵ��>=0.4��<0.6ʱ��Ϊ ����
��Ʒ��ϵ��>=0.2��<0.4ʱ��Ϊ ��ͨ
��Ʒ��ϵ��<0.2ʱ��Ϊ ƽӹ
*/
enum EPetQuality
{
    EPQpingyong = 1,
    EPQputong,
    EPQyouxiu,
    EPQxiyou,
    EPQdingji
};

//���ܣ�Ǭ����90����10%���ʶ��Լ�ʩ�š�
//���ԣ�����100������100��ȫ�����ֿ�+300
const uint16 CE_LUE_BA_XIA = 1;

//���ܣ���Ӧ����3�ס�4�׷����������ȼ�60����������60����10%���ʶԹ���Ŀ��ʩ��
//ˮ�������ܣ� 11��12
//�𹥻����ܣ�15��16
//��������53
//���ԣ���������100����������100
const uint16 CE_LUE_CHAO_FENG = 2;

//����
//15	90��				
//53��57	90��	20%����ʩ����һ			
//112	1000��	50%Ѫ���º�30%Ѫʱ�����Լ�ʩ��һ�Ρ�����2�Σ�			
//���ԣ�ȫ�����ֿ�500����������100����������100					
const uint16 CE_LUE_QI_LING = 3;

//���ܣ���Ӱ����������ս��ֻ��һ���֣��ӵڶ��غϿ�ʼ�����ͷ�5ֻ��Ӱ����ӰѪ���ޣ������ͣ�
//������֮��1��Ѫ��ÿ�غ�6ֻ�־��任λ�á��ͷż��ܺͱ任λ��ǰ��ʹ��̨�ʡ��������λ�λ������
const uint16 CE_HUAN_YING = 4;

const uint16 CE_FANZHENG    = 5;

//���ܣ��й������ܳɹ�ս��ʧ��
const uint16 CL_TAO_PAO     = 6;

//�Ա���Ŀ�굥λʧȥ40%Ѫ����Ŀ�굥λ���ڷ�����̬ʱ��ֻ��1��Ѫ��
const uint16 CL_ZI_BAO      = 7;

//ȫ�����ֵ�+2000,��������+300,����ǿ��+1000
const uint16 CL_LAN_RUO_BOS = 8;

//ȫ�����ֵ�+2000,��������+150
const uint16 CL_LAN_RUO_BOS_FU = 9;

//ȫ�����ֵ�+3000,��������+500,����+500
const uint16 CL_LAN_RUO_LAOLAO  = 10;

//ֻ���¶�
const uint16 CE_MEI_YING = 11;

//�ȹ�����«,��«����ս������
const uint16 FIRST_HULUO = 12;

//��«����ս������
const uint16 CL_HL_DIE_END = 13;

//ɽ�ұ���ս�����Բ�׽����׽�ɹ���������Ŀ��1��
//����17 ״̬Ϊ6|num ����ս�� ��׽�ɹ�num+1 ֱ��num=30
//����ʱ��һ�غ�˵��"����ɽ�ұ���"������������һ�غϾ�ʩ�ţ�100%���ܳɹ���
const uint16 CL_SHAN_TAN = 14;

//�׽�͹ǣ�112����LV3000 ����HPС��20%ʱ�������Լ�ʹ�ã�
//�ٻ�����202��������������HP����30%ʱ��һ�����ٻ��ɺ�«����1ֻ��
//��«����ս��ʧ��
const uint16 CL_SHUYAO = 15;

//�ӵ�2�غϿ�ʼ��ÿ2�غ���Ԯ��������1ֻ��ֱ����Ԯ��5ֻ������������������Ԯ��
const uint16 CL_FENG_YA_LANG = 16;

//��÷�ȼ��й�������������Ҫ��������������÷������������ս��ǿ�ƽ�������ʾս��ʧ�ܡ�
const uint16 CL_MEI_HAN = 17;

//����������͵�
const uint16 CL_FIRST_MIN_FANG = 18;

//ֻ���ǹ�������Ч��,100%���ܡ�
const uint16 CL_ONLY_LANG = 19;

//��HPС��10%ʱ�������Լ�ʹ��
const uint16 CL_XIAN_HU_LU = 20;

//����һ����153����LV100��ֻ����ʹ�ã�
//Ǭ���֣�155����LV100����Ԯ���һ�غ϶�����ʹ�á�
const uint16 CL_FY_XIONG_LANG = 21;

//30%�����ٻ�ʯ���HP����20%ʱ����ʹ�����ֻش�����HP����20%ʱ��Ȼ30%�����ٻ�ʯ�
const uint16 CL_LIU_ER_MI_HOUH = 22;

//�ɲ�׽����׽�����������һ
const uint16 CL_YOU_LANG = 23;

//�������
const uint16 CL_DU_E_DA_XIAN = 24;

//����ս������
const uint16 CL_MONSTER_DIE_END = 25;

//�ɲ�׽��׽�������������
const uint16 CL_DU_MU = 26;

//���ǹ������˹���ʱ���˺�/5
const uint16 CL_GU_MU_TIAN_LU = 27;

//�ٻ�
const uint16 CL_GU_MU = 28;

//ֻ�г���ſɶ�����Ч����Ѫ�����������ɵ��˺�Ϊ��ʵ�˺���1/10
const uint16 CL_ONLY_PET = 29;

//ÿ2�غ���Ԯ з����/ţͷ����/������ ֮һ����
const uint16 CL_ZHAO_HUA_NIU = 30;

/********************
BOSS����ʱս���жԻ���[�����ܵõ����ཱ��]
[BOSSսʤ������]�ӳ��õ�5���ڵ������һ����
[BOSSսʧ�ܻ�����]�ӳ��õ�4�ֲк������һ����
2038,10	//��β�����к�    
2039,15 	//��β��������    
2040,18 	//��β�����ڵ�    
2041,28	//��˲к�	    
2042,33 	//��˾���	    
2043,36 	//����ڵ�	    
2044,56	//����к�	    
2045,66	//���侫��	    
2046,71 	//�����ڵ�	    
2047,86	//�����к�	    
2048,96	//��������	    
2049,99 	//�����ڵ�	    
2050,100 	//�����ڵ�
**************************/
const uint16 CL_MISSION21_BOSS = 32;

//��׽�����ɰ����ϸ��ʵõ�����Ʒ�����ɱ����������Ӧ�Ĳк���
const uint16 CL_MISSION21_BAOBAO = 33;

//ֻʹ�û�����60
const uint16 ONLY_HUN_LUAN = 34;

//ֻ�г���ſɶ�����Ч����Ѫ�����������ɵ��˺�Ϊ��ʵ�˺���1
const uint16 ONLY_PET = 35;

//�����������һ��װ��50���;�
const uint16 SUN_SHI_NAIJIU = 36;

//ֻʹ���¶�57
const uint16 ONLY_DU_57 = 37;

const uint16 ONLY_SKILL_22 = 38;

//������ʱ�����ʵ�
const uint16 DI_MING_ZHONG = 39;

//Ǭ�������ֻ�ʹ��
const uint16 ONLY_QIANKUN_SHENLONG = 40;

const uint16 ONLY_QIANKUN_SHENLONG_YX = 41;

const uint16 USE_SKILL_111 = 42;

//ÿ��غ�ʹ��53����61
const uint16 SHI_YONG_53_61 = 43;

//���ȹ�������
const uint16 FIRST_QI_LING = 44;

//�����ڤ���������˺��ӱ����ұض����С�
const uint16 DI_MING_ZHONG_NOT_QILING = 45;

//ʮ�غϺ�ս������(��������)
const uint16 FIRST_QI_LING_MONSTER = 46;

//ʮ�غϺ�ս������(��������)
const uint16 FIRST_XIAN_REN_MONSTER = 47;

struct SMonsterTmpl
{
    uint32 id;
    string name;
    uint8 minLevel;
    uint8 maxLevel;
    //һ�¾��������С�ɳ�ֵ
    int minHp;
    int maxHp;
    int minMp;//����
    int maxMp;
    int minSpeed;
    int maxSpeed;
    int minAttack;//�﹥�ɳ�min 
    int maxAttack;//�﹥�ɳ�max	
    int minSkillAttack;//���ܳɳ� min
    int maxSkillAttack;//���ܳɳ� max	
    uint8 xiang;
    int exp;
    static const int MAX_SKILL_NUM = 2;
    
    uint16 petSkillId[MAX_SKILL_NUM];
    
    SDropItem *pDropItem;
    uint16 dropNum;
    
    SDropItem *pHeadDropItem;//ͷ�����
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
    EMTNormal,//��ͨ��
    EMTBaoBao,//����
    EMTTongLing,//ͳ��
    //EMTQiPet,//�������
};

struct SMonsterInst
{
    SMonsterInst()
    {
        memset(skillIds,0,sizeof(skillIds));
        memset(skillLevels,0,sizeof(skillLevels));
        memset(skillAttackNum,0,sizeof(skillAttackNum));
        celue = 0;
        lianji = 0;//��������
        lianjiAdd = 0;//����׷��
        lianjiShu = 0;//������
        mingzhong = 0;
        
        fashubaojilv = 0;//����������
        fashubaojiadd = 0;//��������׷��
        
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
    int attack;//������
    int recovery;
    
    int tmplId;
    int exp;//��ֵ��侭��
    int addSkillAttack;
    int daohang;
    
    uint16 liliang;//����
    uint16 lingxing;//����
    uint16 tizhi;//����
    uint16 naili;//����
    uint16 minjie;//����
    
    uint16 celue;//ս������
    
    //�����ϵ�������
    uint8 lianji;//��������
    uint16 lianjiAdd;//����׷��
    uint8 lianjiShu;//������
    uint8 fanJiLv;//������
    uint16 mingzhong;
    
    uint16 fashubaojilv;//����������
    uint16 fashubaojiadd;//��������׷��
    
    uint16 faShuFanTanLv;
    uint16 faShuFanTan;
    
    int fanshang;//
    int fanshangadd;//
    
    int baojilv;
    int baojiadd;
    int addData;
    int quanKang;//ȫ����
    
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
    uint8 zhongcheng;//�ҳ϶����100
    uint8 quality;
    char name[MAX_NAME_LEN];
    
    uint16 tizhi;//����
    uint16 liliang;//����
    uint16 minjie;//����
    uint16 lingxing;//����    
    uint16 naili;//����    
    
    uint16 shuxingdian;//�ɷ������Ե�
    uint16 shouming;//
    
    uint16 hpCZ;//��Ѫ�ɳ�
    uint16 mpCZ;//�����ɳ�
    uint16 speedCZ;//�ٶȳɳ�
    uint16 attackCZ;//�﹥�ɳ�
    uint16 skillAttackCZ;//���ܹ��ܳɳ�
    
    int qinmi;//���ܶ�
    int maxHp;
    int maxMp;
    int hp;//
    int mp;//����
    int speed;//�ٶ�
    //int attack;//�����˺�
    //���ʱ��
    time_t jbTime;
    //int recovery;//����
    int xiuWei;//��Ϊ
    int wuxue;//ͬ�������
    
    time_t shouMingTime;//�ϴο�������ʱ��
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
    uint16 GetLiLiang(bool view = false);//����
    uint16 GetMinJie(bool view = false);//����
    uint16 GetLingXing(bool view = false);//����    
    uint16 GetNaiLi(bool view = false);//����    
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
    //pTmpl������ʹ��new���������
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
