#ifndef _JIANGHU_PROTOCOL_H_
#define _JIANGHU_PROTOCOL_H_
#include "self_typedef.h"
const uint8 PRO_ERROR         = 0;
const uint8 PRO_SUCCESS       = 1;

//�Ա�
const uint8 GENDER_MALE     = 0;
const uint8 GENDER_FEMALE   = 1;

//����
const uint8 XIANG_JIN       = 1;
const uint8 XIANG_MU        = 2;
const uint8 XIANG_SHUI      = 3;
const uint8 XIANG_HUO       = 4;
const uint8 XIANG_TU        = 5;

//����ɫ��
const int MAX_ROLE_NUM      = 6;

//����û���
const int MAX_CON_USER      = 4096;

//Э��������
const int PRO_USER_LOGIN    = 1;
const int PRO_REG_USER      = 2;
const int PRO_CREATE_ROLE   = 3;
const int PRO_SELECT_ROLE   = 4;
const int PRO_NPC_LIST      = 5;
const int PRO_USER_LIST     = 6;
const int PRO_ROLE_EQUIPMENT = 7;
const int PRO_ROLE_PACKAGE  = 8;
const int PRO_ROLE_MOVE     = 9;
const int PRO_JUMP_SCENE    = 10;
const int PRO_IN_OUT_SCENE  = 11;
const int PRO_OPEN_INTERACT = 12;

//30����ֿ�,������Ϣ
//31ʦͽ��
const int PRO_INTERACT      = 13;

const int PRO_CLOSE_INTERACT    = 14;

const int PRO_UPDATE_PACK   = 15;
const int PRO_GET_ITEM_INFO = 16;

const int PRO_OUTFIT        = 17;//װ����Ʒ
const int PRO_UPDATE_CHAR   = 18;

const int PRO_OTHER_ITEM_INFO = 19;//��ѯ���������Ʒ��Ϣ

const int PRO_PLAYER_DETAIL    = 20;

const int PRO_ENTER_BATTLE  = 21;//����ս��
const int PRO_BATTLE        = 22;//ս������
const int PRO_BATTLE_OVER   = 23;//ս������

const int PRO_QUERY_PET     = 24;//�����б�

const int PRO_UPDATE_PET    = 25;//�ͻ��˳���������������˸��³����б�

const int PRO_MSG_CHAT      = 26;//����

const int PRO_USER_SKILL    = 27;//

const int PRO_UPDATE_SKILL  = 28;

const int PRO_USER_TEAM     = 29;// ����

const int PRO_UPDATE_TEAM   = 30;//���¶���

const int PRO_SYS_INFO      = 31;//ϵͳ��Ϣ

const int PRO_USER_PK       = 32;//���pk

const int PRO_PLYAER_MATCH  = 33;//����д�

const int PRO_PLAYER_INFO   = 34;//��ѯ�����Ϣ

const int PRO_NEAR_PLAYER_LIST = 35;//��������б�

const int PRO_UPDATE_PLAYER = 36;//�������״̬

const int PRO_TASK_LIST     = 37;//�����б�

const int PRO_TASK_INFO     = 38;//������Ϣ

const int PRO_UPDATE_TASK   = 39;//��������

const int PRO_PET_SKILL     = 40;

const int PRO_UPDATE_PET_SKILL = 41;//���³��＼��

const int PRO_HOT_LIST      = 42;

const int PRO_ADD_HOT       = 43;

const int PRO_DEL_HOT       = 44;

const int PRO_HOT_ONLINE    = 45;//����������֪ͨ

const int PRO_ONLINE_MAIL   = 46;//

const int RPO_QUERY_MAIL    = 47;//��ѯ�ż�

const int PRO_MAIL_ACK      = 48;//�ż�

const int PRO_PSHOP_PUT     = 49;

const int PRO_PSHOP_LIST    = 50;//�����̵�

const int PRO_PSHOP_BUY     = 51;//�����̵깺����Ʒ

const int PRO_PSHOP_DETAIL  = 52;//��ѯ�����̵���Ʒ

const int PRO_SAFE_TRADE   = 53;//��ȫ����

const int PRO_BANGPAI       = 54;//����

const int PRO_CLOSE_PLAYER = 55;//3��3���ӵ����

const int PRO_LIST_ESHOP    = 56;//�̳�

const int PRO_BUY_EITEM = 57;//�����̳���Ʒ

const int PRO_UPDATE_NPC    = 58;

const int PRO_ADD_NPC = 59;//���npc

const int PRO_DEL_NPC = 60;//del npc

const int PRO_SYNC_POS  = 61;//ͬ����ɫλ��

