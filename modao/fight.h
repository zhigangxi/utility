#ifndef _FIGHT_H_
#define _FIGHT_H_
#include "hash_table.h"
#include "self_typedef.h"
#include "user.h"
#include "monster.h"
#include <boost/thread.hpp>
#include <boost/any.hpp>

class CSocketServer;
class CNetMessage;
class COnlineUser;

const int FIGHT_TIME_OUT = 1800;
const int MATCH_TIME_OUT = 600;

const uint16 SKILL_TAO_PAO  = 200;
const uint16 SKILL_ZI_BAO   = 201;
const uint16 SKILL_ZHAO_HUAN    = 202;
const uint16 SHAOTAO_TAO_PAO = 300;
const uint16 YUGUAI_TAO_PAO = 301;
const uint16 MISSION21_BOSS = 302;

enum EFightJiangLi
{
	/***************
	  30ֻ�ֽ���
	  ÿ��: ���飺����ȼ�*100
	  5%����ͼ��
	  5%������Ȧ
	  5%��˿��
	  5%��ָͼ��
	  5%���ƽ�ָ
	  5%��Ȼ��
	  5%����ͼ��
	  3%���ƻ���
	  2%���ʯ
	 ***************/
	EFJLType1 = 1,

	/*
	   ��Ů����
	   ���飺����ȼ�*500
	   Ǳ�ܣ�����ȼ�*500
	   ������ȡ1-3��
	   10%�հ���ˮ��
	   10%����ͼ��
	   5%������Ȧ
	   10%��˿��
	   5%��ָͼ��
	   5%���ƽ�ָ
	   5%��Ȼ��
	   5%����ͼ��
	   5%���ƻ���
	   5%���ʯ
	   10%��ͭ��Ȧ
	   10%��ʹ��ָ
	   5%��ͭ����
	   10%����ˮ���ϳɷ�����
	   */
	EFJLType2,

	/*
	   ���ֽܹ���
	   ���飺����ȼ�*100
	   ���У�����ȼ�*12
	   30%��ˮ���ϳɷ�
	   70%�հ���ˮ��
	   */
	EFJLType3,

	/*
	   ��ը�ֽ���
	   ���飺����ȼ�*100
	   ���У�����ȼ�*12
	   50%��ˮ���ϳɷ�
	   50%�հ���ˮ��
	   */
	EFJLType4,

	/*
	   ���ѽ���
	   ���飺����ȼ�*1000
	   Ǳ�ܣ�����ȼ�*1000
	   ���У�����ȼ�*15
	   ���䣺����3�ֶ�����������ˮ��1�����ϳɷ�5����20����Ʒѡһ��
	   �������Ե���ˮ������*1
	   ���������������������һ��������ֵΪ��
	   60%  ���ֵ�� 20%~30%
	   30%  ���ֵ�� 30%~40%
	   10%  ���ֵ�� 40%~50%
	   ��ˮ���ϳɷ�����*5
	   ͭ��Ȧ �� ͭ��ָ �� ͭ���� ȡһ��
	   */
	EFJLType5,

	EFJLType6,//���飺200
	EFJLType7,//���飺500
	EFJLType8,//���飺1000

	EFJLType9,//ʮ��Ұ��
	EFJLType10,//ʮ����ͨ
	EFJLType11,//ʮ��Ӣ��
};

class CFight
{
private:
public:
	enum EFightType
	{
		EFTMeetMonster = 1,//Ұ����
		EFTPlayerPk = 2,//���PK
		EFTPlayerQieCuo = 3,//�д�
		EFTScript = 4,//�ű�����

		EFTNianShou = 5,//����ս��

		EFTGuiYu = 6,//����ս��

		EFTMatch = 7,//����

		EFMeiYIng = 8,//��Ӱս��

		EFTDiaoXiang = 9,

		EFGuiJie = 10,//��ڻ
		EFVisibleMonsterMis = 11,
	};

