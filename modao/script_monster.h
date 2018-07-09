
struct SMonsterTmpl
{
    uint32 id;
    string name;
    uint8 minLevel;
    uint8 maxLevel;
    //���¾��������С�ɳ�ֵ
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
    //int recovery;//����
    int xiuWei;//��Ϊ
    int wuxue;//ͬ�������
    
    time_t shouMingTime;//�ϴο�������ʱ��
    /*const static int MAX_SKILL_NUM = 11;
    const static int MAX_SKILL_LEVEL = 120;
    uint16 skill[MAX_SKILL_NUM];
    uint8 skillLevel[MAX_SKILL_NUM];*/
    int exp;
    uint32 tmplId;
    
    SItemInstance kaiJia;
};
