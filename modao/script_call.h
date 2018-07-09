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

//系统消息31
void SysInfo(CUser *pUser,const char *info);
//系统消息76
void SysInfo1(CUser *pUser,const char *info);

//弹出式消息
void SMessage(CUser *pUser,const char *msg);

//物品选择
void SelectItem(CUser *pUser,int i,int j);

//选择宠物
void SelectPet(CUser *pUser,int petId,const char *name,const char *msg);

//卖物品,item为物品id，用“|”隔开
void SellItem(CUser *pUser,int type,const char *items);//items "1|2|3……"

//关闭npc交互
void CloseInteract(CUser *pUser);

//玩家跳转
void TransportUser(CUser *pUser,int sceneId,uint8 x,uint8 y,uint8 face);
//玩家本场景跳转
void UserJumpTo(CUser *pUser,uint16 sceneId,uint8 x,uint8 y,uint8 face);

//进入帮派场景
//bool EnterBangPaiScene(CUser *pUser,uint8 x,uint8 y,uint8 face);
bool EnterBangPaiScene(CUser *pUser,int bId);//14,14,8

//脚本添加物品，程序中用于初始化物品列表
void AddItemTmpl(SItemTemplate*);

//脚本添加怪物，程序中用于初始化怪物
//void AddMonsterTmpl(SMonsterTmpl*);

//通过怪物id获得名字
const char *GetMonsterName(int id);

//获得场景名字
const char *GetSceneName(int id);

//获得物品
SItemTemplate *GetItem(int itemId);

int AddHumanNpc(CUser *pUser,int xiang,int npcId,int sceneId,int x,int y,int timeOut);

const char *GetUserNpcName(CUser *pUser,int npcId);

int AddNpc(CUser *pUser,int npcId,const char *name,int sceneId,int x,int y,int timeOut = 0);

int AddDefaultNpc(CUser *pUser,int npcId,int sceneId,int x,int y,int timeOut = 0);

void DelNpc(CUser *pUser,int npcId);

void BeginFight(CUser *pUser,int monsterId,int level);

void HumanNpcFight(CUser *pUser,int ncpId);

//召讨使任务
void ZhaoTao1(CUser *pUser);
void ZhaoTao2(CUser *pUser);
void ZhaoTao3(CUser *pUser);

void ThreeKuLou(CUser *pUser);
//师门任务战斗
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

//二阶段
void LanRuoBattle(CUser *pUser);
//三阶段
void LanRuoBattle3(CUser *pUser);
//四阶段
void LanRuoBattle4(CUser *pUser);
//五阶段
void LanRuoBattle5(CUser *pUser);

                    //队长
int GetTeamMemNum(CUser *pUser);
CUser *GetTeamMember1(CUser *pUser);
CUser *GetTeamMember2(CUser *pUser);

int GetNpcSceneId(int npcId);

SNpcPos GetNpcScenePos(int sceneId);

const char *GetNpcName(int npcId);

//打开背包，选择物品
void OpenPackage(CUser *pUser,int p);

//skills format "id1|id2|……"
void StudySkill(CUser *pUser,const char *skills);

void ListStudySkill(CUser *pUser,const char *skills);

//学习技能，函数内会对需要条件进行判断，并扣除学习需求，不满足条件返回false
bool LearnSkill(CUser *pUser,int skillId);

void UserRest(CUser *pUser);

//进入兰若地宫
void EnterLanRuoDiGong(CUser *pUser);

//添加喽罗
void AddLouLuo(CUser *pUser);

void DelFaceNpc(CUser *pUser);

//添加无念
void AddWuNian(CUser *pUser);

//添加无心
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

//创建帮派，type:0 用游戏币创建(1000000)，type:1用道具创建（item 1816)
bool CreateBangPai(CUser *pUser,const char *name,int type);

void InputStr(CUser *pUser,const char *pMsg);

//TYPE=0 存钱,TYPE=1 取钱
void BankMoney(CUser *pUser,int type);

//钱庄加、减钱，函数内会把钱转移到玩家身上，money 为负既减钱
int AddBankMoney(CUser *pUser,int money);

//发送钱庄物品
void SendBankItem(CUser *pUser);

//添加钱庄物品，函数内会从包裹中删除
bool AddbankItem(CUser *pUser,uint8 pos,uint8 num);

//删除钱庄物品，函数内会加入包裹
bool GetBankItem(CUser *pUser,uint8 pos,uint8 num);

//首饰鉴定
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

//type:1等级排行
void RankLevel(CUser *pUser,int type);
void RankLevel(CUser *pUser,int type,char *tag);