	enum EOptionType
	{
		EOTNormal = 1,
		EOTSkill  = 2,
		EOTUseItem = 3,// ʹ�õ��� OPV=�������ڱ�������
		EOTRecovery = 4,// ���� OPV=0
		EOTCallUp = 5,// �ٻ� OPV=��������
		EOTCallBack = 6,// �ٻ� 
		EOTProtect = 7,// ����
		EOTCatch = 8,// ��׽
		EOTEscape = 9,// ����
		EOTSpecial = 10,//���⼼��

		EOTSpeek = 13,//���ּ���
		EOTZhaoHuan = 14,//�ٻ�����
	};
	CFight()
	{
		Clear();
	}
	void Clear();
	//�����ң�����pos
	uint8 AddUser(ShareUserPtr user,uint8 pos);
	//uint8 AddPet(uint32 id);
	uint8 AddMonster(ShareMonsterPtr monster,uint8 pos);

	void BeginFight(CSocketServer &sock,CScene *pScene);
	void UserBattle(CNetMessage &msg,CUser *pUser);

	uint8 GetFightNum()
	{
		return m_memNum;
	}
	uint32 GetId()
	{
		return m_id;
	}
	void SetId(uint32 id)
	{
		m_id = id;
	}
	void DelMember(uint8 pos);

	CUser *GetUser(uint8 pos);
	SMonsterInst *GetMonster(uint8 pos);
	SPet *GetPet(uint8 pos);

	bool IsFightEnd();
	void SetFightType(EFightType type)
	{
		m_type = type;
		if(m_type == EFTMatch)
			m_timeOut = MATCH_TIME_OUT;
		else
			m_timeOut = FIGHT_TIME_OUT;
	}
	int GetMonsterNum(int id); 
	void BroadcastMsg(CNetMessage &msg);

	bool FightTimeout();
	void Logout(uint8 pos);

	int monsterId1;
	int monsterId2;
	int GetSpeedImprove(uint8);

	int GetDaoQian6(int turn,int type);

	int GetDaoQian5(int turn,int type);
	bool ReBegin(CSocketServer &sock,ShareUserPtr pUser);
	void SaveFightLog();
	void AddShengWang();

	uint16 GetVisibleMonsterId()
	{
		return m_visibleMonsterId;
	}
	void SetVisibleMonsterId(uint16 id)
	{
		m_visibleMonsterId = id;
	}
	void SetDelNpc(uint8 x,uint8 y)
	{
		m_delNpcX = x;
		m_delNpcy = y;
	}
	void SetDiaoXiangId(int id)
	{
		m_diaoxiangId = id;
	}
	void SetJLType(int type)//���ý�������
	{
		m_jiangliType = type;
	}
	void SetCanTaoPao(bool f)
	{
		m_canTaoPao = f;
	}
	int GetUseTime()
	{
		return GetSysTime() - m_beginTime;
	}
	void GuanZhan(CUser*);
	void LeaveGuanZhan(CUser*);
	void SendGuanZhanOver();
private:
	uint16 GetPkVal(uint8 pos)
	{
		if((pos <= 0) || (pos > MAX_MEMBER))
			return 0;
		return m_members[pos-1].srcPkVal;
	}
	void GiveJiangLi9(CUser *pUser);
	void GiveJiangLi10(CUser *pUser);
	void GiveJiangLi11(CUser *pUser);

	bool TongJiMiss();
	void TongJiMsg();

	void Mission21(CUser *pUser,int state);
	uint8 GetLianJiShu(uint8 pos,int &lianjiAdd);
	int GetBaoJi(uint8 src,int &baojiAdd,int baojiLvAdd = 0);
	int GetFanZhen(uint8 tar);

	void AddJiangLi(CUser *pUser,int state);
	uint8 SpecialSkill(uint8 src,uint8 target,uint8 skillId,CNetMessage &msg);

