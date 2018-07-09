#pragma once

#include <boost/shared_ptr.hpp>
#include <ctime>
#include <map>
#include "vector3d.hpp"
#include "net_msg.h"
#include "logicdef.h"
#define PI 3.1415f
#define SEC  //second
#define MSEC //millisecond
class CScene;
class CPlayer;
enum EMonsterSeq;
typedef boost::shared_ptr<CScene> scene_ptr;
enum MONSTER_STATE//����״̬
{
	WANDER,//�����ƶ�
	BATTLEMOVE,//ս���ƶ�
	BATTLEDOGE,//��������
	BATTLERUNAWAY,//ս������
	BATTLE,//ս��
};
class monster
{
public:
	monster(void);
	~monster(void);
	void fill_sendclient_pack(CNetMessage &msg);
	void fill_serverdata_pack(CNetMessage& msg);
	void fill_data(CNetMessage &msg);
	void load_drop(CNetMessage &msg);
	void PlayerCollectItem(CNetMessage &msg,int sock);
	void ProcessMsg(CNetMessage &msg,int sock);
	int  monster_id()			 {return insid;}
	bool IsDead()				 {return dead_;}
	void SetDead(bool bDead)	 {dead_ = bDead;}
	CScene* GetScene()			 {return m_pScene;}
	void SetScene(CScene* pScene){ m_pScene = pScene; }
	void BeAttack(CPlayer* player);
	void AI_OnLoop(DWORD dwTime);
private:
	void AI_GoOneStep(float speed,DWORD dwTime);//��һ��
	void AI_WalkOneStep(DWORD dwTime); //�����ƶ�
	void AI_PursuitOneStep(DWORD dwTime);//ս���ƶ�
	void SendMovement(EMonsterSeq movement);//���Ͷ�����Ϣ
	void AI_Attack();//������Ŀ���Ѫ������
	float CalcDistance(vector3d src,vector3d dest);//�����������
	bool IsPlayerInSpyRange(CPlayer* player);//�ж�����Ƿ�����ⷶΧ��
	bool IsTargetInAttRange();//�ж�Ŀ���Ƿ��ڿɹ����ķ�Χ��
	bool IsActivePursuit();//�Ƿ�����׷������
	float GetMoveSpeed(){return m_pMonsterT->m_fMoveSpeed*200;}//����ƶ��ٶ�
	float GetPursuitSpeed(){ return 1.5*GetMoveSpeed(); }//���׷���ٶ�
	float GetAttSpeed(){ return 20/*m_pMonsterT->m_nAttackSpeed*/; }//��ù����ٶ�
	bool CalcAttPoint(vector3d& point);//���㹥����
	void ChangeFaceToPos( const vector3d& pos );//�ı䷽��ʹ��������pos��
	bool CheckCrash();//��ײ��⣬����Ƿ����Ŀ��
	bool CheckAccuracy();//���м�⣬���������
	bool CheckTimeExpired(DWORD dwTime);//��⹥��Ƶ��
private:
	bool dead_;
	//add by fly
	CScene*					m_pScene;//����
	DWORD					m_lasttime;//�ϴ�ʱ��
	DWORD					m_lastHitTime;//�ϴι���һ�µ�ʱ��
	MONSTER_STATE			m_state;//����״̬
	CPlayer*				m_pTarget;//Ŀ��
	Logic::MonsterAtt_t*	m_pMonsterT;//ģ��
	bool					m_bBeAttack;//�Ƿ񱻹���
	CPlayer*				m_pAttacker;//������
public:
	int insid;//ʵ��id
	vector3d originPos;//��������
	int originFace;//��������
	vector3d pos;//����
	int face_;//����
	int	m_nMonsterID;	//������
	//int	m_eMonsterRace;	//��������
	//int	m_eMonsterPhysique;	//��������
	std::string m_szMonsterIcon;	//����ͷ��
	std::string	m_szMonsterTitle;	//����ƺ�
	std::string	m_szMonsterName;	//��������
	std::string	m_szMonsterModel;	//����ģ��
	float	m_fMonsterZoom;	//��������
	//std::string	m_szMonsterArt;	//������ͼ
	//int	m_nMonsterWeapon;	//��������
	//std::string	m_szMonsterEffect;	//������Ч
	int	m_eBossTag;	//BOSS���
	//std::string	m_szTextureName1;	//��Ч��ͼ1
	//std::string	m_szTextureName2;	//��Ч��ͼ2
	//int	m_nMonsterTextureID;	//��Ч����
	//int	m_eGoalAIID;	//Ŀ��ѡ��AI
	int	m_nMonsterLev;	//����ȼ�
	int	m_nPhAttack;	//������
	int	m_nMaAttack;	//��������
	int	m_nPhDefend;	//�������
	int	m_nMaDefend;	//��������
	int	m_nMonsterHP;	//����ֵ
	//int	m_nHPResume;	//�����ظ�
	int	m_nMonsterMP;	//����ֵ
	//int	m_nMPResume;	//�����ظ�
	//int	m_nAttackSpeed;	//�����ٶ�
	//int	m_eAttackMode;	//����״̬
	//int	m_nSpyRange;	//��ⷶΧ
	//int	m_nMoveRange;	//�ƶ���Χ
	//int	m_ePursuitType;	//׷����Χ
	//int	m_nAttackDistance;	//������Χ
	//int	m_nFendResist;	//���˵ֿ�
	//int	m_nImpendenceResist;	//���յֿ�
	//int	m_nSkill1;	//����1
	//int	m_nSkill2;	//����2
	//int	m_nSkill3;	//����3
	//int	m_nSkill4;	//����4
	//int	m_nSkill5;	//����5
	//int	m_nSkill6;	//����6
	//int	m_eMoveType;	//�ƶ�����
	//int	m_nHigh;	//�������߶�
	//int	m_nJumpDistance;	//��Ծ����
	//float	m_fMoveSpeed;	//�ƶ��ٶ�
	//int	m_eIdleMoveOrient;	//�����ƶ�·��
	//int	m_nIdleMoveDistan;	//�����ƶ�һ�ξ���
	//int	m_eborderObject;	//�߽����
	//int	m_eHardObject;	//�����ƻ����
	//int	m_eAverageObject;	//һ���ƻ����
	//int	m_ePlayerObject;	//����ƻ����
	__int64	m_nEXP;	//����ֵ
	int	m_nSPPro;	//SP�㼸��
	int	m_nMoneyPro;	//���Ҽ���
	int	m_nMoney;	//����
	int	m_nEquipPro1;	//���伸��1
	int	m_nEquipItem1;	//������1
	int	m_nEquipPro2;	//���伸��2
	int	m_nEquipItem2;	//������2
	int	m_nEquipPro3;	//���伸��3
	int	m_nEquipItem3;	//������3
	int	m_nEquipPro4;	//���伸��4
	int	m_nEquipItem4;	//������4
	int	m_nEquipPro5;	//���伸��5
	int	m_nEquipItem5;	//������5
	int	m_nSpecialArtPro;	//���⼸��
	int	m_nSpecialArt;	//������Ʒ
	int	m_nScriptID;	//�ű��¼�
	int m_nModelSize;//ģ�ʹ�С
	DWORD m_dwHitIntervalTime;//�������ʱ��
};

typedef boost::shared_ptr<monster> monster_ptr;
typedef map<int,monster_ptr> monster_map;

