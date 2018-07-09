#ifndef _ITEM_H_
#define _ITEM_H_
#include <string>
#include "self_typedef.h"
#include "hash_table.h"
using namespace std;
class CCallScript;
class CNetMessage;

enum EAddAttrType//������������
{
	EAAliliang = 1,//����(�̶�ֵ)
	EAAnaili,//����(�̶�ֵ)
	EAAtizhi,//����(�̶�ֵ)
	EAAminjie,//����(�̶�ֵ)
	EAAlingxing,//����(�̶�ֵ)
	
	EAAquanshuxing,//ȫ����(�̶�ֵ)
	
	EAAshanghai,//�˺�
	EAAfangyu,//����
	EAAfali,//����
	EAAqixue,//��Ѫ
	EAAsudu,//�ٶ�
	
	EAAjin,//����
	EAAmu,//ľ��
	EAAshui,//ˮ��
	EAAhuo,//����
	EAAtu,//����
	EAAquanxiangxing,//ȫ����
	
	EAAmingzhong,//������
	EAAhuibiwuli,//�ر�������
	EAAlianjilv,//������
	EAAlianjishu,//������
	EAAbishalv,//��ɱ��
	EAAfanjilv,//������
	EAAfanzhenlv,//������
	EAAgongjijiangdimp,//��������MP���Ľ���
	EAAfuzhujiangdimp,//��������MP���Ľ���
	EAAzhangaijiangdimp,//�ϰ�����MP���Ľ���
	EAAquanjiangdimp,//ȫ����MP���Ľ���
	EAAhuibifashugongji,//�رܷ�������
	EAAjinkang,//����
	EAAmukang,//ľ����
	EAAshuikang,//ˮ����
	EAAhuokang,//����
	EAAtukang,//������
	EAAquankang,//ȫ����
	EAAkangyiwang,//������
	EAAkangzhongdu,//���ж�
	EAAkangbingdong,//������
	EAAkanghuishui,//����˯
	EAAkanghunluan,//������
	EAAquankangyichang,//ȫ���쳣
	EAAquanjineng,//ȫ���ܣ����Ӽ��ܵȼ���
	EAAhushikangxing,//�������п���
	EAAhushikangyichang,//�������п��쳣
	
	//����������1.5
	//������������1.2
	EAAbaojizhuijia,//����ʱ׷���˺�	
	EAAaddbaojiweili,//���ӱ�������	���Ӱٷֱȣ��ڱ���
	EAAlianjizhuijia,//����ʱ׷���˺�	
	EAAfanjizhuijia,//����ʱ׷���˺�	
	EAAfanzhendu,//�����	
	EAAfashubaoji,//��������	
	EAAfashubaojizhuijia,//��������׷��	
	EAAfashufantan,//��������
    EAAFashufantanlv,//����������
    
    EAddAttrTypeEnd,
	EAddAttrTypeNum = EAddAttrTypeEnd -1
};

const uint8 BLUE_EQUIP[] = 
{
    EAAliliang,//����(�̶�ֵ)
	EAAnaili,//����(�̶�ֵ)
	EAAtizhi,//����(�̶�ֵ)
	EAAminjie,//����(�̶�ֵ)
	EAAlingxing,//����(�̶�ֵ)
	
    EAAshanghai,//�˺�
	EAAfangyu,//����
	EAAfali,//����
	EAAqixue,//��Ѫ
	EAAsudu,//�ٶ�
	
	EAAjin,//����
	EAAmu,//ľ��
	EAAshui,//ˮ��
	EAAhuo,//����
	EAAtu,//����
	
	EAAmingzhong,//������
	EAAhuibiwuli,//�ر�������
	EAAlianjilv,//������
	
    EAAbishalv,//��ɱ��
	EAAfanjilv,//������
	EAAfanzhenlv//������
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
    EAAquanshuxing,//ȫ����(�̶�ֵ)
    EAAquanxiangxing,//ȫ����
    EAAlianjishu,//������
    EAAgongjijiangdimp,//��������MP���Ľ���
	EAAfuzhujiangdimp,//��������MP���Ľ���
	EAAzhangaijiangdimp,//�ϰ�����MP���Ľ���
	EAAquanjiangdimp,//ȫ����MP���Ľ���
	//EAAhuibifashugongji,//�رܷ�������
	EAAjinkang,//����
	EAAmukang,//ľ����
	EAAshuikang,//ˮ����
	EAAhuokang,//����
	EAAtukang,//������
	EAAquankang,//ȫ����
	EAAkangyiwang,//������
	EAAkangzhongdu,//���ж�
	EAAkangbingdong,//������
	EAAkanghuishui,//����˯
	EAAkanghunluan,//������
	EAAquankangyichang,//ȫ���쳣
	//EAAquanjineng,//ȫ���ܣ����Ӽ��ܵȼ���
	EAAhushikangxing,//�������п���
	EAAhushikangyichang,//�������п��쳣
	
	EAAbaojizhuijia,//����ʱ׷���˺�	
	EAAaddbaojiweili,//���ӱ�������	
	EAAlianjizhuijia,//����ʱ׷���˺�	
	EAAfanjizhuijia,//����ʱ׷���˺�	
	EAAfanzhendu,//�����	
	EAAfashubaoji,//��������	
	EAAfashubaojizhuijia,//��������׷��	
	EAAfashufantan,//��������
    EAAFashufantanlv//����������
};

