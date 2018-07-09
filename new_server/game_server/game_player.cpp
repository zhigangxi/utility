#include "stdafx.h"
#include "game_map.h"
#include "game_player.h"
#include "DatabaseSql.h"
#include "despatch_command.h"
#include <boost/bind.hpp>
#include "protocol.h"
#include "scene_manager.h"
#include "game_socket.h"
#include "server_data_base.h"
#include <boost/lexical_cast.hpp>
#include "global_manager.h"
#include "game_npc.h"
#include <algorithm>
#include <vector>
#include "logicdef.h"

void CPlayer::PlayerEnterGame(CNetMessage &msg)
{
	CGameSocket &sockServer = CGameSocket::instance();
	int respond = 0;
	msg>>roleId;
	msg.ReWrite();
	msg.SetType(GC_ENTER_GAME);
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return;
	string sqlStr = "select rolename,nickname,experence,requireEXP,race,occupation,level,power,quick,intelligence,energy,\
					maxhp,maxmp,phyattack,maattack,phydefend,madefend,attackspeed,hitrate,movespeed,evadePro,releasePro,MPRevert,HPRevert,\
					CarryWeight,RigidTime,resistance,FrozenRessis,BurnRessis,PalsyRessis,ToxinRessis,VertigoRessis,skillRessis,MARessis,\
					weakRessis,reputation,AttriPoint,SkillPoint,AttackStorm,lucky,money,gold,hairColor,glass,neckchain,cape,belt,mask,\
					wig,headwear,shoulderpad,armor,armguard,legguard,shoes,roleplace,head,weapon,skill from game_user_role_info where roleid = " 
					+ boost::lexical_cast<std::string,int>(roleId);
	char **row;
	if (!pDb->Query(sqlStr.c_str()))
	{
		cout<<sqlStr<<endl;
		return;
	}
	row = pDb->GetRow();
	if(row == NULL)
		return;
	msg<<respond<<(string)row[0]<<(string)row[1]<<(__int64)atoi(row[2])<<(__int64)atoi(row[3])<<atoi(row[4])<<atoi(row[5])
		<<atoi(row[6])<<atoi(row[7])<<atoi(row[8])<<atoi(row[9])<<atoi(row[10])<<atoi(row[11])<<atoi(row[12])<<atoi(row[13])
		<<atoi(row[14])<<atoi(row[15])<<atoi(row[16])<<atoi(row[17])<<atoi(row[18])<<atoi(row[19])<<atoi(row[20])<<atoi(row[21])
		<<atoi(row[22])<<atoi(row[23])<<atoi(row[24])<<atoi(row[25])<<atoi(row[26])<<atoi(row[27])<<atoi(row[28])<<atoi(row[29])
		<<atoi(row[30])<<atoi(row[31])<<atoi(row[32])<<atoi(row[33])<<atoi(row[34])<<atoi(row[35])<<atoi(row[36])<<atoi(row[37])
		<<atoi(row[38])<<atoi(row[39])<<atoi(row[40])<<atoi(row[41])<<atoi(row[42])<<atoi(row[43])<<atoi(row[44])<<atoi(row[45])
		<<atoi(row[46])<<atoi(row[47])<<atoi(row[48])<<atoi(row[49])<<atoi(row[50])<<atoi(row[51])<<atoi(row[52])<<atoi(row[53])
		<<atoi(row[54])<<atoi(row[55])<<atoi(row[56])<<atoi(row[57]);

	scene_ptr scene = scene_manager::instance().create_scene(1);
	scene->PlayerEnterScene(this);
	CGameSocket::instance().SendMsg(sock,msg);
	
	name_ = (string)row[0];
	nick_ = (string)row[1];
	experence = (__int64)atoi(row[2]);
	next_exp = (__int64)atoi(row[3]);
	race_ = atoi(row[4]);
	occupation_ = atoi(row[5]);
	level_ = atoi(row[6]);
	power_ = atoi(row[7]);
	quick_ = atoi(row[8]);
	intelligence_ = atoi(row[9]);
	energy_ = atoi(row[10]);
	maxhp_ = atoi(row[11]);
	maxmp_ = atoi(row[12]);
	phyattack_ = atoi(row[13]);
	maattack_ = atoi(row[14]);
	phydefend_ = atoi(row[15]);
	madefend_ = atoi(row[16]);
	attackspeed_ = atoi(row[17]);
	hitrate_ = atoi(row[18]);
	movespeed_ = atoi(row[19]);
	evadePro_ = atoi(row[20]);
	releasePro_ = atoi(row[21]);
	MPRevert_ = atoi(row[22]);
	HPRevert_ = atoi(row[23]);
	CarryWeight_ = atoi(row[24]);
	RigidTime_ = atoi(row[25]);
	resistance_ = atoi(row[26]);
	FrozenRessis_ = atoi(row[27]);
	BurnRessis_ = atoi(row[28]);
	PalsyRessis_ = atoi(row[29]);
	ToxinRessis_ = atoi(row[30]);
	VertigoRessis_ = atoi(row[31]);
	skillRessis_ = atoi(row[32]);
	MARessis_ = atoi(row[33]);
	weakRessis_ = atoi(row[34]);
	reputation_ = atoi(row[35]);
	AttriPoint_ = atoi(row[36]);
	SkillPoint_ = atoi(row[37]);
	AttackStorm_ = atoi(row[38]);
	lucky_ = atoi(row[39]);
	money_ = atoi(row[40]);
	gold_ = atoi(row[41]);
	hairColor = atoi(row[42]);
	glass = atoi(row[43]);
	neckchain = atoi(row[44]);
	cape = atoi(row[45]);
	belt = atoi(row[46]);
	mask = atoi(row[47]);
	wig = atoi(row[48]);
	headwear = atoi(row[49]);
	shoulderpad = atoi(row[50]);
	armor = atoi(row[51]);
	armguard = atoi(row[52]);
	legguard = atoi(row[53]);
	shoes = atoi(row[54]);
	roleplace = atoi(row[55]);
	head = atoi(row[56]);
	weapon = atoi(row[57]);
    m_skill = row[58];

	hp_ = maxhp_;
	mp_ = maxmp_;

	string sqlStr1 = "select equippackid,expendpackid,materialpackid,taskpackid,decorpackid from \
					 backpack_base_info where roleid = " + boost::lexical_cast<std::string,int>(roleId);

	if (!pDb->Query(sqlStr1.c_str()))
	{
		cout<<sqlStr1<<endl;
		return;
	}

	CNetMessage msg1;
	CNetMessage msg2;
	CNetMessage msg3;
	CNetMessage msg4;

	row = pDb->GetRow();
	if(row == NULL)
		return;
	equippack_id = atoi(row[0]);
	expendpack_id = atoi(row[1]);
	materialpack_id = atoi(row[2]);
	taskpack_id = atoi(row[3]);
	decorpack_id = atoi(row[4]);
	

	msg.ReWrite();
	msg1.SetType(GC_BACKPACK_BASE_INFO);
	msg1<<atoi(row[0])<<atoi(row[1])<<atoi(row[2])<<atoi(row[3])<<atoi(row[4])<<0/*负重*/;
	CGameSocket::instance().SendMsg(sock,(msg1));
	cout<<"GC_BACKPACK_BASE_INFO"<<endl;
	

	char sql_buf[200];
	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf), "select * from backpack_item_info where packid =%d \
		or packid=%d or packid=%d or packid=%d or packid=%d" ,equippack_id,expendpack_id,materialpack_id,
		taskpack_id,decorpack_id);
		

	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}

	
	while ((row = pDb->GetRow()) != NULL)
	{
		for (UINT i = 0;i<53;i++)
		{
			if (i==0)
			{
				msg3<<atoi(row[i]);
			}
			else
			{
				msg3<<(string)row[i];
			}	
		}
	}

	msg2.SetType(GC_BACKPACK_ITEM_INFO);

	UpdateBackpackData(msg3,msg2);
	CGameSocket::instance().SendMsg(sock,(msg2));
	cout<<"GC_BACKPACK_ITEM_INFO"<<endl;

	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf), "select shortcut1,shortcut2,shortcut3,shortcut4,\
		shortcut5 from role_shortcut where roleid = %d" ,roleId);

	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}
	
	CNetMessage msg5;
	msg5.SetType(GC_SET_EXPEND_SHORTCUT);

	
	if ((row = pDb->GetRow()) != NULL)
	{
		for (int i =0;i<5;i++)
		{
			expend_shortcut[i] = atoi(row[i]);
			msg5<<expend_shortcut[i];
		}
	}

	//CGameSocket::instance().SendMsg(sock,msg5);
	sockServer.SendMsg(sock,msg5);
	cout<<"GC_SET_EXPEND_SHORTCUT"<<endl;


	vector3d pos;
	pos.x_ = 1840;
	pos.z_ = 1829;
	msg4.SetType(GC_ROLE_JUMPSENCE);
	string scenename = "家";
	string mapname = "home001";
	(msg4)<<scenename<<mapname<<scene->scene_id_<<(int)0<<pos;
	CGameSocket::instance().SendMsg(sock,(msg4));
	cout<<"GC_ROLE_JUMPSENCE"<<endl;
	
	if( LoadTaskFromDB() )//加载玩家任务
		SendTaskList();//发送任务列表

    CNpcManager *pNcpMgr;
    CGlobalValManager::CreateInstance()->GetVal(&pNcpMgr,EGV_NPC_MANAGER);
    pNcpMgr->SendNpcList(1,sock);
	//CountPlayerAttr();
	GetSolwStoneInfoFromDB();
	

}

void CPlayer::SendAndUpdataSowlStoneInfo()
{
	CNetMessage msg;
	msg.SetType(GC_SOWL_PACKINFO);
	
	for (int i = 0;i<21;i++)
	{	
		msg<<sowlstone_data[i]->id;
		sowlstone_data[i]->FillSowlStoneMsg(msg);
	}
	
	CGameSocket::instance().SendMsg(sock,msg);
	
	
}

void CPlayer::SaveSowlStoneBagToDB()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return;

	char sql_buf[500];

	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf),"update role_soulstone_bag set soulstone01=%d,soulstone02=%d,\
			 soulstone03=%d,soulstone04=%d,soulstone05=%d,soulstone06=%d,soulstone07=%d,soulstone08=%d,soulstone09=%d,\
			soulstone10=%d,soulstone11=%d,soulstone12=%d,soulstone13=%d,soulstone14=%d,soulstone15=%d,soulstone16=%d,\
			soulstone17=%d,soulstone18=%d,soulstone19=%d,soulstone20=%d,soulstone21=%d where roleid =%d",
			sowlstone_data[0]->db_id,sowlstone_data[1]->db_id,sowlstone_data[2]->db_id,sowlstone_data[3]->db_id,sowlstone_data[4]->db_id,
			sowlstone_data[5]->db_id,sowlstone_data[6]->db_id,sowlstone_data[7]->db_id,sowlstone_data[8]->db_id,sowlstone_data[9]->db_id,sowlstone_data[10]->db_id,
			sowlstone_data[11]->db_id,sowlstone_data[12]->db_id,sowlstone_data[13]->db_id,sowlstone_data[14]->db_id,sowlstone_data[15]->db_id,sowlstone_data[16]->db_id,
			sowlstone_data[17]->db_id,sowlstone_data[18]->db_id,sowlstone_data[19]->db_id,sowlstone_data[20]->db_id,roleId);

	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}

}