const int PRO_SYSTEM_INFO  = 62;//ϵͳ��Ϣ

const int PRO_EQUIP_ENFORCE = 63;//װ��ǿ��

const int PRO_ENFORCE_QUERY = 64;//��ѯװ��ǿ����Ϣ

const int PRO_EQUIP_BLUED = 65;//������װ

const int PRO_EQUIP_GREEND = 66;//������װ

const int PRO_EQUIP_REFINE = 67;//��װ��������ɸѡ

const int PRO_SKILL_DESC = 68;//��ѯ��������

const int PRO_OTHER_PET = 69;//��ѯ��ҳ���

const int PRO_UPDATE_PET_PARAM = 70;//���³�����Ϣ

const int PRO_ORNAMENT_SETTING = 71;//��Ƕ��Ʒ
const int PRO_ORNAMENT_CARVE    = 72;//��Ʒ����������

const int PRO_SWITCH_CHANNEL    = 73;//��������Ƶ��

const int PRO_SWITCH_INFO = 74;//��ѯ����ͨ���Ƿ�ر�

const int PRO_SYS_POP_MSG = 75;//ϵͳ������Ϣ

const int PRO_SYS_INFO1 = 76;//

const int PRO_SERVER_UPDATE_EQUIP = 77;//����װ��

/*
һ����Ѫ������							
ʹ�ð�����Ĳ�Ѫ���߽���ҽ�ɫ�ͳ�ս�����Ѫ�����ָ����������߲�
�����������ȶ���ҽ�ɫ���лظ���				
��Ѫ��ƷID��ʹ������˳��					
1��1804							
2��651-682							
������ƷID��ʹ������˳��					
1��1806							
2��701-730							
*/
const int PRO_EASY_RECOVER  = 78;//һ����Ѫ����

const int PRO_PSHOP_SOLD    = 79;//������Ʒ���������ŷ��ɸ�����Ϣ

const int PRO_MY_BANG       = 80;//�ҵİ���

const int PRO_CHANGE_FACE   = 81;//���½�ɫ����

const int PRO_CANCEL_TASK   = 82;//ȡ������

const int PRO_ITEM_DESC = 83;//��Ʒ����

const int PRO_CHARGE = 84;//�ͻ��˳�ֵ

const int PRO_QUERY_RLOC = 85;//

const int PRO_CLIENT_PATH = 87;//Ѱ·��Ϣ

const int PRO_GONGGAO   = 88;//ϵͳ����

const int PRO_AVAILABLE_TASK = 89;//�ɽ�����

const int PRO_SCENE_POS = 90;//���뿨������

const int PRO_SERVER_VERSION = 91;

const int PRO_SPEC_CHAT = 92;

const int MSG_SERVER_BEGIN_XING = 93;

const int MSG_ANSWER_XING = 94;

const int MSG_SERVER_END_XING = 95;

const int MSG_SERVER_MONSTER = 96;//��������

const int MSG_SERVER_ADD_MONSTER = 97;//��������

const int MSG_SERVER_REMOVE_MONSTER = 98;//ɾ������

const int MSG_CLIENT_MONSTER_BATTLE = 99;//����ս��

const int MSG_QUERY_SCENE   = 100;//��ȡ�����ļ�

const int MSG_CLIENT_ITEM_DEF = 101;//

const int MSG_SERVER_HEART_BEAT = 102;

const int MSG_LIST_SERVER = 103;

const int MSG_CLIENT_SYS_MAIL = 104;

const int MSG_CLIENT_LIST_TITLE = 105;

const int MSG_CLIENT_USE_TITLE = 106;

const int MSG_SERVER_JUMP_POINT = 107;

const int MSG_SERVER_QUERY_BILL = 108;

const int MSG_CLIENT_LIST_SHITU = 109;

const int MSG_SERVER_LIST_OBJECT = 110;

const int MSG_SERVER_ADD_OBJECT = 111;

const int MSG_SERVER_UPDATE_OBJECT = 112;

const int MSG_SERVER_REMOVE_OBJECT = 113;

const int MSG_CLIENT_DO_OBJECT  = 114;

const int MSG_SERVER_USE_RESULT = 115;
/*
+----+-----+------+------+
| OP | RES | VAL1 | VAL2 |
+----+-----+------+------+
|  1 |  1  |   4  |   4  |
+----+-----+------+------+
OP=1 ����ѧϰ����
������� VAL1 ѧ��ļ���ID VAL2 ���ǵļ���ID(����0����ʾû
�������ļ���)
*/