struct SUserAward
{
    int id;
    int num;
};
//id: <0没有重奖，0重奖已领过，>0重奖物品
SUserAward GetAward(CUser *pUser);

//设置玩家已经领取奖励
void SetGetAword(CUser *pUser);

int GetHour();

void SysInfoToAllUser(const char *msg);

void SaveDate(CUser *pUser,int type,int data);

int GetScriptVal(int id);
void SetScriptVal(int id,int val);

//提升师父等级
void upgrade_master(CUser *pUser);

//获得徒弟列表
const char *get_disciple(CUser *pUser);

//获得师傅名字
const char *get_master(CUser *pUser);

//是否有师父
bool have_master(CUser *pUser);

//是否有徒弟
bool have_disciple(CUser *pUser);

//解散师徒关系(徒弟调用)
void cancel_master(CUser *pUser);

//解散师徒关系(师父调用)
void cancel_disciple(CUser *pUser,int id);

//是否可以做师父
bool is_master(CUser *pUser);

//收徒,0成功,1,2,3
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

//找出妖孽
void FindYaoNieBattle(CUser *pUser);

//击败狂兽
void JiBaiKSBattle(CUser *pUser);

//寻宝战斗
void XunBaoFight(CUser *pUser);

//得到场景中可过点
SNpcPos GetCanWalkPos(int npcId);

//拆分,0成功，1 拆分石不符 2 已拆分过的石头 3 物品不符
int ChaiFen(CUser *pUser,uint8 weaponPos,uint8 attrPos,uint8 stonePos);

//融合，0成功，1 石头不符 2 石头未拆分过 3 物品不符
int RongHe(CUser *pUser,uint8 weaponPos,uint8 stonePos);

//选择属性
void SelectAttr(CUser *pUser,uint8 pos);

char *GetPaiMing(int level);

int GetWeekDay();

//列出指定页帮派列表
void ListBang(CUser *pUser,int page);

//对某一帮派宣战
//0宣战成功,1 不是指定时间,2 位阶不够
//3 已宣战过,4 对繁荣度小于0的帮派宣战 5无帮派
//6结盟帮不能宣战
int DeclareWar(CUser *pUser,int bid);

//根据帮派id,得到帮派繁荣度
int GetBangPros(CUser *pUser,int bid);

//进入帮战场景
bool EnterBangZhan(CUser *pUser,int bId);

//进入帮战场景
bool EnterBangZhan(CUser *pUser);

//帮战雕像触发战斗
int FightDiaoXiang(CUser *pUser,int id);
//战斗胜利后直接占领该雕像

//查看本帮积分
int GetMeScore(CUser *pUser);

//查看敌帮积分
int GetEnemyScore(CUser *pUser);

//查看本人贡献积分
int GetOwnScore(CUser *pUser);

//上交矿产
int ShangjiaoKuang(CUser *pUser);

//查看矿产
int GetBangKuang(CUser *pUser);

//下次可以出动魅影战士剩余时间
int GetMeiYingTime(CUser *pUser);

//出动魅影战士,返回1时矿不足，2 人物权限不足
int StartMeiYing(CUser *pUser);

int GetBZPosition(CUser *pUser);

//得到占领雕像的帮派
// (0=友帮,1=敌帮) 
int GetDiaoXiangState(CUser *pUser,int id);

//得到雕像剩余保护时间
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

//op=1 加速,op=2 破坏,op=3 修复
void DoFarm(CUser *pUser,int op,int ind);

struct SResInfo
{
    int itemId;//种植作物类型(物品id)
    uint8 state;//状态0正常，1成熟，2加速，4破坏,8水土流失
    int stateTime;//状态剩余时间
    int leftTime;//成熟剩余时间
};

SResInfo GetResourceInfo(CUser *pUser,int ind);

//id=1 精铁,id=2 灵玉,id=3 清莲,id=4 玄晶
int GetBangResource(CUser *pUser,int id);

//提升帮派等级
//id=0 升级帮派 id=1 兵器阁 id=2 守御阁 id=3 百草堂 id=4 试炼堂
//0成功
int UpgradeBang(CUser *pUser,int id);

//获得所在场景帮派id
int GetSceneBang(CUser *pUser);

//获得帮派等级
//id=0 帮派 id=1 兵器阁 id=2 守御阁 id=3 百草堂 id=4 试炼堂
int GetBangLevel(CUser *pUser,int id);

//得到帮派资金
int GetBangMoney(CUser *pUser);
void AddBangMoney(CUser *pUser,int money);

//兑换帮贡
void DuiHuanBG(CUser *pUser,char *info);