void CPlayer::GetSolwStoneInfoFromDB()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	CDatabaseSql *pDb1 = getDb.GetDbConnect();
	char** row;
	SItem item;
	if (pDb == NULL)
		return;

	char sql_buf[800];
	memset(sql_buf,0,sizeof(sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf),"select soulstone01,soulstone02,soulstone03,soulstone04,soulstone05,soulstone06,\
									  soulstone07,soulstone08,soulstone09,soulstone10,soulstone11,soulstone12,soulstone13,\
									  soulstone14,soulstone15,soulstone16,soulstone17,soulstone18,soulstone19,soulstone20,\
									  soulstone21 from role_soulstone_bag where roleid =%d",roleId);

	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}


	if ((row = pDb->GetRow()) != NULL)
	{
		for (int i = 0;i<21;i++)
		{
			item = SItem::CreateItem(0);
			SowlStone* sl = SowlStone::CreateSowlStone(item);
			sl->db_id = atoi(row[i]);
			sowlstone_data[i] = sl;
			//sowlstone_data[i]->db_id = atoi(row[i]);
		}

	}
	if (pDb1 == NULL)
		return;
	
	CNetMessage msg;

	for (int i =0;i<21;i++)
	{
		memset(sql_buf,0,sizeof(sql_buf));
		_snprintf(sql_buf,sizeof(sql_buf),"select type_id,title,level,bind,quality,maxExp,curExp,maxhp,maxmp,power,quick,intelligence,\
										  energy,phyattack,maattack,phydefend,madefend,attackspeed,hitrate,movespeed,evadePro,releasePro,\
										  MPRevert,HPRevert,CarryWeight,RigidTime,resistance,skill1,skill2,skill3,state\
										  from soulstone_infos where soulstoneid = %d",sowlstone_data[i]->db_id);
		if (!pDb->Query(sql_buf))
		{
			cout<<sql_buf<<endl;
			return;
		}
		if ((row = pDb->GetRow()) != NULL)
		{
			for (UINT i = 0;i<31;i++)
			{
				if (i==1)
				{
					msg<<(string)row[i];
				}
				else
				{
					msg<<atoi(row[i]);
				}

			}
		}
	}
	
	
	
	CNetMessage msg1;
	msg1.SetType(GC_SOWL_PACKINFO);
	
	for (int i = 0;i<21;i++)
	{
		sowlstone_data[i]->FillSowlStoneData(msg);
		msg1<<sowlstone_data[i]->id;
		sowlstone_data[i]->FillSowlStoneMsg(msg1);
	}

	CGameSocket::instance().SendMsg(sock,msg1);
	cout<<"GC_SOWL_PACKINFO"<<endl;
	
}

void CPlayer::UpdatePlayerPoint(CNetMessage &msg)
{
	msg>>maxhp_>>maxmp_>>AttriPoint_>>power_>>quick_>>intelligence_>>energy_>>phyattack_>>maattack_
		>>phydefend_>>madefend_>>hitrate_>>evadePro_>>AttackStorm_>>movespeed_>>attackspeed_>>releasePro_				
		>>FrozenRessis_>>BurnRessis_>>PalsyRessis_>>ToxinRessis_>>VertigoRessis_>>skillRessis_>>MARessis_
		>>weakRessis_;
}

void CPlayer::PacketPlayerInfo(CNetMessage &msg)
{
	msg<<nick_<<experence<<next_exp<<race_<<occupation_<<level_<<power_<<quick_<<intelligence_<<energy_
	<<maxhp_<<maxmp_<<phyattack_<<maattack_<<phydefend_<<madefend_<<attackspeed_<<hitrate_<<movespeed_
	<<evadePro_<<releasePro_<<MPRevert_<<HPRevert_<<CarryWeight_<<RigidTime_<<resistance_<<FrozenRessis_
	<<BurnRessis_<<PalsyRessis_<<ToxinRessis_<<VertigoRessis_<<skillRessis_<<MARessis_<<weakRessis_
	<<reputation_<<AttriPoint_<<SkillPoint_<<AttackStorm_<<lucky_<<money_<<gold_<<0<<hairColor<<glass
	<<neckchain<<cape<<belt<<mask<<wig<<headwear<<shoulderpad<<armor<<armguard<<legguard<<shoes<<roleplace
	<<head<<weapon;

}

void CPlayer::PacketPlayerNotifyInfo(CNetMessage& msg)
{
	msg<<roleId<<name_<<nick_<<race_<<occupation_<<level_<<action_<<pos_.x_<<pos_.y_<<pos_.z_
		<<dir_<<maxhp_<<maxmp_<<if_change<<hairColor<<glass
		<<neckchain<<cape<<belt<<mask<<wig<<headwear<<shoulderpad<<armor<<armguard<<legguard<<shoes<<roleplace
		<<head<<weapon;

}

void CPlayer::UpdatePlayerInfo(CNetMessage &msg)
{
	
}

void CPlayer::SendAllBackPackData()
{
	CNetMessage msg;	
	msg.SetType(GC_BACKPACK_ITEM_INFO);
	msg<<equippack_id;
	for (int i = 0;i<52;i++)
	{
		msg<<equip_data[i].type_id<<equip_data[i].id<<equip_data[i].num;
	}



	msg<<expendpack_id;
	for (int i = 0;i<52;i++)
	{
		msg<<expend_data[i].type_id<<expend_data[i].id<<expend_data[i].num;
	}


	msg<<materialpack_id;
	for (int i = 0;i<52;i++)
	{
		msg<<material_data[i].type_id<<material_data[i].id<<material_data[i].num;
	}


	msg<<taskpack_id;
	for (int i = 0;i<52;i++)
	{
		msg<<task_data[i].type_id<<task_data[i].id<<task_data[i].num;
	}



	msg<<decorpack_id;
	for (int i = 0;i<52;i++)
	{
		msg<<decor_data[i].type_id<<decor_data[i].id<<decor_data[i].num;
	}

	CGameSocket::instance().SendMsg(sock,msg);

}

void CPlayer::SavingPlayerInfo()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return;
	char sql_buf[1000];
	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf),
	"call update_user_infos(%I64d,%I64d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\
	%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\'%s\',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
	experence,next_exp,race_,occupation_,level_,power_,quick_,intelligence_,energy_,maxhp_,maxmp_,phyattack_,maattack_,phydefend_,
	madefend_,attackspeed_,
	hitrate_,movespeed_,evadePro_,releasePro_,MPRevert_,HPRevert_,CarryWeight_,RigidTime_,resistance_,FrozenRessis_,
	BurnRessis_,PalsyRessis_,ToxinRessis_,
	VertigoRessis_,skillRessis_,MARessis_,weakRessis_,reputation_,AttriPoint_,SkillPoint_,AttackStorm_,lucky_,nick_.c_str(),money_,gold_,0,
	roleId,hairColor,glass,neckchain,cape,belt,mask,wig,headwear,shoulderpad,armor,armguard,legguard,shoes,roleplace,head,weapon);
	
	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}
	SaveTaskToDB();//保存任务

	CGetDbConnect getDb1;
	CDatabaseSql *pDb1 = getDb.GetDbConnect();

	char buf[200];
	memset(buf,0,sizeof(buf));
	_snprintf(buf,sizeof(buf), "update role_shortcut set shortcut1 = %d,shortcut2 = %d, shortcut3 = %d,shortcut4 = %d,\
							   shortcut5 = %d where roleid = %d",expend_shortcut[0],expend_shortcut[1],expend_shortcut[2],
							   expend_shortcut[3],expend_shortcut[4],roleId);
	if (!pDb->Query(buf))
	{
		cout<<buf<<endl;
		return;
	}

}

void CPlayer::UpdateBackpackData(CNetMessage& msg,CNetMessage& msg1)
{
	string str;
	SItem item;
	msg>>equippack_id;
	msg1<<equippack_id;
	for (int i=0;i<52;i++)
	{
		msg>>str;
		AnalyzeData(str,item);
		item = SItem::CreateItem(item.type_id);
		equip_data[i] = item;
		CombinationData(equip_data[i]);
		msg1<<item.type_id<<item.id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
			<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
			<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
			<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

		for (int i =0;i<8;i++)
		{
			msg1<<item.enchase[i];
		}
		//msg1<<0/*是否强化*/<<item.strengthen/*强化属性*/;;

	}
	msg>>expendpack_id;
	msg1<<expendpack_id;
	for (int i=0;i<52;i++)
	{
		msg>>str;
		AnalyzeData(str,item);
		item = SItem::CreateItem(item.type_id);
		expend_data[i] = item;
		CombinationData(expend_data[i]);
		msg1<<item.type_id<<item.id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
			<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
			<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
			<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

		for (int i =0;i<8;i++)
		{
			msg1<<item.enchase[i];
		}
		//msg1<<0/*是否强化*/<<item.strengthen/*强化属性*/;;
	}
	msg>>materialpack_id;
	msg1<<materialpack_id;
	for (int i=0;i<52;i++)
	{
		msg>>str;
		AnalyzeData(str,item);
		item = SItem::CreateItem(item.type_id);
		material_data[i] = item;
		CombinationData(material_data[i]);
		msg1<<item.type_id<<item.id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
			<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
			<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
			<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

		for (int i =0;i<8;i++)
		{
			msg1<<item.enchase[i];
		}
		//msg1<<0/*是否强化*/<<item.strengthen/*强化属性*/;;
	}
	msg>>taskpack_id;
	msg1<<taskpack_id;
	for (int i=0;i<52;i++)
	{
		msg>>str;
		AnalyzeData(str,item);
		item = SItem::CreateItem(item.type_id);
		task_data[i] = item;
		CombinationData(task_data[i]);
		msg1<<item.type_id<<item.id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
			<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
			<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
			<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

		for (int i =0;i<8;i++)
		{
			msg1<<item.enchase[i];
		}
		//msg1<<0/*是否强化*/<<item.strengthen/*强化属性*/;;
	}
	msg>>decorpack_id;
	msg1<<decorpack_id;
	for (int i=0;i<44;i++)
	{
		msg>>str;
		AnalyzeData(str,item);
		item = SItem::CreateItem(item.type_id);
		decor_data[i] = item;
		CombinationData(decor_data[i]);
		msg1<<item.type_id<<item.id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
			<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
			<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
			<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

		for (int i =0;i<8;i++)
		{
			msg1<<item.enchase[i];
		}
		//msg1<<0/*是否强化*/<<item.strengthen/*强化属性*/;;
	}
}