const int MSG_CLIENT_XIANGQIAN = 116;
/*
+------+------+-------+
| EIND | KONG | STONE |
+------+------+-------+
|   1  |   1  |    1  |
+------+------+-------+
EIND װ��λ��
KONG ��Ƕλ��
STONE ʯͷλ��
����������
MSG_SERVER_XIANGQIAN(116)
+-----+
| RES |
+-----+
|  1  |
+-----+
*/

const int MSG_CLIENT_LIST_FUQI = 117;
/*
+-------+------+-----+-----+------+------+-----+       
| STATE | FUQI | CID | LEN | NICK | CMAP | HOT |       
+-------+------+-----+-----+------+------+-----+       
|   1   |   1  |  4  |  2  |  Var |   2  |  2  |       
+-------+------+-----+-----+------+------+-----+       
STATE=0 û�н��        
FUQI ��0�� ��1��       
CMAP Ϊ���ߺ��ѵ�ǰλ��,����Ϊ0       
HOT  ���ܶ�   
*/

const int MSG_SERVER_VISUAL_EFFECT = 119;
/*
+------+
| TYPE |
+------+
|   1  |
+------+
TYPE=1 ��ͨ��
TYPE=2 �����
*/

const int MSG_CLIENT_DEL_CHAR = 118;

const int MSG_CLIENT_UNDEL_CHAR = 120;

const int MSG_SERVER_LIST_BLACK = 121;

const int MSG_CLIENT_ADD_BLACK = 122;

const int MSG_CLIENT_DEL_BLACK = 123;

const int MSG_SERVER_INVITE_ALLY = 124;

const int MSG_SERVER_NPC_MOVE = 125;

const int MSG_SERVER_NPC_EFFECT = 126;

const int MSG_SERVER_NPC_SAY = 127;

const int MSG_SERVER_XINSHI = 128;

const int MSG_SERVER_LIST_SKILL = 129;

const int MSG_CLIENT_LEARN_SKILL = 130;

const int MSG_SERVER_PROMPT = 131;

const int MSG_SERVER_DO_PET = 132;

const int GUANZHAN_ENTER_BATTLE  = 133;//��ս����ս��
const int LEAVE_GUANZHAN = 134;//�˳���ս
const int ENTER_GUANZHAN = 135;//��սս����ʼ
const int GUAGNZHAN_BATTLE        = 136;//��սս������
const int GUANGZHAN_BATTLE_OVER   = 137;//��սս������


const int UPDATE_OPSN_PACK = 138;//�����������ֿ⡢����ֿ�,��Ϣ����(byte)type,(byte)num,(int)time
                                //type:1�������ı���������2�����ֿ�������3��������ֿ�������numΪ������������timeΪ����ʱ��

const int PRO_TASK_INFO_EXT = 139;

/*�ƺ�,
�ͻ��˷��ʹ���Ϣѯ���Լ��ĳƺţ���Ϣ���ݿ�
������������ҳƺţ����ݣ�(byte)���� ��char*���ƺ�(byte)type��0δ��ã�1��ã�2ʹ�ã�
*/
const int MSG_TEXT_TITLE = 140;

//ʹ�óƺţ��ͻ��˷��ʹ���Ϣ����Ϣ����Ϊ��(char*)�ƺ��ı�
const int MSG_USE_TEXT_TITLE = 141;

//ʦͽ�������Ϣ
//�ͻ��˷�������(byte)0ʦ����1ͽ�ܰ�
//����������(char*)info,������|��Ϊ���з�
const int MSG_SHI_TU_USER_INFO = 142;

//�������
//�ͻ��˷��ͱ�ʾ��ȡ���,������
//���������ͱ�ʾ����ȡ������У�����(short)��Ʒid��num(byte) (short)��һ����ȡʣ��ʱ�䣨���ӣ���(char*)��Ʒ����
const int MSG_NEW_USER_GIFT = 143;

//ÿ�����
//�ͻ��˷��ͱ�ʾ��ȡ���,������
//���������ͱ�ʾ����ȡÿ����У�type(byte)(1��Ʒ��2���顢3Ǳ�ܣ�4����)����(short)�����Ʒid ������ֵ��num(byte),(short)��һ����ȡʣ��ʱ�䣨���ӣ���(char*)��Ʒ����
//(byte)��ʾ��Ʒ����,type(byte)(1��Ʒ��2���顢3Ǳ�ܣ�4����)����(short)�����Ʒid������ֵ,num(byte)
const int MSG_NORMAL_USER_GIFT = 144;

const int MSG_MGR = 0xfffe;

#ifdef QQ
const int MSG_QQ_LOGIN = 0xffff;
#endif

#endif
