#ifndef _SCRIPT_CALL_H_
#define _SCRIPT_CALL_H_
#include <list>
#include <map>
#include <string>
#include "self_typedef.h"

using namespace std;
class CUser;
class SItemTemplate;
class SMonsterTmpl;

struct SNpcPos
{
    int sceneId;
    int x;
    int y;
};

int Dialog(CUser *pUser,const char *name,const char *text);
int DialogT(CUser *pUser,const char *name,const char *text);
int Option(CUser *pUser,const char *name,const char *text,const char *opt);

//ϵͳ��Ϣ31
void SysInfo(CUser *pUser,const char *info);
//ϵͳ��Ϣ76
void SysInfo1(CUser *pUser,const char *info);

//����ʽ��Ϣ
void SMessage(CUser *pUser,const char *msg);

//��Ʒѡ��
void SelectItem(CUser *pUser,int i,int j);

//ѡ�����
void SelectPet(CUser *pUser,int petId,const char *name,const char *msg);

//����Ʒ,itemΪ��Ʒid���á�|������
void SellItem(CUser *pUser,int type,const char *items);//items "1|2|3����"

//�ر�npc����
void CloseInteract(CUser *pUser);

//�����ת
void TransportUser(CUser *pUser,int sceneId,uint8 x,uint8 y,uint8 face);
//��ұ�������ת
void UserJumpTo(CUser *pUser,uint16 sceneId,uint8 x,uint8 y,uint8 face);

//������ɳ���
//bool EnterBangPaiScene(CUser *pUser,uint8 x,uint8 y,uint8 face);
bool EnterBangPaiScene(CUser *pUser,int bId);//14,14,8

//�ű������Ʒ�����������ڳ�ʼ����Ʒ�б�
void AddItemTmpl(SItemTemplate*);

//�ű���ӹ�����������ڳ�ʼ������
//void AddMonsterTmpl(SMonsterTmpl*);

//ͨ������id�������
const char *GetMonsterName(int id);

//��ó�������
const char *GetSceneName(int id);

//�����Ʒ
SItemTemplate *GetItem(int itemId);

int AddHumanNpc(CUser *pUser,int xiang,int npcId,int sceneId,int x,int y,int timeOut);

const char *GetUserNpcName(CUser *pUser,int npcId);

int AddNpc(CUser *pUser,int npcId,const char *name,int sceneId,int x,int y,int timeOut = 0);

int AddDefaultNpc(CUser *pUser,int npcId,int sceneId,int x,int y,int timeOut = 0);

void DelNpc(CUser *pUser,int npcId);

void BeginFight(CUser *pUser,int monsterId,int level);

void HumanNpcFight(CUser *pUser,int ncpId);

//����ʹ����
void ZhaoTao1(CUser *pUser);
void ZhaoTao2(CUser *pUser);
void ZhaoTao3(CUser *pUser);

void ThreeKuLou(CUser *pUser);
//ʦ������ս��
void ShiMenFight(CUser *pUser);

void EpisodeBattle1(CUser *pUser);
void EpisodeBattle2(CUser *pUser);
void EpisodeBattle3(CUser *pUser);
void EpisodeBattle4(CUser *pUser);
void EpisodeBattle5(CUser *pUser);
void EpisodeBattle6(CUser *pUser);
void EpisodeBattle7(CUser *pUser);
void EpisodeBattle8(CUser *pUser);
void EpisodeBattle9(CUser *pUser);
void EpisodeBattle10(CUser *pUser);
void EpisodeBattle11(CUser *pUser);
void EpisodeBattle12(CUser *pUser);
void EpisodeBattle13(CUser *pUser);
void EpisodeBattle14(CUser *pUser);
void EpisodeBattle15(CUser *pUser);
void EpisodeBattle16(CUser *pUser);
void EpisodeBattle17(CUser *pUser);
void EpisodeBattle18(CUser *pUser);
void EpisodeBattle19(CUser *pUser);
void EpisodeBattle20(CUser *pUser);
void EpisodeBattle21(CUser *pUser);

void WenTianBattle(CUser *pUser);
void OpenPackBattle1(CUser *pUser);
void OpenPackBattle2(CUser *pUser);

//���׶�
void LanRuoBattle(CUser *pUser);
//���׶�
void LanRuoBattle3(CUser *pUser);
//�Ľ׶�
void LanRuoBattle4(CUser *pUser);
//��׶�
void LanRuoBattle5(CUser *pUser);

                    //�ӳ�
int GetTeamMemNum(CUser *pUser);
CUser *GetTeamMember1(CUser *pUser);
CUser *GetTeamMember2(CUser *pUser);

int GetNpcSceneId(int npcId);

SNpcPos GetNpcScenePos(int sceneId);

const char *GetNpcName(int npcId);

//�򿪱�����ѡ����Ʒ
void OpenPackage(CUser *pUser,int p);

//skills format "id1|id2|����"
void StudySkill(CUser *pUser,const char *skills);

void ListStudySkill(CUser *pUser,const char *skills);

//ѧϰ���ܣ������ڻ����Ҫ���������жϣ����۳�ѧϰ���󣬲�������������false
bool LearnSkill(CUser *pUser,int skillId);

void UserRest(CUser *pUser);

//���������ع�
void EnterLanRuoDiGong(CUser *pUser);

//������
void AddLouLuo(CUser *pUser);

void DelFaceNpc(CUser *pUser);

//�������
void AddWuNian(CUser *pUser);

//�������
void AddWuXin(CUser *pUser);

void LouLuoFight(CUser *pUser);
void WuNianFight(CUser *pUser);
void WuXinFight(CUser *pUser);
void DengLongGuiFight(CUser *pUser);
void BaiYueFight(CUser *pUser);
void ShuiGuiFight(CUser *pUser);

void ChuShiFight(CUser *pUser);

void QiLinFightYao(CUser*);
void QiLinFightXian(CUser*);

void UpdateNpcState(CUser *pUser,int npcId,int state);

//�������ɣ�type:0 ����Ϸ�Ҵ���(1000000)��type:1�õ��ߴ�����item 1816)
bool CreateBangPai(CUser *pUser,const char *name,int type);

void InputStr(CUser *pUser,const char *pMsg);

//TYPE=0 ��Ǯ,TYPE=1 ȡǮ
void BankMoney(CUser *pUser,int type);

//Ǯׯ�ӡ���Ǯ�������ڻ��Ǯת�Ƶ�������ϣ�money Ϊ���ȼ�Ǯ
int AddBankMoney(CUser *pUser,int money);

//����Ǯׯ��Ʒ
void SendBankItem(CUser *pUser);

//���Ǯׯ��Ʒ�������ڻ�Ӱ�����ɾ��
bool AddbankItem(CUser *pUser,uint8 pos,uint8 num);

//ɾ��Ǯׯ��Ʒ�������ڻ�������
bool GetBankItem(CUser *pUser,uint8 pos,uint8 num);

//���μ���
void ShiPinJianDing(CUser *pUser,int cost);

SNpcPos GetNpcPos(CUser *pUser);

SMonsterTmpl *GetMonster(int id);

void ShiBaiChengFa(CUser *pUser);

void AddBattleRes(CUser *pUser,int scriptId,int monsterId);
void RemoveBattleRes(CUser *pUser,int scriptId,int monsterId);

void CompoundOrnament(CUser *pUser);

void DoItem(CUser *pUser,int stype);

void CompoundItem(CUser *pUser,const char *str);
void CompoundMaterial(CUser *pUser,int id);

void SetHuoDong(bool);
void SetHuoDongBeiLv(int);
bool InHuoDong();
int GetHuoDongBeiLv();

//type:1�ȼ�����
void RankLevel(CUser *pUser,int type);
void RankLevel(CUser *pUser,int type,char *tag);

struct SUserAward
{
    int id;
    int num;
};
//id: <0û���ؽ���0�ؽ��������>0�ؽ���Ʒ
SUserAward GetAward(CUser *pUser);

//��������Ѿ���ȡ����
void SetGetAword(CUser *pUser);

int GetHour();

void SysInfoToAllUser(const char *msg);

void SaveDate(CUser *pUser,int type,int data);

int GetScriptVal(int id);
void SetScriptVal(int id,int val);

//����ʦ���ȼ�
void upgrade_master(CUser *pUser);

//���ͽ���б�
const char *get_disciple(CUser *pUser);

//���ʦ������
const char *get_master(CUser *pUser);

//�Ƿ���ʦ��
bool have_master(CUser *pUser);

//�Ƿ���ͽ��
bool have_disciple(CUser *pUser);

//��ɢʦͽ��ϵ(ͽ�ܵ���)
void cancel_master(CUser *pUser);