	void SetAllUserDie();
	bool ChuShiFightEnd();
	void ChangeHuanYinPos();
	//ս���в������;�
	bool NotDecreaseNaiJiu();
	void UserFightEnd(uint8 pos,list<uint32> &userList);
	void MatchUserFightEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
			int state,uint8 res);
	//state:0ʤ����1 ������2 ����
	void OtherTypeUserFightEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
			int state,int exp,int money,uint8 res);

	void MeiYingEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
			int state,uint8 res);
	void SetSkill23State(uint8 pos,bool flag)
	{
		if((pos > 0) && (pos <= MAX_MEMBER))
		{
			m_members[pos-1].useSkill23 = flag;
		}
	}
	bool GetSkill23State(uint8 pos)
	{
		if((pos > 0) && (pos <= MAX_MEMBER))
		{
			return m_members[pos-1].useSkill23;
		}
		return false;
	}
	//pos 0��ʼ
	void MonsterSkillCeLue(SMonsterInst *pMonster,uint8 pos);

	void GuiYuEnd();
	void SendPkInfo(uint8 dieGroup);
	void SendMatchInfo(uint8 type);
	void NianShouJiangLi(CUser *,CNetMessage&);
	void GuiJieJiangLi(CUser *pUser,CNetMessage &msg);
	void NianShouEnd();
	void VisibleMonsterMisEnd();

	void NoLockBeginFight(CSocketServer &sock,CNetMessage &msg,list<uint32> &userList);
	void UpdateUserInfo(CUser *pUser,list<uint32> &userList);

	void SendUserSelect(uint8 pos);

	//ս������������ҽ��н���,�����Ƿ���Ҫ����
	bool JiangCheng(uint8 state,CUser *pUser,uint8 pos,int gongTolDaoHang,int fangTolDaoHang);

	void GiveJiangCheng(CUser *pUser,bool isGong,bool victory,
			uint8 gongLevel,uint8 fangLevel,
			int gongPkVal,int fangPkVal,
			int gongTolDaoHang,int fangTolDaoHang,
			stringstream &info);
	void GetGongFangInfo(uint8 &gongLevel,uint8 &fangLevel,
			int &gongPkVal,int &fangPkVal,
			uint8 &gongNum,uint8 &fangNum,
			int &gongBang,int &fangBang);

	bool IsGongFang(uint8 pos);
	int GetGongTolDaoHang();
	int GetFangTolDaoHang();

	void DropItem(CUser *pUser,uint8 pos,CNetMessage &msg);
	const static int FIGHT_TIMEOUT = 30;//��
	int GetMaxHp(uint8 pos);
	//1-6������1��7��12������2����û���ⷵ��0
	uint8 OneGroupAllDie();

	void CancelAutoFight(CUser *pUser);

	bool AllUserAutoFight(uint16 &userMask);

	//����������
	int CalculateHitRatio(uint8 src,uint8 target);

	uint8 AddPet(CUser *pUser,SharePetPtr pet,uint8 pos);
	uint8 ZhaoHuanPet(uint8 user,uint8 pet,CNetMessage &msg);
	uint8 NormalButtle(uint8 src,uint8 target,CNetMessage &msg);
	uint8 SkillButtle(uint8 src,uint8 target,uint8 skillId,CNetMessage &msg);

	uint8 ZhaoHuanNiu(uint8 src,CNetMessage &msg);
	uint8 ZhaoHuanGuMu(uint8 src,CNetMessage &msg);
	uint8 ZhaoHuanSkill(uint8 src,CNetMessage &msg);
	uint8 ZhaoHuanHuLu(uint8 src,CNetMessage &msg);
	uint8 ZhaoHuanLang(uint8 src,CNetMessage &msg);
	uint8 ZhaoHuanShiHou(uint8 src,CNetMessage &msg);
	int SkillUseMp(uint8 pos,uint8 skillId,uint8 skillLevel,
			uint8 skillType,uint8 skillJie);

	uint8 CatchMonster(uint8 src,uint8 target,CNetMessage &msg);

	void GetLiveMember(uint8 *arr,uint8 &num);
	void GetAllMember(uint8 *arr,uint8 &num);
	void GetAnotherGroup(uint8 me,uint8 *arr,uint8 &num);
	uint8 GetAnotherGroupNum(uint8 me);
	void GetMeGroup(uint8 me,uint8 *arr,uint8 &num);
	int GetTargetDaoHang(uint8 pos);

	void GetExcept(uint8 except,uint8 *arr,uint8 &num);
	void SortBySpeed(uint8 *arr,uint8 num);

	void SetState(uint8 pos,uint8 state);
	void SetTurn(uint8 pos,uint8 turn);
	void SetOption(uint8 pos,uint8 option,int para,uint8 target);
	void SetProtecter(uint8 pos,uint8 protecter);

	void SetDamageImprove(uint8 pos,int im);
	void SetHpImprove(uint8 pos,int im);
	void SetRecoveryImporve(uint8 pos,int im);//����������
	void SetSpeedImprove(uint8 pos,int im);
	void SetDodgeImprove(uint8 pos,int im);

	void SetDamageTrun(uint8 pos,uint8);
	void SetHpTurn(uint8 pos,uint8);
	void SetRecoveryTurn(uint8 pos,uint8);
	void SetSpeedTurn(uint8 pos,uint8);
	void SetDodgeTurn(uint8 pos,uint8);

	void DecreaseHp(uint8 pos,int hp,uint8 src=0);
	void DecreaseMp(uint8 pos,int mp);
	void ImproveRevovery(uint8 pos,int revovery);

	int CalculateSkillDamage(uint8 src,uint8 target,int skillId);
	uint8 GetState(uint8 pos);
	uint8 GetTurn(uint8 pos);
	void GetOption(uint8 pos,uint8 &option,int &para,uint8 &target);
	uint8 GetTarget(uint8 pos);
	bool IsEmpty(uint8 pos)
	{
		if((pos > 0) && (pos <= MAX_MEMBER))
		{
			return m_members[pos-1].memPtr.empty();
		}
		return true;
	}
	uint8 GetProtecter(uint8 pos)
	{
		if((pos > 0) && (pos <= MAX_MEMBER))
		{
			return m_members[pos-1].protecter;
		}
		return 0;
	}
	int GiveItemByMonster(CUser *pUser,SMonsterInst *pInst);
	int GetDamageImprove(uint8);
	int GetHpImprove(uint8);
	int GetRecoveryImporve(uint8);

	int GetDodgeImprove(uint8);

	uint8 GetDamageTrun(uint8);
	uint8 GetHpTurn(uint8);
	uint8 GetRecoveryTurn(uint8);
	uint8 GetSpeedTurn(uint8);
	uint8 GetDodgeTurn(uint8);

	int CalculateDamage(uint8 src,uint8 target);
	int GetSuccesExp(uint8 pos,int *pMoney = NULL);
	int GetShiYaoExp(uint8 pos);

	bool AllUserOption();
	void CalculateFightResult(list<uint32> &userList);
	uint8 UseSkill21(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target);
	uint8 UseSkill22(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target);
	uint8 UseSkill23(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target);

	uint8 UseSkill21(CNetMessage &msg,SMonsterInst *pMonster,int skillLevel,uint8 src,uint8 target);
	uint8 UseSkill22(CNetMessage &msg,SMonsterInst *pMonster,int skillLevel,uint8 src,uint8 target);
	uint8 UseSkill23(CNetMessage &msg,SMonsterInst *pMonster,int skillLevel,uint8 src,uint8 target);

	uint8 UseSkill24(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target);
	uint8 UseSkill24(CNetMessage &msg,SMonsterInst *pMonster,int skillLevel,uint8 src,uint8 target);

	uint8 UseSkill121(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target);

	void TurnOver(uint8 pos);

	void TestFun();

	template<typename Type> uint8 AddTmpl(Type,uint8 pos);
	template<typename Type> uint8 AddTmplNoLock(Type,uint8 pos);

	uint32  m_id;
	time_t m_beginTime;
	time_t m_turnBegin;

	uint8   m_type;//EFightType
	uint8   m_delNpcX;
	uint8   m_delNpcy;

	int m_timeOut;
	int m_diaoxiangId;

	uint16 m_beginTurnMask;

	uint16 m_visibleMonsterId;//ս����Ӧ�Ŀɼ���id

	const static int  EFFECTIVE_TRUN  = 5;    
	const static int MAX_MEMBER     = 12;
	const static int GROUP2_BEGIN   = MAX_MEMBER/2;
	const static uint8 MISS    = 0;
	const static uint8 HIT_TARGET = 1;

	struct SFightMember
	{
		boost::any memPtr;
		uint8 stateTurn;//״̬�ִ�
		uint8 state;
		int stateAddData1;//״̬�ĸ�������
		int stateAddData2;//״̬�ĸ�������
		bool select;
		uint8 option;
		int para;
		uint8 target;
		uint8 protecter;

		int normalDamageImprove;//����������

		int damageImprove;//�����˺�����
		int hpImprove;
		int recoveryImporve;
		int speedImprove;
		int dodgeImprove;

		uint8 normalDamageTurn;//�����������ִ�
		uint8 damageTrun;
		uint8 hpTurn;
		uint8 recoveryTurn;
		uint8 speedTurn;
		uint8 dodgeTurn;

		uint8 petMask;//����ս����ս��������
		int srcHp;
		int srcMp;
		int srcPkVal;
		int srcDaoHang;
		int srcExp;

		time_t turnBegin;//�ն˷�����һ�ֿ�ʼʱ��

		bool useSkill23;//ʹ�ü���23
	};
	/*
	   ״̬���
	   +----+-----+------+-------+-------+
	   | OP | IND | ���� | ����  | MAXHP | 
	   +----+-----+------+-------+-------+
	   | 1  |  1  |   1  |   1   |   4   |
	   +----+-----+------+-------+-------+
	   OP=5
	   ���� 1=����
	   2=�ж�
	   3=����
	   4=��˯
	   5=ʯ��
	   ���� bit0=��߷�������
	   bit1=��߻ر�
	   bit2=���HP����
	   bit3=����ٶ�
	   bit4=���������

*/
	enum EMemberState
	{
		EMSForget = 1,//����

		EMSEnvenom,//�¶�
		EMSConfusion,//����
		EMSDizzy,//��˯
		EMSFrost,//����

		//154	�������� ��������	�Լ�������ʹ�ú󣬶���ʹ��������ʱ�����ض���������Ч��
		EMSsiwangchanmian,
		//155	Ǭ���� ��������	�Լ�������ʹ�ú󣬱�ʩ�������ܵ�������ʱ�������߶Է�������һ�Σ����Թ��������һ����ֵ�ķ����˺�							
		EMSqiankunzhao,
		//156	����Ȧ ��������	ʹ�ú󣬱�ʩ�ö�����һ���غ����ڵ���һ���˺��ķ�������							
		EMSruyiquan,
		//157	������ ��������	ʹ�ú󣬱�ʩ�ö�����һ���غ����ڵ���һ���˺���������							
		EMSshenlongzhao,

		EMSRecovery,// ���� OPV=0
		EMSProtected,//������

		EMSDied,//����
		EMSEscape,//����
	};
	//bool m_sendPkInfo;
	CScene *m_pScene;
	SFightMember m_members[MAX_MEMBER];
	uint8 m_memNum;
	int m_fightTurn;//ս���ִ�
	int m_jiangliType;//��������
	time_t m_userOpTime;
	bool m_fightIsEnd;
	bool m_useSpeekSkill;
	bool m_useZhaoHuanSkill;
	bool m_canTaoPao;
	list<int> m_guanZhanSock;
	boost::recursive_mutex m_mutex;
};

