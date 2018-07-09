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
	int id;//动态ID
	int type_id;//类型ID
	int num;//数量
	string str;
	int bindid;
	string name;//名称
	int strengthen;//强化属性
	int enchase[8];//镶嵌格子
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
	string title_;  //称号
	int level_;      //等级
	//int bind_;       //是否绑定
	int quality_;   //品质
	int maxExp_; //最大经验
	int curExp_;  //当前经验
	int maxhp_;  //最大红
	int maxmp_; //最大蓝
	int power_;   //白虎之力
	int quick_;    //朱雀之灵
	int intelligence_;//青龙之神
	int energy_;//玄武之精
	int needOccupation;//需要职业
	int neddLevel;//需要等级
	int sellprice;//出售价格
	int weight;//重量
	int endurance;//耐久度
	int phyattack_;//物理攻击
	int maattack_;//法术攻击
	int phydefend_;//物理防御
	int madefend_;//法术防御
	int attackspeed_;//攻击速度
	int hitrate_;//命中率
	int movespeed_;//移动速度
	int evadePro_;//回避率
	int releasePro_;//释放速度
	int MPRevert_;//法力回复
	int HPRevert_;//生命回复
	int CarryWeight_;//负重
	int RigidTime_;//硬直时间
	int resistance_;//抗性
	int skill1_;//技能
	int skill2_;//技能
	int skill3_;//技能
	int state_;//状态
	int db_id;
};
typedef boost::shared_ptr<SowlStone> SowlStone_Ptr;





