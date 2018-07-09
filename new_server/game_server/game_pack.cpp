#include "stdafx.h"
//#include "game_pack.h"
//#include "game_map.h"
//#include "despatch_command.h"
//#include <boost/bind.hpp>
//#include "protocol.h"
//#include "DatabaseSql.h"
//#include "net_msg.h"
//#include "game_socket.h"
//#include "game_player.h"
//#include "server_data_base.hpp"
//#include "vector3d.hpp"
//#include "scene_manager.h"
//#include <boost/lexical_cast.hpp>
//
//
//void CGamePackDeal::AddCommandDeal(CDespatchCommand *pDespatch)
//{
//	SCommand comDeal[] = {
//		CG_ENTER_GAME,boost::bind(&CGamePackDeal::PlayerEnterGame,this,_1,_2),
//		CG_PLAYER_MOVE,boost::bind(&CGamePackDeal::PlayerMove,this,_1,_2),
//	};
//	pDespatch->AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));
//}
//
//void CGamePackDeal::PlayerEnterGame(CNetMessage &msg,int sock)
//{
//	int roleId = 0;
//	int respond = 0;
//	msg>>roleId;
//	msg.ReWrite();
//	msg.SetType(GC_ENTER_GAME);
//	CGetDbConnect getDb;
//	CDatabaseSql *pDb = getDb.GetDbConnect();
//	CPlayer* player = m_pOnlineUser->GetUserBySock(sock);
//	if (pDb == NULL)
//		return;
//	string sqlStr = "select rolename,nickname,experence,requireEXP,race,occupation,level,power,quick,intelligence,energy,\
//		maxhp,maxmp,phyattack,maattack,phydefend,madefend,attackspeed,hitrate,movespeed,evadePro,releasePro,MPRevert,HPRevert,\
//		CarryWeight,RigidTime,resistance,FrozenRessis,BurnRessis,PalsyRessis,ToxinRessis,VertigoRessis,skillRessis,MARessis,\
//		weakRessis,reputation,AttriPoint,SkillPoint,AttackStorm,lucky,money,gold,hairColor,glass,neckchain,cape,belt,mask,\
//		wig,headwear,shoulderpad,armor,armguard,legguard,shoes,roleplace,head,weapon from game_user_role_info where roleid = " 
//		+ boost::lexical_cast<std::string,int>(roleId);
//	char **row;
//	if (!pDb->Query(sqlStr.c_str()))
//	{
//		cout<<sqlStr<<endl;
//		return;
//	}
//	row = pDb->GetRow();
//	if(row == NULL)
//		return;
//	msg<<respond<<(string)row[0]<<(string)row[1]<<(__int64)atoi(row[2])<<(__int64)atoi(row[3])<<atoi(row[4])<<atoi(row[5])
//		<<atoi(row[6])<<atoi(row[7])<<atoi(row[8])<<atoi(row[9])<<atoi(row[10])<<atoi(row[11])<<atoi(row[12])<<atoi(row[13])
//		<<atoi(row[14])<<atoi(row[15])<<atoi(row[16])<<atoi(row[17])<<atoi(row[18])<<atoi(row[19])<<atoi(row[20])<<atoi(row[21])
//		<<atoi(row[22])<<atoi(row[23])<<atoi(row[24])<<atoi(row[25])<<atoi(row[26])<<atoi(row[27])<<atoi(row[28])<<atoi(row[29])
//		<<atoi(row[30])<<atoi(row[31])<<atoi(row[32])<<atoi(row[33])<<atoi(row[34])<<atoi(row[35])<<atoi(row[36])<<atoi(row[37])
//		<<atoi(row[38])<<atoi(row[39])<<atoi(row[40])<<atoi(row[41])<<atoi(row[42])<<atoi(row[43])<<atoi(row[44])<<atoi(row[45])
//		<<atoi(row[46])<<atoi(row[47])<<atoi(row[48])<<atoi(row[49])<<atoi(row[50])<<atoi(row[51])<<atoi(row[52])<<atoi(row[53])
//		<<atoi(row[54])<<atoi(row[55])<<atoi(row[56])<<atoi(row[57]);
//
//	m_pSocket->SendMsg(sock,msg);
//	
//		player->roleId = roleId;
//		player->name_ = (string)row[0];
//		player->nick_ = (string)row[1];
//		player->experence = (__int64)atoi(row[2]);
//		player->next_exp = (__int64)atoi(row[3]);
//		player->race_ = atoi(row[4]);
//		player->occupation_ = atoi(row[5]);
//		player->level_ = atoi(row[6]);
//		player->power_ = atoi(row[7]);
//		player->quick_ = atoi(row[8]);
//		player->intelligence_ = atoi(row[9]);
//		player->energy_ = atoi(row[10]);
//		player->maxhp_ = atoi(row[11]);
//		player->maxmp_ = atoi(row[12]);
//		player->phyattack_ = atoi(row[13]);
//		player->maattack_ = atoi(row[14]);
//		player->phydefend_ = atoi(row[15]);
//		player->madefend_ = atoi(row[16]);
//		player->attackspeed_ = atoi(row[17]);
//		player->hitrate_ = atoi(row[18]);
//		player->movespeed_ = atoi(row[19]);
//		player->evadePro_ = atoi(row[20]);
//		player->releasePro_ = atoi(row[21]);
//		player->MPRevert_ = atoi(row[22]);
//		player->HPRevert_ = atoi(row[23]);
//		player->CarryWeight_ = atoi(row[24]);
//		player->RigidTime_ = atoi(row[25]);
//		player->resistance_ = atoi(row[26]);
//		player->FrozenRessis_ = atoi(row[27]);
//		player->BurnRessis_ = atoi(row[28]);
//		player->PalsyRessis_ = atoi(row[29]);
//		player->ToxinRessis_ = atoi(row[30]);
//		player->VertigoRessis_ = atoi(row[31]);
//		player->skillRessis_ = atoi(row[32]);
//		player->MARessis_ = atoi(row[33]);
//		player->weakRessis_ = atoi(row[34]);
//		player->reputation_ = atoi(row[35]);
//		player->AttriPoint_ = atoi(row[36]);
//		player->SkillPoint_ = atoi(row[37]);
//		player->AttackStorm_ = atoi(row[38]);
//		player->lucky_ = atoi(row[39]);
//		player->money_ = atoi(row[40]);
//		player->gold_ = atoi(row[41]);
//		player->hairColor = atoi(row[42]);
//		player->glass = atoi(row[43]);
//		player->neckchain = atoi(row[44]);
//		player->cape = atoi(row[45]);
//		player->belt = atoi(row[46]);
//		player->mask = atoi(row[47]);
//		player->wig = atoi(row[48]);
//		player->headwear = atoi(row[49]);
//		player->shoulderpad = atoi(row[50]);
//		player->armor = atoi(row[51]);
//		player->armguard = atoi(row[52]);
//		player->legguard = atoi(row[53]);
//		player->shoes = atoi(row[54]);
//		player->roleplace = atoi(row[55]);
//		player->head = atoi(row[56]);
//		player->weapon = atoi(row[57]);
//
//	string sqlStr1 = "select equippackid,expendpackid,materialpackid,taskpackid,decorpackid from \
//		backpack_base_info where roleid = " + boost::lexical_cast<std::string,int>(roleId);
//
//	if (!pDb->Query(sqlStr1.c_str()))
//	{
//		cout<<sqlStr1<<endl;
//		return;
//	}
//	row = pDb->GetRow();
//	if(row == NULL)
//		return;
//	player->equippack_id = atoi(row[0]);
//	player->expendpack_id = atoi(row[1]);
//	player->materialpack_id = atoi(row[2]);
//	player->taskpack_id = atoi(row[3]);
//	player->decorpack_id = atoi(row[4]);
//
//	msg.ReWrite();
//	msg.SetType(GC_BACKPACK_BASE_INFO);
//	msg<<atoi(row[0])<<atoi(row[1])<<atoi(row[2])<<atoi(row[3])<<atoi(row[4]);
//	m_pSocket->SendMsg(sock,msg);
//
//	string sqlStr2 = "select * from backpack_item_info where packid =%d or packid=%d or packid=%d or packid=%d or packid=%d" 
//		+ boost::lexical_cast<std::string,int>(roleId);
//
//	if (!pDb->Query(sqlStr2.c_str()))
//	{
//		cout<<sqlStr<<endl;
//		return;
//	}
//	row = pDb->GetRow();
//	if(row == NULL)
//		return;
//
//	msg.ReWrite();
//	msg.SetType(GC_BACKPACK_ITEM_INFO);
//
//	UINT i = 0;
//	while (row[i]!=NULL)
//	{
//		msg<<row[i];
//		i++;
//	}
//	m_pSocket->SendMsg(sock,msg);
//
//	
//	scene_ptr scene = scene_manager::instance().create_scene(1);
//	scene->PlayerEnterScene(player);
//	
//}
//
//void CGamePackDeal::PlayerMove(CNetMessage &msg,int sock)
//{
//	msg.ReWrite();
//	msg.SetType(GC_PLAYER_MOVE);
//	msg<<(int)0;
//	m_pSocket->SendMsg(sock,msg);
//}
//