void CPlayer::SavingBackpackData()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return;
	char sql_buf[2000];
	memset(sql_buf,0,sizeof (sql_buf));
	_snprintf(sql_buf,sizeof(sql_buf),"call update_role_backpack(%d,%d,%d,%d,%d,\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
	  \'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
	  \'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
	  \'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
	  \'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\
	  \'%s\',\'%s\',\'%s\',\'%s\',\'%s\')",
	  equippack_id,expendpack_id,materialpack_id,taskpack_id,decorpack_id,equip_data[0].str.c_str(),equip_data[1].str.c_str(),equip_data[2].str.c_str(),equip_data[3].str.c_str(),equip_data[4].str.c_str(),equip_data[5].str.c_str(),equip_data[6].str.c_str(),equip_data[7].str.c_str(),equip_data[8].str.c_str(),equip_data[9].str.c_str(),
	  equip_data[10].str.c_str(),equip_data[11].str.c_str(),equip_data[12].str.c_str(),equip_data[13].str.c_str(),equip_data[14].str.c_str(),equip_data[15].str.c_str(),equip_data[16].str.c_str(),equip_data[17].str.c_str(),equip_data[18].str.c_str(),equip_data[19].str.c_str(),equip_data[20].str.c_str(),equip_data[21].str.c_str(),equip_data[22].str.c_str(),equip_data[23].str.c_str(),equip_data[24].str.c_str(),equip_data[25].str.c_str(),equip_data[26].str.c_str(),equip_data[27].str.c_str(),equip_data[28].str.c_str(),equip_data[29].str.c_str(),
	  equip_data[30].str.c_str(),equip_data[31].str.c_str(),equip_data[32].str.c_str(),equip_data[33].str.c_str(),equip_data[34].str.c_str(),equip_data[35].str.c_str(),equip_data[36].str.c_str(),equip_data[37].str.c_str(),equip_data[38].str.c_str(),equip_data[39].str.c_str(),equip_data[40].str.c_str(),equip_data[41].str.c_str(),equip_data[42].str.c_str(),equip_data[43].str.c_str(),equip_data[44].str.c_str(),equip_data[45].str.c_str(),equip_data[46].str.c_str(),equip_data[47].str.c_str(),equip_data[48].str.c_str(),equip_data[49].str.c_str(),
	  equip_data[50].str.c_str(),equip_data[51].str.c_str(),expend_data[0].str.c_str(),expend_data[1].str.c_str(),expend_data[2].str.c_str(),expend_data[3].str.c_str(),expend_data[4].str.c_str(),expend_data[5].str.c_str(),expend_data[6].str.c_str(),expend_data[7].str.c_str(),expend_data[8].str.c_str(),expend_data[9].str.c_str(),
	  expend_data[10].str.c_str(),expend_data[11].str.c_str(),expend_data[12].str.c_str(),expend_data[13].str.c_str(),expend_data[14].str.c_str(),expend_data[15].str.c_str(),expend_data[16].str.c_str(),expend_data[17].str.c_str(),expend_data[18].str.c_str(),expend_data[19].str.c_str(),expend_data[20].str.c_str(),expend_data[21].str.c_str(),expend_data[22].str.c_str(),expend_data[23].str.c_str(),expend_data[24].str.c_str(),expend_data[25].str.c_str(),expend_data[26].str.c_str(),expend_data[27].str.c_str(),expend_data[28].str.c_str(),expend_data[29].str.c_str(),
	  expend_data[30].str.c_str(),expend_data[31].str.c_str(),expend_data[32].str.c_str(),expend_data[33].str.c_str(),expend_data[34].str.c_str(),expend_data[35].str.c_str(),expend_data[36].str.c_str(),expend_data[37].str.c_str(),expend_data[38].str.c_str(),expend_data[39].str.c_str(),material_data[0].str.c_str(),material_data[1].str.c_str(),material_data[2].str.c_str(),material_data[3].str.c_str(),material_data[4].str.c_str(),material_data[5].str.c_str(),material_data[6].str.c_str(),material_data[7].str.c_str(),material_data[8].str.c_str(),material_data[9].str.c_str(),
	  material_data[10].str.c_str(),material_data[11].str.c_str(),material_data[12].str.c_str(),material_data[13].str.c_str(),material_data[14].str.c_str(),material_data[15].str.c_str(),material_data[16].str.c_str(),material_data[17].str.c_str(),material_data[18].str.c_str(),material_data[19].str.c_str(),material_data[20].str.c_str(),material_data[21].str.c_str(),material_data[22].str.c_str(),material_data[23].str.c_str(),material_data[24].str.c_str(),material_data[25].str.c_str(),material_data[26].str.c_str(),material_data[27].str.c_str(),material_data[28].str.c_str(),material_data[29].str.c_str(),
	  material_data[30].str.c_str(),material_data[31].str.c_str(),material_data[32].str.c_str(),material_data[33].str.c_str(),material_data[34].str.c_str(),material_data[35].str.c_str(),material_data[36].str.c_str(),material_data[37].str.c_str(),material_data[38].str.c_str(),material_data[39].str.c_str(),task_data[0].str.c_str(),task_data[1].str.c_str(),task_data[2].str.c_str(),task_data[3].str.c_str(),task_data[4].str.c_str(),task_data[5].str.c_str(),task_data[6].str.c_str(),task_data[7].str.c_str(),task_data[8].str.c_str(),task_data[9].str.c_str(),
	  task_data[10].str.c_str(),task_data[11].str.c_str(),task_data[12].str.c_str(),task_data[13].str.c_str(),task_data[14].str.c_str(),task_data[15].str.c_str(),task_data[16].str.c_str(),task_data[17].str.c_str(),task_data[18].str.c_str(),task_data[19].str.c_str(),task_data[20].str.c_str(),task_data[21].str.c_str(),task_data[22].str.c_str(),task_data[23].str.c_str(),task_data[24].str.c_str(),task_data[25].str.c_str(),task_data[26].str.c_str(),task_data[27].str.c_str(),task_data[28].str.c_str(),task_data[29].str.c_str(),
	  task_data[30].str.c_str(),task_data[31].str.c_str(),task_data[32].str.c_str(),task_data[33].str.c_str(),task_data[34].str.c_str(),task_data[35].str.c_str(),task_data[36].str.c_str(),task_data[37].str.c_str(),task_data[38].str.c_str(),task_data[39].str.c_str(),decor_data[0].str.c_str(),decor_data[1].str.c_str(),decor_data[2].str.c_str(),decor_data[3].str.c_str(),decor_data[4].str.c_str(),decor_data[5].str.c_str(),decor_data[6].str.c_str(),decor_data[7].str.c_str(),decor_data[8].str.c_str(),decor_data[9].str.c_str(),
	  decor_data[10].str.c_str(),decor_data[11].str.c_str(),decor_data[12].str.c_str(),decor_data[13].str.c_str(),decor_data[14].str.c_str(),decor_data[15].str.c_str(),decor_data[16].str.c_str(),decor_data[17].str.c_str(),decor_data[18].str.c_str(),decor_data[19].str.c_str(),decor_data[20].str.c_str(),decor_data[21].str.c_str(),decor_data[22].str.c_str(),decor_data[23].str.c_str(),decor_data[24].str.c_str(),decor_data[25].str.c_str(),decor_data[26].str.c_str(),decor_data[27].str.c_str(),decor_data[28].str.c_str(),decor_data[29].str.c_str(),
	  decor_data[30].str.c_str(),decor_data[31].str.c_str(),decor_data[32].str.c_str(),decor_data[33].str.c_str(),decor_data[34].str.c_str(),decor_data[35].str.c_str(),decor_data[36].str.c_str(),decor_data[37].str.c_str(),decor_data[38].str.c_str(),decor_data[39].str.c_str(),decor_data[40].str.c_str(),decor_data[41].str.c_str(),
	  decor_data[42].str.c_str(),decor_data[43].str.c_str());
	char **row;
	if (!pDb->Query(sql_buf))
	{
		cout<<sql_buf<<endl;
		return;
	}
	row = pDb->GetRow();
	if(row == NULL)
		return;
}

void CPlayer::FectchBackpackData(CNetMessage& msg)
{
	/*msg<<equippack_id;
	for (int i=0;i<52;i++)
	{
		msg<<equip_data[i];
	}
	msg<<expendpack_id;
	for (int i=0;i<40;i++)
	{
		msg<<expend_data[i];
	}
	msg<<materialpack_id;
	for (int i=0;i<40;i++)
	{
		msg<<material_data[i];
	}
	msg<<taskpack_id;
	for (int i=0;i<40;i++)
	{
		msg<<task_data[i];
	}
	msg<<decorpack_id;
	for (int i=0;i<44;i++)
	{
		msg<<decor_data[i];
	}*/
}

void CPlayer::PlayerChat(CNetMessage& msg)
{
	chatContent.clear();
	msg>>chatContent>>channel;
	COnlineUserList::instance().ForEach(boost::bind(&CPlayer::ChatBroadcast,this,_1));
}

void  CPlayer::ChatBroadcast(CPlayer& player)
{
	CNetMessage msg;
	msg.SetType(GC_CHAT_INFO);
	(msg)<<roleId<<name_<<chatContent<<channel;
	CGameSocket::instance().SendMsg(player.sock,msg);
}

void CPlayer::PlayerQuitGame(CPlayer &player)
{
	if (player.roleId>0)
	{
		scene_manager::instance().scene_map_[player.curSceneId]->PlayerExitScene(player.roleId);
		player.SavingPlayerInfo();
		player.SavingBackpackData();
		player.SaveSowlStoneBagToDB();
		for (int i =0;i<21;i++)
		{
			player.sowlstone_data[i]->SaveSowlStoneInfoToDB();
		}
	}
	//COnlineUserList::instance().DelUser(player.userId);
}

void CPlayer::PlayerUpgrade()
{
	level_++;
	CNetMessage msg;
	msg.SetType(GC_PLAYER_UPGRADE);
	maxhp_+=server_data::instance().UpgradeInfo_map[level_].m_nHP;
	maxmp_+=server_data::instance().UpgradeInfo_map[level_].m_nMP;
	AttriPoint_+=server_data::instance().UpgradeInfo_map[level_].m_nAP;
	SkillPoint_+=server_data::instance().UpgradeInfo_map[level_].m_nSP;
	next_exp = server_data::instance().UpgradeInfo_map[level_].m_nExp;
	msg<<level_<<next_exp<<maxhp_<<maxmp_<<AttriPoint_<<SkillPoint_;
	CGameSocket::instance().SendMsg(sock,msg);
	
}