//��ɢʦͽ��ϵ(ʦ������)
void cancel_disciple(CUser *pUser,int id);

//�Ƿ������ʦ��
bool is_master(CUser *pUser);

//��ͽ,0�ɹ�,1,2,3
int do_master(CUser *pUser);

int master_level(CUser *pUser);

void ChuShi(CUser *pUser);

bool GetDiscipleAward(CUser *pUser,int type);
void DelDiscipleAward(CUser *pUser,int type);

int GetMasterAward(CUser *pUser,int type);
void DelMasterAward(CUser *pUser,int type);

uint32 GetMaster(CUser *pUser);

CUser *GetTeamLeader(CUser *pUser);

void EnterGuiYu(CUser *pUser,int level);

//�ҳ�����
void FindYaoNieBattle(CUser *pUser);

//���ܿ���
void JiBaiKSBattle(CUser *pUser);

//Ѱ��ս��
void XunBaoFight(CUser *pUser);

//�õ������пɹ���
SNpcPos GetCanWalkPos(int npcId);

//���,0�ɹ���1 ���ʯ���� 2 �Ѳ�ֹ���ʯͷ 3 ��Ʒ����
int ChaiFen(CUser *pUser,uint8 weaponPos,uint8 attrPos,uint8 stonePos);

//�ںϣ�0�ɹ���1 ʯͷ���� 2 ʯͷδ��ֹ� 3 ��Ʒ����
int RongHe(CUser *pUser,uint8 weaponPos,uint8 stonePos);

//ѡ������
void SelectAttr(CUser *pUser,uint8 pos);

char *GetPaiMing(int level);

int GetWeekDay();

//�г�ָ��ҳ�����б�
void ListBang(CUser *pUser,int page);

//��ĳһ������ս
//0��ս�ɹ�,1 ����ָ��ʱ��,2 λ�ײ���
//3 ����ս��,4 �Է��ٶ�С��0�İ�����ս 5�ް���
//6���˰ﲻ����ս
int DeclareWar(CUser *pUser,int bid);

//���ݰ���id,�õ����ɷ��ٶ�
int GetBangPros(CUser *pUser,int bid);

//�����ս����
bool EnterBangZhan(CUser *pUser,int bId);

//�����ս����
bool EnterBangZhan(CUser *pUser);

//��ս���񴥷�ս��
int FightDiaoXiang(CUser *pUser,int id);
//ս��ʤ����ֱ��ռ��õ���

//�鿴�������
int GetMeScore(CUser *pUser);

//�鿴�а����
int GetEnemyScore(CUser *pUser);

//�鿴���˹��׻���
int GetOwnScore(CUser *pUser);

//�Ͻ����
int ShangjiaoKuang(CUser *pUser);

//�鿴���
int GetBangKuang(CUser *pUser);

//�´ο��Գ�����Ӱսʿʣ��ʱ��
int GetMeiYingTime(CUser *pUser);

//������Ӱսʿ,����1ʱ���㣬2 ����Ȩ�޲���
int StartMeiYing(CUser *pUser);

int GetBZPosition(CUser *pUser);

//�õ�ռ�����İ���
// (0=�Ѱ�,1=�а�) 
int GetDiaoXiangState(CUser *pUser,int id);

//�õ�����ʣ�ౣ��ʱ��
int GetDiaoXiangTime(CUser *pUser,int id);

bool InWaZhongHuoDong();

char *GetHuoDongDi();

void DelHuoDongDi(int x,int y);

int GetWaZhongScene();

void ListWarBang(CUser *pUser);

const char *GetBangName(int id);

void UpdateUserInfo(CUser *pUser);

bool PlantResource(CUser *pUser,int ind);

int GainResource(CUser *pUser,int ind);

void ListResource(CUser *pUser);

//op=1 ����,op=2 �ƻ�,op=3 �޸�
void DoFarm(CUser *pUser,int op,int ind);

struct SResInfo
{
    int itemId;//��ֲ��������(��Ʒid)
    uint8 state;//״̬0������1���죬2���٣�4�ƻ�,8ˮ����ʧ
    int stateTime;//״̬ʣ��ʱ��
    int leftTime;//����ʣ��ʱ��
};

SResInfo GetResourceInfo(CUser *pUser,int ind);

//id=1 ����,id=2 ����,id=3 ����,id=4 ����
int GetBangResource(CUser *pUser,int id);