//返回0 未领过
//返回1 已经领过奖 中奖
//返回2 已经领过 未中奖
//返回-1 不在领奖名单中
int GetAward2(CUser *pUser);

void SetAword2(CUser *pUser,int state);

bool AddPet(CUser *pUser,int monsterId);

void DonateBang(CUser *pUser);

void GetBangHistory(CUser *pUser);

//返回"题目|答案1|答案2|答案3|答案4"
const char *GetQuestion();
const char *GetQuestion1();
const char *GetQuestion2();

char *IdentifyBook(CUser *pUser,uint8 pos);

//提交成绩
//乡试 1
//会试 2
//殿试 3
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

//怒焰金麟(102)	20倍
void NuYanQiLing(CUser *pUser);

//仙界追兵/妖魔鬼怪1(45)	20倍
void XianBing1(CUser *pUser);

//仙界追兵/妖魔鬼怪2	60倍
void XianBing2(CUser *pUser);


//仙界追兵/妖魔鬼怪1(45)	20倍
void YaoBing1(CUser *pUser);

//仙界追兵/妖魔鬼怪2	60倍
void YaoBing2(CUser *pUser);

//门派传送人(57)	50倍
void MenPaiChuanSong(CUser *pUser);

//青龙宝宝(50)	60倍
void QingLongBaoBao(CUser *pUser);

//骷髅大王(39)	80倍
void KuLouDaWang(CUser *pUser);

//仙人（剧情）
void JuQingXianRen(CUser *pUser);

//麒麟（剧情）
void JuQingQiLing(CUser *pUser);

//冥妖王（剧情）
void JuQingMingYao(CUser *pUser);

int GetChongZhi(CUser *pUser);

int CheckMarry(CUser *pUser);

int DoDivorce(CUser *pUser);

void GetWedding(CUser *pUser);

int EnterWedding(CUser *pUser,int id);

//type=1 普通婚礼 type=2 豪华婚礼
int DoMarry(CUser *pUser,int type);

//已经结婚 返回对方角色名，否则返回nil
const char *GetMarried(CUser *pUser);

int GetMarriedId(CUser *pUser);

int YanQing(CUser *pUser,int id,int num);

int GiveHongBao(CUser *pUser,int ind);

//ret兑换的通宝数目 
int DuiHuanTB(CUser *pUser);

void InputNumber(CUser *pUser,int id);

bool IsBrideGroom(CUser *pUser);

//eind-宠铠位置 aind-属性位置
//拆分,0成功，1 没有相应石头 3 属性值为空 4 包裹满
int ChaiFenSX(CUser *pUser,uint8 eind,uint8 aind);

void AddPetXiuWei(CUser *pUser,uint8 ind,int xw);

int GetMarryLeftTime(CUser *pUser);

//0成功
int ChangeCharName(CUser *pUser,char *name);

bool CanChangeName(CUser *pUser);

//提交马拉松成绩
void MLSChengJi(CUser *pUser,int val);

//查询马拉松排名，0没参加
int MLSGetPaiMing(CUser *pUser);

//0为幸运玩家,-1为没有参加,-2领取过奖励
int MLSGetJiangLi(CUser *pUser);

//领取奖励
void MLSLingJiang(CUser *pUser);

//显示发布通缉界面 
void IPostTongji(CUser *pUser);

//显示通缉榜 
void ITongjiBang(CUser *pUser);

//显示我发布的任务 
void IMyTongji(CUser *pUser);

//type=0 通缉令 type=1 3万金币 
//1 目标昵称不存在	 
//2 没有所需物品 
//3 没有足够资金 
//0 任务发布成功
int PostTongji(CUser *pUser,int id,const char *name,int type,int money);

int ChangeTongji(CUser *pUser,int id,int money);

int CancelTongji(CUser *pUser,int id);

//1 已接过此任务 
//2 任务过期消失 任务已完成 任务已撤销 
//3 不能接取自己的任务 0 成功
//5 任务满
int TakeTongji(CUser *pUser,int id);

// 返回是否在线 所在场景 
//char *GetUserInfo(CUser *pUser,int id);

// 返回剩余时间 悬赏奖励
//char *GetTongjiInfo(CUser *pUser,int id);

struct STongJiInfo
{
    int id;
    uint32 owner;//发布者
    uint32 roleId;//通缉者
    int money;//悬赏金钱
    time_t t;//结束时间
    char name[MAX_NAME_LEN];//被杀者名字
    uint8 level;
    uint8 state;//是否有人接过，1接过
    list<uint32> userList;//领取过此任务的玩家
};