int CPlayer::PlayerAddItem(SItem& item,int needcells)
{
	int ColuType = server_data::instance().UseItemData_map[item.type_id].m_nColuType;
	int OverLap = server_data::instance().UseItemData_map[item.type_id].m_eOverLap;
	const int OverLapNum = 99;
	int space = 0;
	int count = 0;
	int cells = 0;
	
		if (ColuType == 5)
		{
			if (OverLap == 1)
			{
				for (int i= 0;i<32;i++)
				{
					if (decor_data[i].type_id == 0)
					{
						cells++;
					}
				}
				if (needcells<cells)
				{
					for (int i = 0;i<32;i++)
					{
						if (decor_data[i].type_id == 0)
						{
							decor_data[i]=item;
							CombinationData(decor_data[i]);
							break;
						}
						if (i==31)
						{
							return 0;
						}
					}
				 }
				else
				{
					return 0;	
				}
			 }
			else
			{
				for (int i = 0;i<32;i++)
				{
					if (decor_data[i].type_id == item.type_id)
					{
						count += (OverLapNum - decor_data[i].num);
					}
					if (decor_data[i].type_id == 0)
					{
						count+=OverLapNum;
					}
				}
				if(count>item.num) 
				{	
					for (int i = 0;i<32;i++)
					{
						if (decor_data[i].type_id == 0)
						{
							decor_data[i]=item;
							CombinationData(decor_data[i]);
							break;
						}
						if(decor_data[i].type_id == item.type_id)
						{
							space = (OverLapNum - decor_data[i].num);
							if (space>(item.num+decor_data[i].num))
							{
								decor_data[i].num+=item.num;
								CombinationData(decor_data[i]);
								break;
							}
							else
							{

								decor_data[i].num = OverLapNum;
								item.num -= (OverLapNum-decor_data[i].num);
								CombinationData(decor_data[i]);
								break;
							}
						}	
					}
				}
				else
				{
					return false;
				}
			}
			return decorpack_id;
		}
			
		
		if (ColuType == 1)
		{
			if (OverLap == 1)
			{
				for (int i= 0;i<40;i++)
				{
					if (equip_data[i].type_id == 0)
					{
						cells++;
					}
				}
				if (needcells<cells)
				{
					for (int i = 0;i<40;i++)
					{
						if (equip_data[i].type_id == 0)
						{
							equip_data[i]=item;
							CombinationData(equip_data[i]);
							break;
						}
						if (i==39)
						{
							return 0;
						}
					}
				}
				else
				{
					return 0;
				}
			}
			else
			{
				for (int i = 0;i<40;i++)
				{
					if (equip_data[i].type_id == item.type_id)
					{
						count += (OverLapNum - equip_data[i].num);
					}
					if (equip_data[i].type_id == 0)
					{
						count+=OverLapNum;
					}
				}
				if(count>item.num) 
				{	
					for (int i = 0;i<40;i++)
					{
						if (equip_data[i].type_id == 0)
						{
							equip_data[i]=item;
							CombinationData(equip_data[i]);
							break;
						}
						if(equip_data[i].type_id == item.type_id)
						{
							space = (OverLapNum - equip_data[i].num);
							if (space>(item.num+equip_data[i].num))
							{
								equip_data[i].num+=item.num;
								CombinationData(equip_data[i]);
								break;
							}
							else
							{
								equip_data[i].num = OverLapNum;
								CombinationData(equip_data[i]);
								item.num -= (OverLapNum-equip_data[i].num);
							}
						}	
					}
				}
				else
				{
					return 0;
				}
					
			}
			return equippack_id;
		}
		
	
	if (ColuType == 2)
	{
		
		if (OverLap == 1)
		{
			for (int i= 0;i<40;i++)
			{
				if (expend_data[i].type_id == 0)
				{
					cells++;
				}
			}
			if (needcells<cells)
			{
				for (int i = 0;i<40;i++)
				{
					if (expend_data[i].type_id == 0)
					{
						expend_data[i]=item;
						CombinationData(expend_data[i]);
						break;
					}
					if (i==39)
					{
						return 0;
					}
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			for (int i = 0;i<40;i++)
			{
				if (expend_data[i].type_id == item.type_id)
				{
					count += (OverLapNum - expend_data[i].num);
				}
				if (expend_data[i].type_id == 0)
				{
					count+=OverLapNum;
				}
			}
			if(count>item.num) 
			{	
				for (int i = 0;i<40;i++)
				{
					if (expend_data[i].type_id == 0)
					{
						expend_data[i]=item;
						CombinationData(expend_data[i]);
						break;
					}
					if(expend_data[i].type_id == item.type_id)
					{
						space = (OverLapNum - expend_data[i].num);
						if (space>(item.num+expend_data[i].num))
						{
							expend_data[i].num+=item.num;
							CombinationData(expend_data[i]);
							break;
						}
						else
						{
							expend_data[i].num = OverLapNum;
							CombinationData(expend_data[i]);
							item.num -= (OverLapNum-expend_data[i].num);
						}
					}	
				}
			}
			else
			{
				return 0;
			}
		}
		return expendpack_id;
		
	}							//拾取消耗品图标
	if (ColuType == 3)
	{
		if (OverLap == 1)
		{
			for (int i= 0;i<40;i++)
			{
				if (material_data[i].type_id == 0)
				{
					cells++;
				}
			}
			if (needcells<cells)
			{
				for (int i = 0;i<40;i++)
				{
					if (material_data[i].type_id == 0)
					{
						material_data[i]=item;
						CombinationData(material_data[i]);
						break;
					}
					if (i==39)
					{
						return 0;
					}
				}
			}
			else
			{
				return 0;
			}
			
		}
		else
		{
			for (int i = 0;i<40;i++)
			{
				if (material_data[i].type_id == item.type_id)
				{
					count += (OverLapNum - material_data[i].num);
				}
				if (material_data[i].type_id == 0)
				{
					count+=OverLapNum;
				}
			}
			if(count>item.num) 
			{	
				for (int i = 0;i<40;i++)
				{
					if (material_data[i].type_id == 0)
					{
						material_data[i]=item;
						CombinationData(material_data[i]);
						break;
					}
					if(material_data[i].type_id == item.type_id)
					{
						space = (OverLapNum - material_data[i].num);
						if (space>(item.num+material_data[i].num))
						{
							material_data[i].num+=item.num;
							CombinationData(material_data[i]);
							break;
						}
						else
						{
							material_data[i].num = OverLapNum;
							item.num -= (OverLapNum-material_data[i].num);
							CombinationData(material_data[i]);
						}
					}	
				}
			}
			else
			{
				return 0;
			}
		}
		return materialpack_id;
		
	}							//拾取材料图标
	if (ColuType == 4)
	{
		if (OverLap == 1)
		{
			for (int i= 0;i<40;i++)
			{
				if (task_data[i].type_id == 0)
				{
					cells++;
				}
			}
			if (needcells<cells)
			{
				for (int i = 0;i<40;i++)
				{
					if (task_data[i].type_id == 0)
					{
						task_data[i]=item;
						CombinationData(task_data[i]);
						break;
					}
					if (i==39)
					{
						return 0;
					}
				}
			}
			else
			{
				return 0;
			}
			
		}
		else
		{
			for (int i = 0;i<40;i++)
			{
				if (task_data[i].type_id == item.type_id)
				{
					count += (OverLapNum - task_data[i].num);
				}
				if (task_data[i].type_id == 0)
				{
					count+=OverLapNum;
				}
			}
			if(count>item.num) 
			{	
				for (int i = 0;i<40;i++)
				{
					if (task_data[i].type_id == 0)
					{
						task_data[i]=item;
						CombinationData(task_data[i]);
						break;
					}
					if(task_data[i].type_id == item.type_id)
					{
						space = (OverLapNum - task_data[i].num);
						if (space>(item.num+task_data[i].num))
						{
							task_data[i].num+=item.num;
							CombinationData(task_data[i]);
							break;
						}
						else
						{
							task_data[i].num = OverLapNum;
							CombinationData(task_data[i]);
							item.num -= (OverLapNum-task_data[i].num);
						}
					}	
				}
			}
			else
			{
				return 0;
			}		
			
		}
		return taskpack_id;
	}
		return 0;
}

int CPlayer::PlayerLoseItem(SItem& item)
{
	CNetMessage msg;
	msg.SetType(GC_PACK_UPDATA);
	int type = server_data::instance().UseItemData_map[item.type_id].m_nType;
	int OverLap = server_data::instance().UseItemData_map[item.type_id].m_eOverLap;
	const int OverLapNum = 99;
	int space = 0;
	int count = 0;
	SItem ref_item;
	ref_item.id = 0;

	if (type<=100)//物品类型
	{
		if (type<=40&&type>20)
		{
			for (int i = 0;i<32;i++)
			{
				if (decor_data[i].id == item.id)
				{
					if (OverLap == 1)
					{
						decor_data[i] = SItem::CreateItem(0);
						CombinationData(decor_data[i]);
						break;
					}
					else 
					{
						decor_data[i].num--;
						if (decor_data[i].num>0)
						{

							CombinationData(decor_data[i]);
							break;
						}
						else
						{
							decor_data[i] = SItem::CreateItem(0);
							CombinationData(decor_data[i]);
							break;
						}
					}
					
				}
				if (i == 31)
				{
					return 0;
				}
			}
			
			msg<<item.type_id<<decorpack_id;
			for (int i = 0;i<32;i++)
			{
				msg<<decor_data[i].type_id<<decor_data[i].id<<decor_data[i].num;
			}
			CGameSocket::instance().SendMsg(sock,msg);
			return decorpack_id;
		}
		else
		{
			for (int i = 0;i<40;i++)
			{
				if (equip_data[i].id == item.id)
				{
					if (OverLap == 1)
					{
						equip_data[i] = SItem::CreateItem(0);
						CombinationData(equip_data[i]);
						break;	
					}
					else
					{
						equip_data[i].num--;
						if (equip_data[i].num>0)
						{

							CombinationData(equip_data[i]);
							break;
						}
						else
						{
							equip_data[i] = SItem::CreateItem(0);
							CombinationData(equip_data[i]);
							break;
						}
					}
		
				}
				if (i == 39)
				{
					return 0;
				}
			}
			msg<<item.type_id<<equippack_id;
			for (int i = 0;i<40;i++)
			{
				msg<<equip_data[i].type_id<<equip_data[i].id<<equip_data[i].num;
			}
			CGameSocket::instance().SendMsg(sock,msg);
			return equippack_id;
		}
	}
	if (type<=200&&type>100)
	{
		for (int i = 0;i<40;i++)
		{
   			if (expend_data[i].id == item.id)
			{
				if (OverLap == 1)
				{
					expend_data[i] = SItem::CreateItem(0);
					CombinationData(expend_data[i]);
					break;
				}
				else
				{
					expend_data[i].num--;
					if (expend_data[i].num>0)
					{

						CombinationData(expend_data[i]);
						break;
					}
					else
					{
						expend_data[i] = SItem::CreateItem(0);
						CombinationData(expend_data[i]);
						break;
					}
				}
				
			}

			if (i == 39)
			{
				return 0;
			}
		}
		msg<<item.type_id<<expendpack_id;
		for (int i = 0;i<40;i++)
		{
			msg<<expend_data[i].type_id<<expend_data[i].id<<expend_data[i].num;
		}
		CGameSocket::instance().SendMsg(sock,msg);
		return expendpack_id;
	}
	if (type<=300&&type>200)
	{
		for (int i = 0;i<40;i++)
		{
			if (OverLap == 1)
			{
				material_data[i] = SItem::CreateItem(0);
				CombinationData(material_data[i]);
				break;
			}
			else
			{
				material_data[i].num--;
				if (material_data[i].num>0)
				{

					CombinationData(material_data[i]);
					break;
				}
				else
				{
					material_data[i] = SItem::CreateItem(0);
					CombinationData(material_data[i]);
					break;
				}
			}
		
			if (i == 39)
			{
				return 0;
			}
		}
		msg<<item.type_id<<materialpack_id;
		for (int i = 0;i<40;i++)
		{
			msg<<material_data[i].type_id<<material_data[i].id<<material_data[i].num;
		}
		CGameSocket::instance().SendMsg(sock,msg);
		return materialpack_id;
	}
	if (type<=400&&type>300)
	{	
		for (int i = 0;i<40;i++)
		{
			if (task_data[i].id == item.id)
			{
				if (OverLap == 1)
				{
					task_data[i] = SItem::CreateItem(0);
					CombinationData(task_data[i]);
					break;
				}
				else
				{
					task_data[i].num--;
					if (task_data[i].num>0)
					{

						CombinationData(task_data[i]);
						break;
					}
					else
					{
						task_data[i] = SItem::CreateItem(0);
						CombinationData(task_data[i]);
						break;
					}
				}
			}
			if (i == 39)
			{
				return 0;
			}
		}
		msg<<item.type_id<<taskpack_id;
		for (int i = 0;i<40;i++)
		{
			msg<<task_data[i].type_id<<task_data[i].id<<task_data[i].num;
		}
		CGameSocket::instance().SendMsg(sock,msg);
		return taskpack_id;
	}
	return false;
}

void CPlayer::PlayerAddSowlStone(SItem* item)
{
	
	
	CNetMessage msg;
	int pos;
	msg.SetType(GC_COLLECT_SOWL);
	int cells = 0;
	for (int i= 0;i<18;i++)
	{
		if (sowlstone_data[i]->type_id == 0)
		{
			cells++;
		}
	}
	if (cells>1)
	{
		for (int i = 0;i<18;i++)
		{
			if (sowlstone_data[i]->type_id == 0)
			{
				//SowlStone* sl = SowlStone::CreateSowlStone(*item);
				int db_id = sowlstone_data[i]->db_id;
				delete sowlstone_data[i];
				sowlstone_data[i]= (SowlStone*)item;
				sowlstone_data[i]->db_id = db_id;
				pos = i;
				break;
			}
		}
		msg<<pos<<sowlstone_data[pos]->id<<sowlstone_data[pos]->type_id<<sowlstone_data[pos]->title_<<sowlstone_data[pos]->level_
			<<sowlstone_data[pos]->bindid<<sowlstone_data[pos]->quality_<<sowlstone_data[pos]->maxExp_<<sowlstone_data[pos]->curExp_
			<<sowlstone_data[pos]->maxhp_<<sowlstone_data[pos]->maxmp_<<sowlstone_data[pos]->power_<<sowlstone_data[pos]->quick_
			<<sowlstone_data[pos]->intelligence_<<sowlstone_data[pos]->energy_<<sowlstone_data[pos]->phyattack_<<sowlstone_data[pos]->maattack_
			<<sowlstone_data[pos]->phydefend_<<sowlstone_data[pos]->madefend_<<sowlstone_data[pos]->attackspeed_
			<<sowlstone_data[pos]->hitrate_<<sowlstone_data[pos]->movespeed_<<sowlstone_data[pos]->evadePro_<<sowlstone_data[pos]->releasePro_
			<<sowlstone_data[pos]->MPRevert_<<sowlstone_data[pos]->HPRevert_<<sowlstone_data[pos]->CarryWeight_<<sowlstone_data[pos]->RigidTime_
			<<sowlstone_data[pos]->resistance_<<sowlstone_data[pos]->skill1_<<sowlstone_data[pos]->skill2_<<sowlstone_data[pos]->skill3_
			<<sowlstone_data[pos]->state_;

		CGameSocket::instance().SendMsg(sock,msg);
	}
	
	
	
}

int CPlayer::PlayerCheckItemSpace(int bagid)
{
	return 0;
}

void CPlayer::PlayerAddAttriPoint(CNetMessage& msg)
{
	int tempPower,tempQuick,tempIntelligence,tempEnergy;
	msg>>tempPower>>tempQuick>>tempIntelligence>>tempEnergy;
	int usePoint;
	usePoint = ((tempPower-power_)+(tempQuick-quick_)+(tempIntelligence-intelligence_)+(tempEnergy-energy_));
	if ((usePoint<=AttriPoint_)&&(usePoint>0))
	{
		power_ = tempPower;
		quick_ = tempQuick;
		intelligence_ = tempIntelligence;
		energy_ = tempEnergy; 
		AttriPoint_-=usePoint;
		CountPlayerAttr();
	}
	msg.ReWrite();
	msg.SetType(GC_ROLE_INFO);
	PacketPlayerInfo(msg);
	CGameSocket::instance().SendMsg(sock,msg);
}

void CPlayer::PlayerRoleInfo(CNetMessage& msg)
{	
	msg.ReWrite();
	msg.SetType(GC_ROLE_INFO);
	PacketPlayerInfo(msg);
	CGameSocket::instance().SendMsg(sock,msg);
}

void CPlayer::PlayerDeleteItem(CNetMessage &msg)
{
	SItem item;
	msg>>item.id>>item.type_id>>item.num;
	msg.ReWrite();
	msg.SetType(GC_USE_DELETE_ITEM);
	int bpackid = 0;
	if (bpackid = PlayerLoseItem(item))
	{
		msg<<0;//成功
	}
	else
	{
		msg<<1;//失败
	}
	CGameSocket::instance().SendMsg(sock,msg);
	SendUpdateBackPackData(bpackid);
}

void CPlayer::PlayerUseItem(CNetMessage &msg)
{
	SItem item;
	SItem item1;
	msg>>item.id>>item.type_id;

	CNetMessage msg1;
	string item_name = server_data::instance().ItemEData_map[item.type_id].m_szName;

	msg.ReWrite();
	msg.SetType(GC_USE_ITEM);

	int place = GetItemById(item.id,item);

		msg<<SUCCESS_FLAG<<(string)item_name;

		if (server_data::instance().UseItemData_map.find(item.type_id)!=server_data::instance().UseItemData_map.end())
		{
			int ColuType =  server_data::instance().UseItemData_map[item.type_id].m_nColuType;
			if (ColuType == 1)//装备栏物品
			{
				int type = server_data::instance().UseItemData_map[item.type_id].m_nType;
				switch(type)
				{
				case 1://头盔 
					item1 = equip_data[40];
					equip_data[40] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 2://肩甲
					item1 = equip_data[41];
					equip_data[41] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 3://铠甲
					item1 = equip_data[42];
					equip_data[44] = equip_data[place];
					equip_data[place] = item1;
				    break;
				case 4://腰带
					item1 = equip_data[46];
					equip_data[46] = equip_data[place];
					equip_data[place] = item1;
				    break;
				case 5://护手
					item1 = equip_data[47];
					equip_data[47] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 6://护腿
					item1 = equip_data[48];
					equip_data[48] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 7://鞋子
					item1 = equip_data[45];
					equip_data[45] = equip_data[place];
					equip_data[place] = item1;
				    break;
				case 8://徽章
					item1 = equip_data[42];
					equip_data[42] = equip_data[place];
					equip_data[place] = item1;
				    break;
				case 9://项链
					item1 = equip_data[49];
					equip_data[49] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 10://手镯
					item1 = equip_data[50];
					equip_data[50] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 11://戒指
					item1 = equip_data[51];
					equip_data[51] = equip_data[place];
					equip_data[place] = item1;
					break;
				case 12://武器
					item1 = equip_data[43];
					equip_data[43] = equip_data[place];
					equip_data[place] = item1;
					break;
				default:
				    break;
				}

				SendUpdateBackPackData(equippack_id);
			}
			if (ColuType == 5)//装饰栏物品
			{
				int type = server_data::instance().UseItemData_map[item.type_id].m_nType;
				
				switch(type)
				{
				case 21://头盔
					item1 = decor_data[32];
					decor_data[32] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 22://头发
					item1 = decor_data[33];
					decor_data[33] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 23://眼镜
					item1 = decor_data[34];
					decor_data[34] = decor_data[place];
					decor_data[place] = item1;
				    break;
				case 24://面部
					item1 = decor_data[36];
					decor_data[36] = decor_data[place];
					decor_data[place] = item1;
				    break;
				case 25://项链
					item1 = decor_data[35];
					decor_data[35] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 26://披风
					item1 = decor_data[37];
					decor_data[37] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 27://肩甲
					item1 = decor_data[38];
					decor_data[38] = decor_data[place];
					decor_data[place] = item1;
				    break;
				case 28://铠甲
					item1 = decor_data[39];
					decor_data[39] = decor_data[place];
					decor_data[place] = item1;
				    break;
				case 29://腰带
					item1 = decor_data[42];
					decor_data[42] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 30://护手
					item1 = decor_data[40];
					decor_data[40] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 31://护腿
					item1 = decor_data[43];
					decor_data[43] = decor_data[place];
					decor_data[place] = item1;
					break;
				case 32://鞋子
					item1 = decor_data[41];
					decor_data[41] = decor_data[place];
					decor_data[place] = item1;
					break;
				default:
				    break;
				}
				SendUpdateBackPackData(decorpack_id);
			}
			if (ColuType == 2)
			{
				switch(item.type_id)
				{
				case 40000://扭蛋
					CNpcManager::instance().WringEgg(&item,sock);
					break;
				case 13010://小红
					msg1.SetType(GC_USE_ADD_HP);
					hp_+=50;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13011://中红
					msg1.SetType(GC_USE_ADD_HP);
					hp_+=100;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13012://大红
					msg1.SetType(GC_USE_ADD_HP);
					hp_+=200;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13013://超级大红
					msg1.SetType(GC_USE_ADD_HP);
					hp_+=300;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13050://小蓝
					msg1.SetType(GC_USE_ADD_MP);
					mp_+=50;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13051://中蓝
					msg1.SetType(GC_USE_ADD_MP);
					mp_+=100;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13052://大蓝
					msg1.SetType(GC_USE_ADD_MP);
					mp_+=200;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
				case 13053://超级大蓝
					msg1.SetType(GC_USE_ADD_MP);
					mp_+=300;
					msg1<<hp_;
					PlayerLoseItem(item);
					break;
					/*	case:
					break;
					case:
					break;
					case:
					break;
					case:
					break;
					case:
					break;
					case:
					break;
					case:
					break;*/
				default:

					break;
				}
				SendUpdateBackPackData(expendpack_id);
				CGameSocket::instance().SendMsg(sock,msg1);
			}
			
			
		}

	
}

void CPlayer::PlayerExchangeItemPlace(CNetMessage& msg)
{
	int place_1,place_2;
	int bpack_id;
	msg>>bpack_id>>place_1>>place_2;
	SItem temp;
	int type;

	if (bpack_id == equippack_id)
	{
		if (place_2>39 && place_2<52)
		{
			type = server_data::instance().UseItemData_map[equip_data[place_1].type_id].m_nType;
			switch(type)
			{
			case 1://头盔 
				if (place_2 == 40)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 2://肩甲
				if (place_2 == 41)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 3://铠甲
				if (place_2 == 44)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 4://腰带
				if (place_2 == 46)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 5://护手
				if (place_2 == 47)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 6://护腿
				if (place_2 == 48)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 7://鞋子
				if (place_2 == 45)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 8://徽章
				if (place_2 == 42)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 9://项链
				if (place_2 == 49)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 10://手镯
				if (place_2 == 50)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 11://戒指
				if (place_2 == 51)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			case 12://武器
				if (place_2 == 43)
				{
					temp = equip_data[place_1];
					equip_data[place_1] = equip_data[place_2];
					equip_data[place_2] = temp;
				}
				break;
			default:
				break;
				
			}
		}
		else
		{
			temp = equip_data[place_1];
			equip_data[place_1] = equip_data[place_2];
			equip_data[place_2] = temp;
		}
		
		
	}
	if (bpack_id == expendpack_id)
	{
		temp = expend_data[place_1];
		expend_data[place_1] = expend_data[place_2];
		expend_data[place_2] = temp;
	}
	
	if (bpack_id == materialpack_id)
	{
		temp = material_data[place_1];
		material_data[place_1] = material_data[place_2];
		material_data[place_2] = temp;
	}
	
	if (bpack_id == taskpack_id)
	{
		temp = task_data[place_1];
		task_data[place_1] = task_data[place_2];
		task_data[place_2] = temp;
	}
	
	if (bpack_id == decorpack_id)
	{
		if (place_2>31 && place_2<44)
		{
			switch(type)
			{
			case 21://头盔
				if (place_2 == 32)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 22://头发
				if (place_2 == 33)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 23://眼镜
				if (place_2 == 34)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 24://面部
				if (place_2 == 36)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 25://项链
				if (place_2 == 35)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 26://披风
				if (place_2 == 37)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 27://肩甲
				if (place_2 == 38)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 28://铠甲
				if (place_2 == 39)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 29://腰带
				if (place_2 == 42)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 30://护手
				if (place_2 == 40)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 31://护腿
				if (place_2 == 43)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			case 32://鞋子
				if (place_2 == 41)
				{
					temp = decor_data[place_1];
					decor_data[place_1] = decor_data[place_2];
					decor_data[place_2] = temp;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			temp = decor_data[place_1];
			decor_data[place_1] = decor_data[place_2];
			decor_data[place_2] = temp;
		}
	
	}

	if (bpack_id == 0)
	{
		SowlStone* sl = sowlstone_data[place_1];
		sowlstone_data[place_1] = sowlstone_data[place_2];
		sowlstone_data[place_2] = sl;
		/*if (place_2>)
		{
		}*/
		SendAndUpdataSowlStoneInfo();
	}
	
	SendUpdateBackPackData(bpack_id);
	
}

void CPlayer::SendUpdateBackPackData(int bpackid)
{
	CNetMessage msg;
	msg.SetType(GC_PACK_UPDATA);
	if (bpackid == equippack_id)
	{

		msg<<equippack_id;
		for (int i = 0;i<52;i++)
		{
			msg<<equip_data[i].type_id<<equip_data[i].id<<equip_data[i].num
				<<1/*物品等级*/<<equip_data[i].SItemData.m_nSellPric<<equip_data[i].SItemData.m_eBind
				<<equip_data[i].SItemData.m_eOccupation<<equip_data[i].SItemData.m_nLevel<<equip_data[i].SItemData.m_fWeight<<equip_data[i].SItemData.m_nEndurance
				<<100/*当前耐久*/<<equip_data[i].SItemData.m_nMaxPhAttack<<equip_data[i].SItemData.m_nMinPhAttack<<equip_data[i].SItemData.m_nMaxMaAttack
				<<equip_data[i].SItemData.m_nMinMaAttack<<equip_data[i].SItemData.m_nPhDefend<<equip_data[i].SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<equip_data[i].enchase[i];
			}
		}
		
	}
	if (bpackid == expendpack_id)
	{

		msg<<expendpack_id;
		for (int i = 0;i<52;i++)
		{
			msg<<expend_data[i].type_id<<expend_data[i].id<<expend_data[i].num
				<<1/*物品等级*/<<expend_data[i].SItemData.m_nSellPric<<expend_data[i].SItemData.m_eBind
				<<expend_data[i].SItemData.m_eOccupation<<expend_data[i].SItemData.m_nLevel<<expend_data[i].SItemData.m_fWeight<<expend_data[i].SItemData.m_nEndurance
				<<100/*当前耐久*/<<expend_data[i].SItemData.m_nMaxPhAttack<<expend_data[i].SItemData.m_nMinPhAttack<<expend_data[i].SItemData.m_nMaxMaAttack
				<<expend_data[i].SItemData.m_nMinMaAttack<<expend_data[i].SItemData.m_nPhDefend<<expend_data[i].SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<expend_data[i].enchase[i];
			}
		}
	}

	if (bpackid == materialpack_id)
	{

		msg<<materialpack_id;
		for (int i = 0;i<52;i++)
		{
			msg<<material_data[i].type_id<<material_data[i].id<<material_data[i].num
				<<1/*物品等级*/<<material_data[i].SItemData.m_nSellPric<<material_data[i].SItemData.m_eBind
				<<material_data[i].SItemData.m_eOccupation<<material_data[i].SItemData.m_nLevel<<material_data[i].SItemData.m_fWeight<<material_data[i].SItemData.m_nEndurance
				<<100/*当前耐久*/<<material_data[i].SItemData.m_nMaxPhAttack<<material_data[i].SItemData.m_nMinPhAttack<<material_data[i].SItemData.m_nMaxMaAttack
				<<material_data[i].SItemData.m_nMinMaAttack<<material_data[i].SItemData.m_nPhDefend<<material_data[i].SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<material_data[i].enchase[i];
			}
		}
	}

	if (bpackid == taskpack_id)
	{

		msg<<taskpack_id;
		for (int i = 0;i<52;i++)
		{
			msg<<task_data[i].type_id<<task_data[i].id<<task_data[i].num
				<<1/*物品等级*/<<task_data[i].SItemData.m_nSellPric<<task_data[i].SItemData.m_eBind
				<<task_data[i].SItemData.m_eOccupation<<task_data[i].SItemData.m_nLevel<<task_data[i].SItemData.m_fWeight<<task_data[i].SItemData.m_nEndurance
				<<100/*当前耐久*/<<task_data[i].SItemData.m_nMaxPhAttack<<task_data[i].SItemData.m_nMinPhAttack<<task_data[i].SItemData.m_nMaxMaAttack
				<<task_data[i].SItemData.m_nMinMaAttack<<task_data[i].SItemData.m_nPhDefend<<task_data[i].SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<task_data[i].enchase[i];
			}
		}
	}

	if (bpackid == decorpack_id)
	{

		msg<<decorpack_id;
		for (int i = 0;i<52;i++)
		{
			msg<<decor_data[i].type_id<<decor_data[i].id<<decor_data[i].num
				<<1/*物品等级*/<<decor_data[i].SItemData.m_nSellPric<<decor_data[i].SItemData.m_eBind
				<<decor_data[i].SItemData.m_eOccupation<<decor_data[i].SItemData.m_nLevel<<decor_data[i].SItemData.m_fWeight<<decor_data[i].SItemData.m_nEndurance
				<<100/*当前耐久*/<<decor_data[i].SItemData.m_nMaxPhAttack<<decor_data[i].SItemData.m_nMinPhAttack<<decor_data[i].SItemData.m_nMaxMaAttack
				<<decor_data[i].SItemData.m_nMinMaAttack<<decor_data[i].SItemData.m_nPhDefend<<decor_data[i].SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<decor_data[i].enchase[i];
			}
		}
	}
	CGameSocket::instance().SendMsg(sock,msg);

}