//�������ɵȼ�
//id=0 �������� id=1 ������ id=2 ������ id=3 �ٲ��� id=4 ������
//0�ɹ�
int UpgradeBang(CUser *pUser,int id);

//������ڳ�������id
int GetSceneBang(CUser *pUser);

//��ð��ɵȼ�
//id=0 ���� id=1 ������ id=2 ������ id=3 �ٲ��� id=4 ������
int GetBangLevel(CUser *pUser,int id);

//�õ������ʽ�
int GetBangMoney(CUser *pUser);
void AddBangMoney(CUser *pUser,int money);

//�һ��ﹱ
void DuiHuanBG(CUser *pUser,char *info);

//����0 δ���
//����1 �Ѿ������ �н�
//����2 �Ѿ���� δ�н�
//����-1 �����콱������
int GetAward2(CUser *pUser);

void SetAword2(CUser *pUser,int state);

bool AddPet(CUser *pUser,int monsterId);

void DonateBang(CUser *pUser);

void GetBangHistory(CUser *pUser);

//����"��Ŀ|��1|��2|��3|��4"
const char *GetQuestion();
const char *GetQuestion1();
const char *GetQuestion2();

char *IdentifyBook(CUser *pUser,uint8 pos);

//�ύ�ɼ�
//���� 1
//���� 2
//���� 3
void SubmitScore(CUser *pUser,int type,int score);

int GetDaTiPaiMing(CUser *pUser,int type);

void QiChongBattle1(CUser *pUser);
void QiChongBattle2(CUser *pUser);
void QiChongBattle3(CUser *pUser);
void QiChongBattle4(CUser *pUser);
void QiChongBattle5(CUser *pUser);
void QiChongBattle6(CUser *pUser);
void QiChongBattle7(CUser *pUser);
void QiChongBattle8(CUser *pUser);
void QiChongBattle9(CUser *pUser);
void QiChongBattle10(CUser *pUser);
void QiChongBattle11(CUser *pUser);
void QiChongBattle12(CUser *pUser);
void QiChongBattle13(CUser *pUser);
void QiChongBattle14(CUser *pUser);

void LangBattle1(CUser *pUser);
void LangBattle2(CUser *pUser);
void LangBattle3(CUser *pUser);
void LangBattle4(CUser *pUser);
void LangBattle5(CUser *pUser);
void LangBattle6(CUser *pUser);
void LangBattle7(CUser *pUser);
void LangBattle8(CUser *pUser);
void LangBattle9(CUser *pUser);
void LangBattle10(CUser *pUser);
void LangBattle12(CUser *pUser);

void ShiYaoYW1(CUser *pUser);
void ShiYaoYW2(CUser *pUser);
void ShiYaoYW3(CUser *pUser);
void ShiYaoYW4(CUser *pUser);
void ShiYaoYW5(CUser *pUser);
void ShiYaoYW6(CUser *pUser);
void ShiYaoYW7(CUser *pUser);
void ShiYaoYW8(CUser *pUser);
void ShiYaoYW9(CUser *pUser);
void ShiYaoYW10(CUser *pUser);

void ShiYaoYX1(CUser *pUser);
void ShiYaoYX2(CUser *pUser);
void ShiYaoYX3(CUser *pUser);
void ShiYaoYX4(CUser *pUser);
void ShiYaoYX5(CUser *pUser);
void ShiYaoYX6(CUser *pUser);
void ShiYaoYX7(CUser *pUser);
void ShiYaoYX8(CUser *pUser);
void ShiYaoYX9(CUser *pUser);
void ShiYaoYX10(CUser *pUser);

void ShiYaoPT1(CUser *pUser);
void ShiYaoPT2(CUser *pUser);
void ShiYaoPT3(CUser *pUser);
void ShiYaoPT4(CUser *pUser);
void ShiYaoPT5(CUser *pUser);
void ShiYaoPT6(CUser *pUser);
void ShiYaoPT7(CUser *pUser);
void ShiYaoPT8(CUser *pUser);
void ShiYaoPT9(CUser *pUser);
void ShiYaoPT10(CUser *pUser);

//ŭ�����(102)	20��
void NuYanQiLing(CUser *pUser);

//�ɽ�׷��/��ħ���1(45)	20��
void XianBing1(CUser *pUser);

//�ɽ�׷��/��ħ���2	60��
void XianBing2(CUser *pUser);


