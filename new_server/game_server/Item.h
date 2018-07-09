#pragma once
#include "net_msg.h"
#include "logicdef.h"
#include <boost/shared_ptr.hpp>



class SItem
{
typedef boost::shared_ptr<SItem> SItem_Ptr;
public:
	SItem():type_id(0),num(1),id(0){}
	SItem(int tyid,int id_):type_id(tyid),id(id_){}
public:
	static SItem CreateItem(int type_id,int id = 0, int bindid = 0, int num = 1);
	static SItem* CreateItem(SItem* item);
	static SItem CreateItem(SItem& item);
public:
	int id;//��̬ID
	int type_id;//����ID
	int num;//����
	string str;
	int bindid;
	string name;//����
	int strengthen;//ǿ������
	int enchase[8];//��Ƕ����
	Logic::ItemData_t SItemData;
};
typedef boost::shared_ptr<SItem> SItem_Ptr;


class SowlStone: public SItem
{
public:
	SowlStone(){}
	SowlStone(int tyid,int id_):SItem(tyid,id_),db_id(0){}
public:
	void FillSowlStoneData(CNetMessage& msg);
	void FillSowlStoneMsg(CNetMessage& msg);
	static SowlStone* CreateSowlStone(SItem &item);
	void SaveSowlStoneInfoToDB();
public:
	string title_;  //�ƺ�
	int level_;      //�ȼ�
	//int bind_;       //�Ƿ��
	int quality_;   //Ʒ��
	int maxExp_; //�����
	int curExp_;  //��ǰ����
	int maxhp_;  //����
	int maxmp_; //�����
	int power_;   //�׻�֮��
	int quick_;    //��ȸ֮��
	int intelligence_;//����֮��
	int energy_;//����֮��
	int needOccupation;//��Ҫְҵ
	int neddLevel;//��Ҫ�ȼ�
	int sellprice;//���ۼ۸�
	int weight;//����
	int endurance;//�;ö�
	int phyattack_;//������
	int maattack_;//��������
	int phydefend_;//�������
	int madefend_;//��������
	int attackspeed_;//�����ٶ�
	int hitrate_;//������
	int movespeed_;//�ƶ��ٶ�
	int evadePro_;//�ر���
	int releasePro_;//�ͷ��ٶ�
	int MPRevert_;//�����ظ�
	int HPRevert_;//�����ظ�
	int CarryWeight_;//����
	int RigidTime_;//Ӳֱʱ��
	int resistance_;//����
	int skill1_;//����
	int skill2_;//����
	int skill3_;//����
	int state_;//״̬
	int db_id;
};
typedef boost::shared_ptr<SowlStone> SowlStone_Ptr;