void CPlayer::CountPlayerAttr()
{
	maxhp_=(power_*3)*10;
	maxmp_=(intelligence_*3)*10;
	phyattack_=(power_*2)*10;
	maattack_=(intelligence_*2)*10;
	phydefend_=(int)(power_*0.7+energy_*0.3)*10;
	madefend_=(int)(intelligence_*0.6+quick_*0.4)*10;
	attackspeed_=(int)(power_*0.6+quick_*0.4)*10;
	hitrate_=(int)(power_*0.3+quick_*0.7)*10/*/1000*/;
	movespeed_=(int)(quick_*0.1)*10;
	evadePro_=(int)(quick_*0.1)/*1000*/;
	releasePro_=(int)(quick_*0.4+intelligence_*0.6)*10;
	resistance_=(int)(quick_*0.5+intelligence_*0.5)*10;
	MPRevert_=(int)(intelligence_*0.5+energy_*0.5)*10;
	HPRevert_=(int)(energy_*0.6+power_*0.4)*10;
	CarryWeight_=(int)(energy_*0.2)*10;
	RigidTime_=(int)(energy_*0.1)*10;
	FrozenRessis_=(resistance_+movespeed_)/*/5000*/;
	BurnRessis_=(resistance_+maxhp_)/*/5000*/;
	PalsyRessis_=(resistance_+evadePro_)/*/5000*/;
	ToxinRessis_=(resistance_+HPRevert_)/*/5000*/;
	VertigoRessis_=(resistance_+phydefend_)/*/5000*/;
	skillRessis_=(resistance_+hitrate_)/*/5000*/;
	MARessis_=(resistance_+releasePro_)/*/5000*/;
	weakRessis_=(resistance_+CarryWeight_)/*/5000*/;
}