typedef boost::shared_ptr<CFight> ShareFightPtr;

class CFightManager
{
public:
	CFightManager();
	ShareFightPtr CreateFight();
	void AddFight(ShareFightPtr ptr);

	void UserBattle(CNetMessage*,int);

	//1������һ�δ˺���
	void RunFightTimeOut();
	ShareFightPtr FindFight(uint32 id);
private:
	//bool EachFight(uint32 id,ShareFightPtr pFight);
	CHashTable<uint32,ShareFightPtr> m_fights;
	//list<ShareFightPtr> m_gcFights;

	uint32 m_curFightId;
	COnlineUser &m_onlineUser;
	boost::recursive_mutex m_mutex;
};

inline void CFight::SetOption(uint8 pos,uint8 option,int para,uint8 target)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].option = option;
		m_members[pos-1].para = para;
		m_members[pos-1].target = target;
		m_members[pos-1].select = true;
	}
}

inline void CFight::SetTurn(uint8 pos,uint8 turn)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].stateTurn = turn;
	}
}

inline uint8 CFight::GetTurn(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].stateTurn;
	}
	return 0;
}

inline void CFight::GetOption(uint8 pos,uint8 &option,int &para,uint8 &target)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		option = m_members[pos-1].option;
		para = m_members[pos-1].para;
		target = m_members[pos-1].target;
	}
}
inline void CFight::SetState(uint8 pos,uint8 state)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].state = state;
		m_members[pos-1].stateTurn = 0;
	}
}