bool GetTongJiInfo(int id,STongJiInfo *pInfo);

void DelTongJiMiss(CUser *pUser,int id);

//得到结盟帮,"id1|id2"，没有返回NULL
char *GetAllyBang(CUser *pUser);

//帮派结盟
void AllyBang(CUser *pUser,int bid);

//解散结盟
void UnallyBang(CUser *pUser,int bid);

//邀请结盟帮派
int InviteAlly(CUser *pUser,int bid);

//是否能结盟,0 可以，1 结盟数量到上限，2 本结盟时间， 3 bid结盟时间 4 没有帮派 
//5 宣战中不能结盟
int CheckAlly(CUser *pUser,int bid);

// 显示兑换表
void IWWDuihuan(CUser *pUser);

// 提交晶石
//void WWWaKuang(CUser *pUser,int id);

//提交晶石 id,0-4金木水火土
void WWTijiaoKuang(CUser *pUser,int id);

//得到晶石数量 
int WWGetKuang(CUser *pUser,int id);

//设置晶石数量
void WWSetKuang(CUser *pUser,int id,int num);

void FightCoupleTrial(CUser *pUser);

//得到当前仙境大战胜利帮派 
int WWGetWinBang();

//得到总晶石 
int WWGetTotalKuang();

//得到抢走的矿 
int WWGetLostKuang();

bool InWorldWar();

bool CanWaKuang(CUser *pUser);

//得到积分 
int WWGetJifen(CUser *pUser);

//得到帮派总积分 
int WWGetTotalJifen();

//积分兑换通宝 
int WWDuihuanJifen(CUser *pUser);

void AddTolJiFen(int add);

int AgreeWWFangShou(CUser *pUser,uint8 agree,int bId);

bool IsFangShou(int bId);

//使用物品 63(石头） 64 （鱼干）
int UseItemToNpc(CUser *pUser,int id,int item);

//下注 65（拳）
int Xiazhu(CUser *pUser,int guanjun,int yajun,int num);

//得到普通下注数 
int GetNormalZhu(int guanjun);

//得到专家下注数 
int GetExpertZhu(int guanjun,int yajun);

//得到我的下注数 
int GetMyZhu(CUser *pUser,int guanjun,int yajun);

bool InSaiPao();

int GetJingcaiAward(CUser *pUser);

char *GetMingCi();
//270-276 非活动时间不能进入

void EnterFuBenWJ(CUser *pUser);

//1普通，2英雄
void EnterFuBenShiYao(CUser *pUser,int level);

/*
蓝属性：
EAAqixue
气血3000+

EAAfangyu
防御3000+

EAAshanghai
伤害3000+

绿属性：
EAAquanjineng
全技能100+

EAAquanshuxing
全属性100+

EAAbishalv
狂暴100+

月光石：
EAAshui
水相100+
EAAhuo
火相100+
EAAmu
木箱100+

90级+10铠甲（避水兽甲）
蓝属性：
气血3000+
防御3000+
伤害3000+
绿属性：
力量400+
全属性100+
灵力400+
月光石：
金相100+
土相100+
*/
bool FuBenKaiJia(CUser *pUser,uint8 pos);

//TYPE=1 普通礼花
//TYPE=2 结婚礼花
//scene 0,所有场景
void TeXiao(int type,int scene);

void SendSysChannelMsg(const char *info);

//拦路虎战斗
void SaiPaoFight(CUser *pUser);

void DiaoYuFight(CUser *pUser);

//0正确，1已领取，2无
int FindJiHuoMa(char *str);

//打开寄信界面
void OpenXinShi(CUser *pUser);

//列出收到的信
void ListXinShi(CUser *pUser);

int GetLeiTaiJiFen(CUser *pUser,int mapId);

void TiJiaoYiJian(CUser *pUser,char *info);

void SendBankPet(CUser *pUser);

//pos宠物位置
bool SaveBankPet(CUser *pUser,uint8 pos);

//pos在宠物商店中的位置
bool GetBankPet(CUser *pUser,uint8 pos);

void SendMissionInfo(CUser *pUser,uint16 missionId,const char *info);

//结拜,type=1金钱结拜，2 结伴石结拜
//0成功，1金钱不足，2没有结义石,3没有队伍，4结拜称号名称非法,5已有结拜
int JieBai(CUser *pUser,int type,const char *title);

//0成功
int JieSanJieBai(CUser *pUser);

const char *GetJieBaiUser(CUser *pUser);

int CanJieBai(CUser *pUser);

void DelRole(CUser *pUser,int timeOut);

#endif