enum EItemType
{
    EITQiang = 1	,//ǹ
    EITZhua = 2	,//צ
    EITJian = 3	,//��
    EITDao = 4	,//��
    EITGun = 5	,//��
    
    EITMaoZi = 6	,//ñ��
    EITYiFu = 7	,//�·�
    EITYaoDai = 8,//����
    EITXieZi = 9,//Ь��
    
    EITXiangLian = 10,//��,���ӷ���
    EITYuPei = 11,//�壬������Ѫ
    EITShouZhuo = 12,//�������ӹ���
    
    EITAddAttr = 13,//���ȼ�ʯͷ13
    EITNormalYaoPin = 14,//��pkҩ
    EITNvWaShi = 15,//Ů�ʯ15
    EITPKYaoPin = 16,//pkҩ
    
    EITTeShu = 17,//������Ʒ17
    
    EITMission = 18,//������Ʒ
    
    EITChargeItem = 19,//�շѵ���,�ɵ���
    
    EITMissionCanSave = 20,//
    
    EITPetBook = 21,//��������
    EITPetKaiJia = 22,//��������
    
    EITCanDelMiss = 23,//�ɶ���������Ʒ
    
    EItemDieJiaNum = 20,//��Ʒ��������
    
    EBankItemDieJIaNum = 99//
};

//Ʒ������
enum EQualityType
{
    EQTWhite,//��װ
    EQTBlue,//��װ
    EQTGreen,//��װ
};

struct SItemTemplate
{
    SItemTemplate():
        type(0),//����
        level(0),//����ȼ�	
        mod(0),//ģʽ
        sex(0),//�Ա�
        //canDieJia(false),
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
    //bool canDieJia;//�Ƿ�ɵ���
    uint16 fangYu;//����
    uint16 id;
    uint16 gongji;//������
    uint16 naijiu;//�;�
    uint16 addFaLi;//���ӷ���
    uint16 addQiXue;//������Ѫ
    uint16 addSudu;//�����ٶ�
    uint16 pic;//ͼƬ
    
    //����Ʒ����ΪEITAddAttrʱ��addXue��λ��ʾ�����������
    int addXue;//����Ѫ��
    
    int jiage;//�۸�    
    string name;//����
    string describe;//˵��
    CCallScript *pScript;//������Ʒ�ű�
};

/***********
0x1 hp�ٷֱ�
0x2 hp�̶�ֵ
0x4 mp�ٷֱ�
0x8 mp�ٷֱ�
************/
enum EItemModType
{
    EIMTHpPer = 1,
    EIMTHpFix = 2,
    EIMTMpPer = 4,
    EIMTMpFix = 8,
    EIMTZhongcheng = 16,//�ָ������ҳ϶�
    EIMTShouming = 32,//�ָ���������
    EIMTJiaoYiBang = 64,//���׺��		
    EIMTName = 128,//��������Ʒ		
};

/********
1-����
2-����
3-����
4-����
5-����
*********/
enum EReqAttrType//������������
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
    const static char CAN_NOT_XIE_XIA = 'n';//����ж��װ��
    uint8   num;
    uint8   level;//ǿ���ȼ�
    uint8   strengthenLevel;//ǿ������
    uint8   quality:4;//Ʒ��
    uint8   bangDing:4;//�Ƿ�󶨣�0�����󶨣�1����
    uint8   addAttrNum;//������������
    uint8   addAttrType[MAX_KAIJIA_ATTR_NUM];//������������
    uint8   reqAttrType;//������������
    uint16  reqAttrVal;//��������ֵ
    uint16  addAttrVal[MAX_KAIJIA_ATTR_NUM];//��������ֵ
    uint16  naijiu;//�;�
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
    //pItem������ʹ��new���������
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

//�жϸ��������Ƿ���Լӵ���������Ʒ
bool CanAddAttr(uint8 attrType,uint8 itemType);

//ѡ����װ�Ƿ�ɹ�
bool SelectGreenAttr(uint8 &duan,uint8 stoneLevel,uint8 stoneNum);

//�������ʯ�ɱ�
double GetStoneChengBen(uint8 level);

//��������ɱ�
int GetLeveUpChengBen(uint8 level);

const int MAX_QIANGHUA_LEVEL = 15;

#endif