inline uint8 CFight::GetState(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].state;
	}
	return 0;
}

inline uint8 CFight::GetTarget(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].target;
	}
	return 0;
}

inline void CFight::SetProtecter(uint8 pos,uint8 protecter)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].protecter = protecter;
	}
}

inline void CFight::SetDamageImprove(uint8 pos,int im)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].damageImprove = im;
	}
}
inline void CFight::SetHpImprove(uint8 pos,int im)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].hpImprove = im;
	}
}
inline void CFight::SetRecoveryImporve(uint8 pos,int im)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].recoveryImporve = im;
	}
}
inline void CFight::SetSpeedImprove(uint8 pos,int im)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].speedImprove = im;
	}
}
inline void CFight::SetDodgeImprove(uint8 pos,int im)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].dodgeImprove = im;
	}
}    
inline void CFight::SetDamageTrun(uint8 pos ,uint8 t)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].damageTrun = t;
	}
}
inline void CFight::SetHpTurn(uint8 pos,uint8 t)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].hpTurn = t;
	}
}
inline void CFight::SetRecoveryTurn(uint8 pos,uint8 t)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].recoveryTurn = t;
	}
}
inline void CFight::SetSpeedTurn(uint8 pos,uint8 t)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].speedTurn = t;
	}
}
inline void CFight::SetDodgeTurn(uint8 pos,uint8 t)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		m_members[pos-1].dodgeTurn = t;
	}
}
inline int CFight::GetDamageImprove(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].damageImprove;
	}
	return 0;
}
inline int CFight::GetHpImprove(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].hpImprove;
	}
	return 0;
}
inline int CFight::GetRecoveryImporve(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].recoveryImporve;
	}
	return 0;
}
inline int CFight::GetSpeedImprove(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].speedImprove;
	}
	return 0;
}
inline int CFight::GetDodgeImprove(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].dodgeImprove;
	}
	return 0;
}

inline uint8 CFight::GetDamageTrun(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].damageTrun;
	}
	return 0;
}
inline uint8 CFight::GetHpTurn(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].hpTurn;
	}
	return 0;
}
inline uint8 CFight::GetRecoveryTurn(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].recoveryTurn;
	}
}
inline uint8 CFight::GetSpeedTurn(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].speedTurn;
	}
	return 0;
}
inline uint8 CFight::GetDodgeTurn(uint8 pos)
{
	if((pos > 0) && (pos <= MAX_MEMBER))
	{
		return m_members[pos-1].dodgeTurn;
	}
}
#endif
