struct SItemTemplate
{
    SItemTemplate():
        type(0),//����
        level(0),//����ȼ�	
        mod(0),//ģʽ
        sex(0),//�Ա�
        fangYu(0),//����
        id(0),
        gongji(0),//������
        naijiu(0),//�;�
        addFaLi(0),//���ӷ���
        addQiXue(0),//������Ѫ
        addSudu(0),//�����ٶ�
        pic(0),//ͼƬ
        addXue(0),//����Ѫ��
        jiage(0),//�۸�
        pScript(NULL)
    {
    }
    
    uint8 type;//����
    uint8 level;//����ȼ�	
    uint8 mod;//ģʽ
    uint8 sex;//�Ա�
    uint16 fangYu;//����
    uint16 id;
    uint16 gongji;//������
    uint16 naijiu;//�;�
    uint16 addFaLi;//���ӷ���
    uint16 addQiXue;//������Ѫ
    uint16 addSudu;//�����ٶ�
    uint16 pic;//ͼƬ
    int addXue;//����Ѫ��
    int jiage;//�۸�    
    string name;//����
    string describe;//˵��
};

#define MAX_ADD_ATTR_NUM  9

struct SItemInstance
{
    uint8 level;
    uint8   strengthenLevel;//ǿ������
    uint8   quality;//Ʒ��
    uint8   addAttrNum;//������������
    uint8   num;
    uint8   addAttrType[MAX_ADD_ATTR_NUM];//������������
    uint8   reqAttrType;//������������
    uint16  reqAttrVal;//��������ֵ
    uint16  addAttrVal[MAX_ADD_ATTR_NUM];//��������ֵ
    uint16  naijiu;//�;�
    uint16 tmplId;
    int GetItemValue();
    int GetAddAttrType(uint8 pos);
    int GetAddAttrVal(uint8 pos);
};