void CPlayer::SetExpendShortCut(CNetMessage& msg)
{
	int type_id;
	int place_1,place_2;
  	msg>>place_1>>place_2>>type_id;
	if (place_1<0)
	{
		expend_shortcut[place_2] = type_id;	
	}
	else
	{
		if (place_2!=place_1)
		{
			expend_shortcut[place_1] = expend_shortcut[place_2];
			expend_shortcut[place_2] = type_id;
		}
	}
	
	msg.ReWrite();
	msg.SetType(GC_SET_EXPEND_SHORTCUT);
	for (int i = 0;i<5;i++)
	{
		msg<<expend_shortcut[i];
	}

	CGameSocket::instance().SendMsg(sock,msg);
	
}

void CPlayer::TrimPack(CNetMessage& msg)
{
	vector<SItem> iv;
	for (int i = 0;i<40;i++)
	{
		if (equip_data[i].type_id != 0)
		{
			iv.push_back(equip_data[i]);
			equip_data[i] = SItem::CreateItem(0);
			//equip_data[i].type_id = 0;
		}
		if (expend_data[i].type_id != 0)
		{
			iv.push_back(expend_data[i]);
			expend_data[i] = SItem::CreateItem(0);
			//expend_data[i].type_id = 0;
		}
		if (material_data[i].type_id != 0)
		{
			iv.push_back(material_data[i]);
			material_data[i] = SItem::CreateItem(0);
			//material_data[i].type_id = 0;
		}
		if (task_data[i].type_id != 0)
		{
			iv.push_back(task_data[i]);
			task_data[i] = SItem::CreateItem(0);
			//task_data[i].type_id = 0;
		}
		
	}
	for (int i = 0;i<32;i++)
	{
		if (decor_data[i].type_id != 0)
		{
			iv.push_back(decor_data[i]);
			decor_data[i] = SItem::CreateItem(0);
			//decor_data[i].type_id = 0;
		}

	}
	
	for (size_t i = 0; i < iv.size(); i++)
	{
		PlayerAddItem(iv[i]);
	}
	SendAllBackPackData();
}