//�ɽ�׷��/��ħ���1(45)	20��
void YaoBing1(CUser *pUser);

//�ɽ�׷��/��ħ���2	60��
void YaoBing2(CUser *pUser);

//���ɴ�����(57)	50��
void MenPaiChuanSong(CUser *pUser);

//��������(50)	60��
void QingLongBaoBao(CUser *pUser);

//���ô���(39)	80��
void KuLouDaWang(CUser *pUser);

//���ˣ����飩
void JuQingXianRen(CUser *pUser);

//���루���飩
void JuQingQiLing(CUser *pUser);

//ڤ���������飩
void JuQingMingYao(CUser *pUser);

int GetChongZhi(CUser *pUser);

int CheckMarry(CUser *pUser);

int DoDivorce(CUser *pUser);

void GetWedding(CUser *pUser);

int EnterWedding(CUser *pUser,int id);

//type=1 ��ͨ���� type=2 ��������
int DoMarry(CUser *pUser,int type);

//�Ѿ���� ���ضԷ���ɫ�������򷵻�nil
const char *GetMarried(CUser *pUser);

int GetMarriedId(CUser *pUser);

int YanQing(CUser *pUser,int id,int num);

int GiveHongBao(CUser *pUser,int ind);

//ret�һ���ͨ����Ŀ 
int DuiHuanTB(CUser *pUser);

void InputNumber(CUser *pUser,int id);

bool IsBrideGroom(CUser *pUser);

//eind-����λ�� aind-����λ��
//���,0�ɹ���1 û����Ӧʯͷ 3 ����ֵΪ�� 4 ������
int ChaiFenSX(CUser *pUser,uint8 eind,uint8 aind);

void AddPetXiuWei(CUser *pUser,uint8 ind,int xw);

int GetMarryLeftTime(CUser *pUser);

//0�ɹ�
int ChangeCharName(CUser *pUser,char *name);

bool CanChangeName(CUser *pUser);

//�ύ�����ɳɼ�
void MLSChengJi(CUser *pUser,int val);

//��ѯ������������0û�μ�
int MLSGetPaiMing(CUser *pUser);

//0Ϊ�������,-1Ϊû�вμ�,-2��ȡ������
int MLSGetJiangLi(CUser *pUser);

//��ȡ����
void MLSLingJiang(CUser *pUser);

//��ʾ����ͨ������ 
void IPostTongji(CUser *pUser);

//��ʾͨ���� 
void ITongjiBang(CUser *pUser);

//��ʾ�ҷ��������� 
void IMyTongji(CUser *pUser);

//type=0 ͨ���� type=1 3���� 
//1 Ŀ���ǳƲ�����	 
//2 û��������Ʒ 
//3 û���㹻�ʽ� 
//0 ���񷢲��ɹ�
int PostTongji(CUser *pUser,int id,const char *name,int type,int money);

int ChangeTongji(CUser *pUser,int id,int money);

int CancelTongji(CUser *pUser,int id);

//1 �ѽӹ������� 
//2 ���������ʧ ��������� �����ѳ��� 
//3 ���ܽ�ȡ�Լ������� 0 �ɹ�
//5 ������
int TakeTongji(CUser *pUser,int id);

// �����Ƿ����� ���ڳ��� 
//char *GetUserInfo(CUser *pUser,int id);

// ����ʣ��ʱ�� ���ͽ���
//char *GetTongjiInfo(CUser *pUser,int id);

struct STongJiInfo
{
    int id;
    uint32 owner;//������
    uint32 roleId;//ͨ����
    int money;//���ͽ�Ǯ
    time_t t;//����ʱ��
    char name[MAX_NAME_LEN];//��ɱ������
    uint8 level;
    uint8 state;//�Ƿ����˽ӹ���1�ӹ�
    list<uint32> userList;//��ȡ������������
};

bool GetTongJiInfo(int id,STongJiInfo *pInfo);

void DelTongJiMiss(CUser *pUser,int id);

//�õ����˰�,"id1|id2"��û�з���NULL
char *GetAllyBang(CUser *pUser);

//���ɽ���
void AllyBang(CUser *pUser,int bid);

//��ɢ����
void UnallyBang(CUser *pUser,int bid);

//������˰���
int InviteAlly(CUser *pUser,int bid);

