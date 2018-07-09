#include "stdafx.h"
#include "Item.h"
#include "server_data_base.h"
#include "DatabaseSql.h"

SItem SItem::CreateItem(int type_id,int id, int bindid, int num)
{
	SItem item;
	item.type_id = type_id;
	item.num = num;
	item.bindid = bindid;
	/*item.endurance = server_data::instance().UseItemData_map[type_id].m_nEndurance;
	item.maattack_ = server_data::instance().UseItemData_map[type_id].m_nMaAttack;
	item.madefend_ = server_data::instance().UseItemData_map[type_id].m_nMaDefend;
	item.neddLevel = server_data::instance().UseItemData_map[type_id].m_nLevel;
	item.needOccupation = server_data::instance().UseItemData_map[type_id].m_eOccupation;
	item.phyattack_ = server_data::instance().UseItemData_map[type_id].m_nPhyAttack;
	item.phydefend_ = server_data::instance().UseItemData_map[type_id].m_nPhDefend;
	item.sellprice = server_data::instance().UseItemData_map[type_id].m_nBuyPric;
	item.weight = server_data::instance().UseItemData_map[type_id].m_fWeight;*/
	item.name = server_data::instance().ItemEData_map[type_id].m_szName;

	
	if (id == 0)
	{
		item.id = server_data::instance().CreateID();
		//item.SItemData = {0};
	}
	else
	{
		item.id = id;
		if (server_data::instance().UseItemData_map.find(type_id) != server_data::instance().UseItemData_map.end())
			item.SItemData = server_data::instance().UseItemData_map[type_id];
	}
	

	return item;
}

SItem* SItem::CreateItem(SItem* item)
{
	item->id = server_data::instance().CreateID();
	return item;
}

SItem SItem::CreateItem(SItem& item)
{
	item.id = server_data::instance().CreateID();
	return item;
}

void SowlStone::FillSowlStoneData(CNetMessage &msg)
{
	msg>>type_id>>title_>>level_>>bindid>>quality_>>maxExp_>>curExp_>>maxhp_>>maxmp_>>power_>>quick_
		>>intelligence_>>energy_>>phyattack_>>maattack_>>phydefend_>>madefend_>>attackspeed_
		>>hitrate_>>movespeed_>>evadePro_>>releasePro_>>MPRevert_>>HPRevert_>>CarryWeight_>>RigidTime_
		>>resistance_>>skill1_>>skill2_>>skill3_>>state_;
}

void SowlStone::FillSowlStoneMsg(CNetMessage &msg)
{
	msg<<type_id<<title_<<level_<<bindid<<quality_<<maxExp_<<curExp_<<maxhp_<<maxmp_<<power_<<quick_
		<<intelligence_<<energy_<<phyattack_<<maattack_<<phydefend_<<madefend_<<attackspeed_
		<<hitrate_<<movespeed_<<evadePro_<<releasePro_<<MPRevert_<<HPRevert_<<CarryWeight_<<RigidTime_
		<<resistance_<<skill1_<<skill2_<<skill3_<<state_;
}


SowlStone* SowlStone::CreateSowlStone(SItem &item)
{
	SowlStone* sl = new SowlStone(item.type_id,item.id);
	//sl->id = item.id;
	//sl->type_id = item.type_id;
	sl->level_ = 1;
	sl->quality_ = 1;   //品质
	sl->maxExp_ = 100; //最大经验
	sl->curExp_ = 0;  //当前经验
	sl->maxhp_ = 100;  //最大红
	sl->maxmp_ = 100; //最大蓝
	sl->power_ = 1;   //白虎之力
	sl->quick_ = 1;    //朱雀之灵
	sl->intelligence_ = 1;//青龙之神
	sl->energy_ = 1;//玄武之精
	sl->phyattack_ = 1;//物理攻击
	sl->maattack_ = 1;//法术攻击
	sl->phydefend_ = 1;//物理防御
	sl->madefend_ = 1;//法术防御
	sl->attackspeed_ =1;//攻击速度
	sl->hitrate_ = 1;//命中率
	sl->movespeed_ = 1;//移动速度
	sl->evadePro_ = 1;//回避率
	sl->releasePro_ = 1;//释放速度
	sl->MPRevert_ = 1;//法力回复
	sl->HPRevert_ = 1;//生命回复
	sl->CarryWeight_ =1;//负重
	sl->RigidTime_ = 1;//硬直时间
	sl->resistance_ =1;//抗性
	sl->skill1_ = 0;//技能
	sl->skill2_ = 0;//技能
	sl->skill3_ = 0;//技能
	sl->state_ = 0;

	return sl;
}

void SowlStone::SaveSowlStoneInfoToDB()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return;

	char sql_buf[500];
	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf),"update soulstone_infos set type_id=%d,title='%s',level=%d,"\
									  "bind=%d,quality=%d,maxExp=%d,curExp=%d,maxhp=%d,maxmp=%d,"\
									  "power=%d,quick=%d,intelligence=%d,energy=%d,phyattack=%d,"\
									 "maattack=%d,phydefend=%d,madefend=%d,attackspeed=%d,hitrate=%d,"\
									  "movespeed=%d,evadePro=%d,releasePro=%d,MPRevert=%d,HPRevert=%d,"\
									  "CarryWeight=%d,RigidTime=%d,resistance=%d,skill1=%d,skill2=%d,"\
									  "skill3=%d,state=%d where soulstoneid=%d",
									 type_id,"qq",level_,bindid,quality_,maxExp_,curExp_,maxhp_,maxmp_,power_,quick_,
		intelligence_,energy_,phyattack_,maattack_,phydefend_,madefend_,attackspeed_,hitrate_,movespeed_,
		evadePro_,releasePro_,MPRevert_,HPRevert_,CarryWeight_,RigidTime_,resistance_,skill1_,skill2_,skill3_,
		state_,db_id);

	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}

}