void CPlayer::ProcessMsg(CNetMessage &msg)
{
	int type = msg.GetType();
	switch(type)
	{
	case CG_ENTER_GAME:
		PlayerEnterGame(msg);
		break;
	case CG_CHAT_INFO:
		PlayerChat(msg);
		break;
	case CG_ADD_ATTRIPOINT:
		PlayerAddAttriPoint(msg);
		break;
	case CG_ROLE_INFO:
		PlayerRoleInfo(msg);
		break;
	case CG_USE_DORP_ITEM:
		PlayerDeleteItem(msg);
		break;
	case CG_USE_ITEM:
		PlayerUseItem(msg);
		break;
	case CG_EXCHANGE_ITEM_PLACE:
		PlayerExchangeItemPlace(msg);
		break;
	case CG_SET_EXPEND_SHORTCUT:
		SetExpendShortCut(msg);
		break;
	case CG_PACK_TRIM:
		TrimPack(msg);
		break;
	case CG_USE_DELETE_ITEM:
		PlayerDeleteItem(msg);
		break;
		default:
		break;
	}
}

bool CPlayer::AddTask(CNpcTask* pTask)
{
	if( pTask == NULL )
		return false;
	CPlayerTask* pPTask = GetTaskById(pTask->m_id);
	if( pPTask && pPTask->m_repeatedTimes >= pTask->m_canRepeatTimes )//是否超过次数限制
		return false;
//	判定玩家是否可以接此任务
	if( level_ <= pTask->m_level && (occupation_ == pTask->m_occupation || pTask->m_occupation == -1) )
		if( !IsTaskDone(pTask->m_dependTask) )//前置任务没完成
			return false;
	else
		return false;
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
	{
		return false;
	}
	CNpcManager* npcManager = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&npcManager,EGV_NPC_MANAGER);
	if( npcManager == NULL )
		return false;
	list<STaskItem> itemlist;
	STaskItem item;
	list<STaskItem>::iterator i = pTask->m_needItem.begin();
	for ( ; i != pTask->m_needItem.end(); i++ )
	{
		item = *i;
		switch ( i->type )
		{
		case ETIMonster://怪物或对话人物
		case ETITalkMan:
			item.num = 0;
			item.target = i->num;
			break;
		case ETIItem://物品
			item.num = GetItemNumByTypeId( i->val );
			item.target = i->num;
			break;
		case ETIExpe://经验
			item.num = (int)experence;
			item.target = i->num;
			break;
		case ETIMoney://金钱
			item.num = (int)money_;
			item.target = i->num;
			break;
		default:
			break;
		}
		itemlist.push_back(item);
	}
	if ( pPTask )
	{
		pPTask->m_doneItem = itemlist;
		if( itemlist.empty() )
			pPTask->m_doneState = UnCommit;
		else
			pPTask->m_doneState = UnDone;
	}
	else
	{
		CPlayerTask task;
		task.m_roleId = roleId;
		task.m_taskId = pTask->m_id;
		task.m_pTask = pTask;
		task.m_doneItem = itemlist;
		if( itemlist.empty() )
			task.m_doneState = UnCommit;
		m_taskList.push_back(task);
		//	插入数据库
		string sql,itemStr;
		npcManager->WriteTaskItem(&(task.m_doneItem),itemStr,true);
		sql = "insert into player_task values( " + boost::lexical_cast<std::string,int>(task.m_roleId) +
			"," + boost::lexical_cast<std::string,int>(task.m_taskId) + "," +
			boost::lexical_cast<std::string,int>( task.m_doneState) + "," +
			boost::lexical_cast<std::string,int>( task.m_repeatedTimes) + ",'" + itemStr + "')";
		pDb->Query(sql.c_str());
	}
	return true;
}

void CPlayer::UpdateTask(int type,int value,int num)
{
	int npcId = 0;
	string name;
	PlayerTaskListIter iter = m_taskList.begin();
	for ( ; iter != m_taskList.end() ; iter++ )
	{
		if( iter->m_doneState != UnDone )
			continue;
		list<STaskItem>::iterator i = iter->m_doneItem.begin();
		for (; i != iter->m_doneItem.end(); i++)
		{
			if( i->match(type,value) )
			{
				switch ( type )
				{
				case ETIMonster:
				case ETITalkMan:
					if( value == i->val )
						i->num++;
					break;
				case ETIItem:
					if( value == i->val )
						i->num = GetItemNumByTypeId(value);
					break;
				case ETIExpe:
					i->num = (int)experence;
					break;
				case ETIMoney:
					i->num = (int)money_;
					break;
				default:
					break;
				}
				if( i->ok() && iter->ok() )
				{
					npcId = iter->m_pTask->m_npcId;
					name = iter->m_pTask->m_name;
					iter->m_doneState = UnCommit;
					CNetMessage msg;
					msg.SetType(GC_TASKCOMPLETE);
					msg<<0<<npcId<<name;
					CGameSocket::instance().SendMsg(sock,msg);
					SendTaskList();//给玩家发送任务列表
					return;
				}
			}
		}
	}
}

void CPlayer::UT_KillMonster( int id )//杀死怪物，更新任务
{ UpdateTask(1,id,1); }
void CPlayer::UT_AddItem( int id, int num)//增加物品（购买或拾取），更新任务
{ UpdateTask(2,id,num); }
void CPlayer::UT_AddExp( int exp )//增加经验，更新任务
{ UpdateTask(3,exp,0); }
void CPlayer::UT_AddMoney( int money )//增加金钱，更新任务
{ UpdateTask(4,money,0); }

int  CPlayer::GetItemNumByTypeId( int itemTypeId )
{
	int num = 0;
	int i = 0;
	for ( ; i < sizeof(equip_data)/sizeof(SItem); i++ )
	{
		if( itemTypeId == equip_data[i].type_id )
			num += equip_data[i].num;
	}
	for ( i = 0; i < sizeof(expend_data)/sizeof(SItem); i++ )
	{
		if( itemTypeId == expend_data[i].type_id )
			num += expend_data[i].num;
	}
	for ( i = 0; i < sizeof(material_data)/sizeof(SItem); i++ )
	{
		if( itemTypeId == material_data[i].type_id )
			num += material_data[i].num;
	}
	for ( i = 0; i < sizeof(decor_data)/sizeof(SItem); i++ )
	{
		if( itemTypeId == decor_data[i].type_id )
			num += decor_data[i].num;
	}
	return num;
}

bool CPlayer::LoadTaskFromDB()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
	{
		return false;
	}
	string sql = "select * from player_task where role_id = " + boost::lexical_cast<std::string,int>(roleId) ;
	if (!pDb->Query(sql.c_str()))
	{
		return false;
	}
	CNpcManager* npcManager = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&npcManager,EGV_NPC_MANAGER);
	if( npcManager == NULL )
		return false;
	char **row;
	string strItem;
	while((row = pDb->GetRow()) != NULL)
	{
		CPlayerTask task;
		list<STaskItem> itemlist;
		task.m_roleId = roleId;
		task.m_taskId = atoi(row[1]);
		task.m_doneState = (ETaskState)atoi(row[2]);
		task.m_repeatedTimes = atoi(row[3]);
		strItem = row[4];
		if( !(task.m_pTask = npcManager->GetTaskById(task.m_taskId)) )
			continue;
		if( npcManager->ReadTaskItem(&itemlist,strItem,true) )
		{
			task.m_doneItem = itemlist;
			m_taskList.push_back(task);
		}
		else
		{
			if( task.m_doneState == Done )
			{
				task.m_doneItem = itemlist;
				m_taskList.push_back(task);
			}
		}
	}
	SendTaskList();
	return true;
}

bool CPlayer::SaveTaskToDB()
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
	{
		return false;
	}
	CNpcManager* npcManager = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&npcManager,EGV_NPC_MANAGER);
	if( npcManager == NULL )
		return false;
	string sql,itemStr;
	PlayerTaskListIter iter = m_taskList.begin();
	for ( ; iter != m_taskList.end(); iter++ )
	{
		npcManager->WriteTaskItem(&(iter->m_doneItem),itemStr,true);
		sql = "update player_task set done_state = " + boost::lexical_cast<std::string,int>( iter->m_doneState) +
			",repeated_times = " + boost::lexical_cast<std::string,int>( iter->m_repeatedTimes) + ", done_item = '"
			+ itemStr + "' where role_id = " + boost::lexical_cast<std::string,int>(iter->m_roleId) + " and task_id = " +
			boost::lexical_cast<std::string,int>(iter->m_taskId) ;
		pDb->Query(sql.c_str());
	}
	return true;
}

void CPlayer::AnalyzeData(string &str,SItem &item)
{
	size_t pos = -1;
	do
	{
		pos += 1;
		item.type_id = atoi(str.c_str() + pos);
		pos = str.find('|',pos);
		pos += 1;
		item.bindid = atoi(str.c_str() + pos);
		pos = str.find('|',pos);
		if(pos == string::npos)
			return;
		pos += 1;
		item.num = atoi(str.c_str() + pos);
		str.clear();
		item = SItem::CreateItem(item);
		
	}while((pos = str.find('|',pos)) != string::npos);
	return;
}

