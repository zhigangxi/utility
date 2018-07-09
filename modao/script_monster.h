
struct SMonsterTmpl
{
    uint32 id;
    string name;
    uint8 minLevel;
    uint8 maxLevel;
    //以下均是最大最小成长值
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
};

#define MAX_NAME_LEN  16
#define MAX_KANGXING  10
#define MAX_SKILL_NUM  10
struct SPet
{
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
    //int recovery;//防御
    int xiuWei;//修为
    int wuxue;//同人物道行
    
    time_t shouMingTime;//上次扣寿命的时间
    /*const static int MAX_SKILL_NUM = 11;
    const static int MAX_SKILL_LEVEL = 120;
    uint16 skill[MAX_SKILL_NUM];
    uint8 skillLevel[MAX_SKILL_NUM];*/
    int exp;
    uint32 tmplId;
    
    SItemInstance kaiJia;
};