//�Ƿ��ܽ���,0 ���ԣ�1 �������������ޣ�2 ������ʱ�䣬 3 bid����ʱ�� 4 û�а��� 
//5 ��ս�в��ܽ���
int CheckAlly(CUser *pUser,int bid);

// ��ʾ�һ���
void IWWDuihuan(CUser *pUser);

// �ύ��ʯ
//void WWWaKuang(CUser *pUser,int id);

//�ύ��ʯ id,0-4��ľˮ����
void WWTijiaoKuang(CUser *pUser,int id);

//�õ���ʯ���� 
int WWGetKuang(CUser *pUser,int id);

//���þ�ʯ����
void WWSetKuang(CUser *pUser,int id,int num);

void FightCoupleTrial(CUser *pUser);

//�õ���ǰ�ɾ���սʤ������ 
int WWGetWinBang();

//�õ��ܾ�ʯ 
int WWGetTotalKuang();

//�õ����ߵĿ� 
int WWGetLostKuang();

bool InWorldWar();

bool CanWaKuang(CUser *pUser);

//�õ����� 
int WWGetJifen(CUser *pUser);

//�õ������ܻ��� 
int WWGetTotalJifen();

//���ֶһ�ͨ�� 
int WWDuihuanJifen(CUser *pUser);

void AddTolJiFen(int add);

int AgreeWWFangShou(CUser *pUser,uint8 agree,int bId);

bool IsFangShou(int bId);

//ʹ����Ʒ 63(ʯͷ�� 64 ����ɣ�
int UseItemToNpc(CUser *pUser,int id,int item);

//��ע 65��ȭ��
int Xiazhu(CUser *pUser,int guanjun,int yajun,int num);

//�õ���ͨ��ע�� 
int GetNormalZhu(int guanjun);

//�õ�ר����ע�� 
int GetExpertZhu(int guanjun,int yajun);

//�õ��ҵ���ע�� 
int GetMyZhu(CUser *pUser,int guanjun,int yajun);

bool InSaiPao();

int GetJingcaiAward(CUser *pUser);

char *GetMingCi();
//270-276 �ǻʱ�䲻�ܽ���

void EnterFuBenWJ(CUser *pUser);

//1��ͨ��2Ӣ��
void EnterFuBenShiYao(CUser *pUser,int level);

/*
�����ԣ�
EAAqixue
��Ѫ3000+

EAAfangyu
����3000+

EAAshanghai
�˺�3000+

�����ԣ�
EAAquanjineng
ȫ����100+

EAAquanshuxing
ȫ����100+

EAAbishalv
��100+

�¹�ʯ��
EAAshui
ˮ��100+
EAAhuo
����100+
EAAmu
ľ��100+

90��+10���ף���ˮ�޼ף�
�����ԣ�
��Ѫ3000+
����3000+
�˺�3000+
�����ԣ�
����400+
ȫ����100+
����400+
�¹�ʯ��
����100+
����100+
*/
bool FuBenKaiJia(CUser *pUser,uint8 pos);

//TYPE=1 ��ͨ��
//TYPE=2 �����
//scene 0,���г���
void TeXiao(int type,int scene);

void SendSysChannelMsg(const char *info);

//��·��ս��
void SaiPaoFight(CUser *pUser);

void DiaoYuFight(CUser *pUser);

//0��ȷ��1����ȡ��2��
int FindJiHuoMa(char *str);

//�򿪼��Ž���
void OpenXinShi(CUser *pUser);

//�г��յ�����
void ListXinShi(CUser *pUser);

int GetLeiTaiJiFen(CUser *pUser,int mapId);

void TiJiaoYiJian(CUser *pUser,char *info);

void SendBankPet(CUser *pUser);

//pos����λ��
bool SaveBankPet(CUser *pUser,uint8 pos);

//pos�ڳ����̵��е�λ��
bool GetBankPet(CUser *pUser,uint8 pos);

void SendMissionInfo(CUser *pUser,uint16 missionId,const char *info);

//���,type=1��Ǯ��ݣ�2 ���ʯ���
//0�ɹ���1��Ǯ���㣬2û�н���ʯ,3û�ж��飬4��ݳƺ����ƷǷ�,5���н��
int JieBai(CUser *pUser,int type,const char *title);

//0�ɹ�
int JieSanJieBai(CUser *pUser);

const char *GetJieBaiUser(CUser *pUser);

int CanJieBai(CUser *pUser);

void DelRole(CUser *pUser,int timeOut);

#endif