void CPlayer::CombinationData(SItem &item)
{
	item.str.clear();
	item.str = ""+boost::lexical_cast<std::string,int>(item.type_id)+"|"
		+boost::lexical_cast<std::string,int>(item.bindid)+"|"+boost::lexical_cast<std::string,int>(item.num);

}

//SItem CPlayer::CreateItem(int type_id,int num)
//{
//	
//}

int CPlayer::GetItemById(int id,SItem& item)
{
	int i = 0;
	for ( ; i < sizeof(equip_data)/sizeof(SItem); i++ )
	{
		if( id == equip_data[i].id )
		{
			item = equip_data[i];
			return i;
		}
	}
	for ( i = 0; i < sizeof(expend_data)/sizeof(SItem); i++ )
	{
		if( id == expend_data[i].id )
		{
			item = expend_data[i];
			return i;
		}
	}
	for ( i = 0; i < sizeof(material_data)/sizeof(SItem); i++ )
	{
		if( id == material_data[i].id )
		{
			item = material_data[i];
			return i;
		}
	}
	for ( i = 0; i < sizeof(decor_data)/sizeof(SItem); i++ )
	{
		if( id == decor_data[i].id )
		{
			item = decor_data[i];
			return i;
		}
	}
	return false;
}

bool CPlayer::RemoveItem(int itemTypeId,int num )
{
	if ( num < 1 || GetItemNumByTypeId(itemTypeId) < num )
		return false;
	int i = 0;
	for ( ; i < sizeof(equip_data)/sizeof(SItem) && num >= 0; i++ )
	{
		if( itemTypeId == equip_data[i].type_id )
		{
			num -= equip_data[i].num;
			PlayerLoseItem(equip_data[i]);
		}
	}
	for ( i = 0; i < sizeof(expend_data)/sizeof(SItem) && num >= 0; i++ )
	{
		if( itemTypeId == expend_data[i].type_id )
		{
			num -= expend_data[i].num;
			PlayerLoseItem(expend_data[i]);
		}
	}
	for ( i = 0; i < sizeof(material_data)/sizeof(SItem) && num >= 0; i++ )
	{
		if( itemTypeId == material_data[i].type_id )
		{
			num -= material_data[i].num;
			PlayerLoseItem(material_data[i]);
		}
	}
	for ( i = 0; i < sizeof(decor_data)/sizeof(SItem) && num >= 0; i++ )
	{
		if( itemTypeId == decor_data[i].type_id )
		{
			num -= decor_data[i].num;
			PlayerLoseItem(decor_data[i]);
		}
	}
	return true;
}

bool CPlayer::GetGotTask(int taskId,CPlayerTask& task)
{
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end() ; it++ )
	{
		if( taskId == it->m_taskId && it->m_doneState == UnDone )
		{
			task = *it;
			return true;
		}
	}
	return false;
}


bool CPlayer::IsTaskDoneUnCommit(int taskId)
{
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end() ; it++ )
	{
		if( taskId == it->m_taskId && it->m_doneState == UnCommit )
			return true;
	}
	return false;
}

bool CPlayer::CanGetTask( int taskId )
{
//	CPlayerTask* pUnComTask = GetUnCommitTaskById(taskId);
	CPlayerTask* pDoneTask  = GetDoneTaskById(taskId);
	CNpcManager* pNpcManager = NULL;
	CGlobalValManager::CreateInstance()->GetVal( &pNpcManager, EGV_NPC_MANAGER );
	if ( pNpcManager )
	{
		CNpcTask* pTask = pNpcManager->GetTaskById(taskId);
		if ( pTask )
		{
			if( level_ >= pTask->m_level && (pTask->m_occupation & occupation_) //等级和职业判断
				&& (pTask->m_dependTask == 0 || IsTaskDone(pTask->m_dependTask)) //前置任务判断
				&& ( !pDoneTask || (pDoneTask && (pDoneTask->m_repeatedTimes < pTask->m_canRepeatTimes)) ) )//重复次数判断
				return true;
		}
	}
	return false;
}

bool CPlayer::IsTaskDone( int taskId )
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if ( pTask && pTask->m_repeatedTimes > 0 )
		return true;
	return false;
}

CPlayerTask* CPlayer::GetUnDoneTaskById(int taskId)
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if(pTask && pTask->m_doneState == UnDone)
		return pTask;
	return NULL;
}
CPlayerTask* CPlayer::GetUnCommitTaskById(int taskId)
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if(pTask && pTask->m_doneState == UnCommit)
		return pTask;
	return NULL;
}
CPlayerTask* CPlayer::GetDoneTaskById(int taskId)
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if(pTask && pTask->m_doneState == Done)
		return pTask;
	return NULL;
}
CPlayerTask* CPlayer::GetTaskById(int taskId)
{
	return GetTaskByIdFromList(taskId,m_taskList);
}

CPlayerTask* CPlayer::GetTaskByIdFromList( int taskId, PlayerTaskList& tasklist )
{
	PlayerTaskListIter it = tasklist.begin();
	for ( ; it != tasklist.end(); it++ )
	{
		if( taskId == it->m_taskId )
			return &(*it);
	}
	return NULL;
}

void CPlayer::SendTaskList()
{
	CNetMessage msg;
	msg.SetType(GC_SEND_TASKLIST);
	msg << GetUnDoneAndUnCommitTaskNum();//任务数
	list<STaskItem>::iterator i;
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end() ; it++ )
	{//任务id，任务状态，任务名，任务目标，任务类型，任务项数，任务项。。。，任务奖励数，任务奖励项。。。
		if(it->m_doneState == Done)
			continue;
		msg<< it->m_taskId << it->m_doneState << it->m_pTask->m_name <<it->m_pTask->m_target<<it->m_pTask->m_type << it->m_doneItem.size() ;
		for ( i = it->m_doneItem.begin(); i != it->m_doneItem.end(); i++ )
		{
			msg << GetTaskItemName(i->type,i->val) << i->num << i->target << (int)(i->ok() ? SUCCESS_FLAG : ERROR_FLAG) ;
		}
		msg << it->m_pTask->m_awardItem.size();
		for ( i = it->m_pTask->m_awardItem.begin(); i != it->m_pTask->m_awardItem.end(); i++ )
		{
			msg << i->type << i->val << i->num ;
		}
	}
	CGameSocket::instance().SendMsg(sock,msg);
}

void CPlayer::SendTaskListWithNpc(int npcId)
{
	CNpcManager* pNpcManager = NULL;
	CGlobalValManager::CreateInstance()->GetVal(&pNpcManager,EGV_NPC_MANAGER );
	if( pNpcManager == NULL )
		return;
	CNetMessage msg;
	msg.SetType(GC_SEND_TASKLIST);
	msg << GetUnDoneAndUnCommitTaskNum();//任务数
	list<STaskItem>::iterator i;
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end(); it++ )
	{//任务id，任务状态，任务名，任务目标，任务类型，任务项数，任务项。。。，任务奖励数，任务奖励项。。。
		if(it->m_doneState == Done || !pNpcManager->IsTaskBelongToNpc(it->m_taskId,npcId) )
			continue;
		msg<< it->m_taskId << it->m_doneState << it->m_pTask->m_name <<it->m_pTask->m_target<<it->m_pTask->m_type << it->m_doneItem.size() ;
		for ( i = it->m_doneItem.begin(); i != it->m_doneItem.end(); i++ )
		{
			msg << GetTaskItemName(i->type,i->val) << i->num << i->target << (int)(i->ok() ? SUCCESS_FLAG : ERROR_FLAG) ;
		}
		msg << it->m_pTask->m_awardItem.size();
		for ( i = it->m_pTask->m_awardItem.begin(); i != it->m_pTask->m_awardItem.end(); i++ )
		{
			msg << i->type << i->val << i->num ;
		}
	}
	CGameSocket::instance().SendMsg(sock,msg);
}

bool CPlayer::AbandonTask(int taskId)
{
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	if (pDb == NULL)
		return false;
	bool flag = false;
	CPlayerTask* pTask = NULL;
	if ( pTask = GetTaskById(taskId) )
	{
		if ( pTask->m_repeatedTimes == 0 )
		{
			RemoveTask(taskId);
			//数据库操作
			string sql = "delete from player_task where role_id = " + 
				boost::lexical_cast<std::string,int>(roleId) + " and task_id = " + boost::lexical_cast<std::string,int>(taskId) ;
			pDb->Query(sql.c_str());
		}
		else
			pTask->m_doneState = Done;
	}
	return true;
}

string CPlayer::GetTaskItemName(int type,int id)
{
	string name;
	switch ( type )
	{
	case ETIMonster:
		{
			std::map<int,Logic::MonsterAtt_t>::iterator it = server_data::instance().MonsterTypeData_map.find(id);
			if( server_data::instance().MonsterTypeData_map.end() != it )
				name = it->second.m_szMonsterName;
		}
		break;
	case ETIItem:
		{
			std::map<int,Logic::ItemEData_t>::iterator it = server_data::instance().ItemEData_map.find(id);
			if( server_data::instance().ItemEData_map.end() != it )
				name = it->second.m_szName;
		}
		break;
	case ETIExpe:
		{
			name = "经验";
		}
		break;
	case ETIMoney:
		{
			name = "金钱";
		}
		break;
	case ETITalkMan:
		{
			CGameNpc* pNpc = NULL;
			CNpcManager* pNpcManager = NULL;
			CGlobalValManager::CreateInstance()->GetVal(&pNpcManager,EGV_NPC_MANAGER);
			if( pNpcManager && (pNpc = pNpcManager->GetNpcById(id)) )
				name = pNpc->m_name;
		}
		break;
	default:
		break;
	}
	return name;
}

bool CPlayer::IsAcceptTask(int taskId)
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if ( pTask && pTask->m_doneState != Done )
		return true;
	return false;
}

void CPlayer::UnCommitToDone(int taskId)
{
	CPlayerTask* pTask = GetTaskById(taskId);
	if ( pTask )
	{
		pTask->m_doneState = Done;
		pTask->m_repeatedTimes++;
	}
}

int CPlayer::GetUnDoneAndUnCommitTaskNum()
{
	int num = 0;
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end(); it++ )
	{
		if( it->m_doneState != Done )
			num++;
	}
	return num;
}

void CPlayer::RemoveTask(int taskId)
{
	PlayerTaskListIter it = m_taskList.begin();
	for ( ; it != m_taskList.end(); it++ )
	{
		if ( taskId == it->m_taskId )
		{
			m_taskList.erase(it);
			return;
		}
	}
}

int CPlayer::CalcAttDamage(CPlayer* pTarget)
{
	return 0;
}

int CPlayer::CalcAttDamage(monster_ptr pTarget)
{
	return 0;
}