#include "stdafx.h"
#include "game_map.h"
#include "scene_manager.h"
#include "despatch_command.h"
#include "server_data_base.h"
#include "protocol.h"

#include "game_socket.h"
#include "global_manager.h"
#include "game_npc.h"
#include <ctime>
#include <utility>


CScene::CScene(int id,int stype,std::string mapname,std::string scenename)
:scene_id_(id),scene_type_(stype),map_name_(mapname),scene_name_(scenename),id_(0)
{

	srand((unsigned)time(NULL)); 
	m_pOnlineUser = COnlineUserList::CreateInstance();

}

CScene::CScene()
{
	srand((unsigned)time(NULL)); 
	m_pOnlineUser = COnlineUserList::CreateInstance();

}

CScene::~CScene(void)
{
}


void CScene::ProcessMsg(CNetMessage &msg,int sock)
{
	int type = msg.GetType();
	switch(type)
	{
	case CG_ROLE_JUMPSENCE:
		PlayerJumpMap(msg,sock);
		break;
	case CG_PLAYER_MOVE:
		PlayerMove(msg,sock);
		break;
	case CG_MONSTER_BATTLE:
		PlayerAttackMonster(msg,sock);
		break;
	case CG_PACK_COLLECT:
		PlayerCollectItem(msg,sock);
	    break;
	case CG_USE_DORP_ITEM:
	    PlayerDropItem(msg,sock);
		break;
	case CG_PLAYER_REVIVE:
		PlayerRevive(msg,sock);
		break;
	/*case :
		break;
	case :
	    break;
	case :
	    break;
	default:
	    break;*/
	}
}

void CScene::PlayerEnterScene(int player_id)						//人物进入场景
{

}
void  CScene::PlayerEnterScene(CPlayer* user)
{
    boost::mutex::scoped_lock lock(mutex_);
	player_map_[user->roleId] = user;
	player_map_[user->roleId]->curSceneId = scene_id_;
	PlayerEnterNotify(user->roleId);
	SendMonsterList(map_name_,user->sock);
}

void CScene::PlayerExitScene(int role_id)					//人物退出场景
{
	PlayerExitNotify(role_id);
    boost::mutex::scoped_lock lock(mutex_);
	if (player_map_.find(role_id)!=player_map_.end())
	{
		player_map_.erase(role_id);
	}	
}

void CScene::PlayerExitNotify(int role_id)
{
	//packet pack(protocol::game::S_PLAYER_EXIT_TOWN_NOTIFY);
	CNetMessage msg; 
	msg.SetType(GC_PLAYER_EXIT_TOWN_NOTIFY);
    boost::mutex::scoped_lock lock(mutex_);
	if (player_map_.find(role_id)!=player_map_.end())
	{
		msg<<role_id<<player_map_[role_id]->name_;
		for (map<int,CPlayer*>::iterator iter = player_map_.begin();
			iter != player_map_.end();
			iter++)
		{
			if (iter->second)
			{
				if (iter->first != role_id)
				{
					//iter->second->post_message(pack);
					//iter->second->packet_notify_info(other_pack);
					//other_pack<<role_id<<player_map_[role_id]->name_<<state<<pos.x_<<pos.y_<<pos.z_<<dir;
					//iter->second->packet_notify_info(other_pack);
					CGameSocket::instance().SendMsg(iter->second->sock,msg);
				}
			}

		}
	}
}


void CScene::PlayerJumpMap(CNetMessage &msg,int sock)
{
	SCEINFO sceinfo;
	UINT roleid;
	vector3d pos;
	UINT npcid;
	UINT sceneid;
	int copyid;
	msg>>roleid>>npcid>>copyid;

	cWorld* proot=server_data::instance().m_pTheWorld;
	cLeafNode *pmap;
	pmap=proot->GetUnitTemplate(Logic::Unit_Scene);

	CPlayer* User = m_pOnlineUser->GetUserBySock(sock);


	ResultRows r = pmap->Query(cQuery(npcid));
	assert(r.nCount);//被合并的行的计数
	{
		unsigned i;
		for(i=0;i<r.nCount;i++)
		{
			const cLeafRow * pRow = pmap->Result(&r,i);
			const Logic::Scene_t * pScnTbl = (const Logic::Scene_t *)pRow->GetBuffer();
			if(pScnTbl->m_nDupliID==copyid)
			{
				sceneid = pScnTbl->m_nSitem;
				pos.x_ = pScnTbl->m_fXCoordinate;
				pos.z_ = pScnTbl->m_fZCoordinate;
				break;
			}
		}
	}
	if (server_data::instance().sceinfo_map.find(sceneid) == server_data::instance().sceinfo_map.end())
		return;
	sceinfo = server_data::instance().sceinfo_map[sceneid];
	scene_manager::instance().jump_scene(User,User->curSceneId,sceneid);

	msg.ReWrite();
	msg.SetType(GC_ROLE_JUMPSENCE);
	msg<<sceinfo.scene_name_<<sceinfo.map_name_<<sceneid<<sceinfo.scene_type_<<pos;
	CGameSocket::instance().SendMsg(sock,msg);
	

    CNpcManager *pNcpMgr;
    CGlobalValManager::CreateInstance()->GetVal(&pNcpMgr,EGV_NPC_MANAGER);
    pNcpMgr->SendNpcList(sceinfo.scene_id_,sock);
}

void CScene::PlayerMove(CNetMessage &msg, int sock)
{
    CNetMessage msg1;
	int state = 0;
	int action = 0;
	int roleid;
	vector3d pos;
	int dir;
	int skill;
	try
	{
		msg>>roleid>>action>>pos.x_>>pos.y_>>pos.z_>>dir>>skill;
	}

	catch (...)
	{
		std::cout<<"cuo le";
	}
	boost::mutex::scoped_lock lock(mutex_);
	for (map<int,CPlayer*>::iterator it = player_map_.begin();
		it != player_map_.end();
		it++)
	{
		if (it->first == roleid)
		{
			it->second->pos_ = pos;
			it->second->dir_ = dir;
			it->second->action_=action;
			//packet ack(protocol::game::S_PLAYER_STATE_ACK);
			msg.ReWrite();
			msg.SetType(GC_PLAYER_MOVE);
			msg<<state;
			//it->second->post_message(ack);	
			CGameSocket::instance().SendMsg(it->second->sock,msg);
		}
	
		else if(player_map_.find(roleid) != player_map_.end())
		{
			try
			{
				if (it->second)
					//if(!(it->second->if_dead))
					{
						//packet ack(protocol::game::S_PLAYER_STATE_NOTIFY);
						msg1.SetType(GC_PLAYER_STATE_NOTIFY);
						msg1<<roleid<<player_map_[roleid]->name_<<action<<pos.x_<<pos.y_<<pos.z_<<dir
							<<skill<<player_map_[roleid]->hp_<<player_map_[roleid]->mp_;
						//it->second->post_message(ack);
						CGameSocket::instance().SendMsg(it->second->sock,msg1);
					}
			}
			catch (...)
			{
				//	
			}
		}
	}
	/*msg.ReWrite();
	msg.SetType(GC_PLAYER_MOVE);
	msg<<0;
	CGameSocket::instance().SendMsg(sock,msg);*/
}

void CScene::PlayerDropItem(CNetMessage &msg,int sock)
{
	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	SItem* item = new SItem();
	msg>>item->id>>item->type_id>>item->num;
	msg.ReWrite();
	msg.SetType(GC_USE_DORP_ITEM);
	int bagid;
	
	if (bagid = player->PlayerLoseItem(*item))
	{
		if ((scene_type_ != 1)&&(scene_type_ != 0))
		{
			drop_item_map[item->id] = item;
			msg<<1;//
			msg<<item->type_id;
			msg<<item->id;
			msg<<(string)server_data::instance().ItemEData_map[item->type_id].m_szDropModel;
			msg<<(string)server_data::instance().ItemEData_map[item->type_id].m_szName;
		}
		else
		{
			msg<<0;	
		}
		CGameSocket::instance().SendMsg(sock,msg);	
	}
	else
	{
		msg<<0;
		CGameSocket::instance().SendMsg(sock,msg);	
	}
	player->SendUpdateBackPackData(bagid);
}


void CScene::PlayerEnterNotify(int role_id)			
{
    if (player_map_.find(role_id)!=player_map_.end())
	{
		CPlayer* ppt= player_map_[role_id];
		//packet self_pack(protocol::game::S_PLAYER_ENTER_TOWN_NOTIFY);
		//packet other_pack(protocol::game::S_TOWN_PLAYER_LIST);
		//ppt->pos_ = town_->start_pos_;
		//to do set player position
		CNetMessage self_msg;
		CNetMessage other_msg;
		self_msg.SetType(GC_PLAYER_ENTERSCENE_NOTIFY);
		other_msg.SetType(GC_SCENE_PLAYER_LIST);

		ppt->PacketPlayerNotifyInfo(self_msg);
		int count = static_cast<int>(player_map_.size())-1;
		other_msg<<count;
		for (map<int,CPlayer*>::iterator iter = player_map_.begin();
			iter != player_map_.end();
			iter++)
		{
			if (iter->first == role_id)
			{
				/*packet ack(protocol::game::S2C_JUMP_SCENE);
				ack<<(short)0;
				iter->second->post_message(ack);*/
			}else{
				if (iter->second)
				{
					//iter->second->(self_msg);
					CGameSocket::instance().SendMsg(iter->second->sock,self_msg);
					iter->second->PacketPlayerNotifyInfo(other_msg);
				}else{
					player_map_.erase(iter);
				}
			}
		}
		if (count)
		{
			//ppt->post_message(other_pack);
			CGameSocket::instance().SendMsg(ppt->sock,other_msg);
		}
	}
}

void CScene::SendMonsterList(string mapname,int sock)					
{
	monster_map* pMap = NULL;
	if ( server_data::instance().MatchMap(mapname,&pMap) )
	{
		int i = static_cast<int>(pMap->size());
		CNetMessage msg;
		CNetMessage msg1;
		msg1.SetType(GC_SENCE_MONTER);
		msg1<<i;
		if (i)
		{
			for (monster_map::iterator it = pMap->begin(); it!=pMap->end(); it++)
			{
				it->second->fill_serverdata_pack(msg);
				it->second->fill_sendclient_pack(msg1);
				monster_ptr mp( new monster() );
				mp->fill_data(msg);
				mp->SetScene(this);
				monster_map_.insert(std::make_pair(mp->monster_id(),mp));
			}
		}
		CGameSocket::instance().SendMsg(sock,msg1);
	}
}
void CScene::PlayerAttackMonster(CNetMessage &msg, int sock)
{
	int monsterid;
	msg>>monsterid;
	msg.ReWrite();
	msg.SetType(GC_MONSTER_BATTLE);
	CNetMessage msg1;

	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	
	if(monster_map_[monsterid] == NULL)
		return;

	monster_map_[monsterid]->m_nMonsterHP-=50;
	if (monster_map_[monsterid]->m_nMonsterHP>0)
	{
		msg<<monsterid<<monster_map_[monsterid]->m_nMonsterHP<<monster_map_[monsterid]->m_nMonsterMP;
		CGameSocket::instance().SendMsg(sock,msg);
		monster_map_[monsterid]->BeAttack(player);
	}
	else
	{
		monster_map_[monsterid]->m_nMonsterHP = 0;
		//monster_map_[monsterid]->SetDead(true);
		msg<<monsterid<<monster_map_[monsterid]->m_nMonsterHP<<monster_map_[monsterid]->m_nMonsterMP;
		CGameSocket::instance().SendMsg(sock,msg);
		__int64 addExp = 0;
		SItem* item = new SItem();
		SItem* moneyitem = new SItem();
		monster_map_[monsterid]->load_drop(msg1);
		msg1>>item->type_id>>addExp>>addMoney;
		//item.id = server_data::instance().CreateID();
		//moneyitem.id = server_data::instance().CreateID();
		
		if ((player->experence+addExp)<player->next_exp)
		{
			player->experence+=addExp;
		}
		else
		{
			player->experence = (player->experence+addExp-player->next_exp);
			player->PlayerUpgrade();
		}
		item = SItem::CreateItem(item);
		moneyitem = SItem::CreateItem(moneyitem);
		msg1.ReWrite();
		msg1.SetType(GC_MONSTER_DEAD);
		msg1<<addExp;
		msg1<<player->experence;
		msg1<<1;/*钱id*/
		msg1<<moneyitem->id;
		msg1<<1;/*掉落物品数量*/
		msg1<<item->type_id;
		msg1<<item->id;
		msg1<<(string)server_data::instance().ItemEData_map[item->type_id].m_szDropModel;
		msg1<<(string)server_data::instance().ItemEData_map[item->type_id].m_szName;

		if (item->type_id!=1)
		{	
			if (item->type_id == 100)
			{
				SowlStone* sl = SowlStone::CreateSowlStone(*item);
				drop_item_map.insert(make_pair(item->id,sl));
			}
			else
			{
				drop_item_map.insert(make_pair(item->id,item));
			}
			
		}
		
		int tempId = monster_map_[monsterid]->m_nMonsterID;
		monster_map_.erase(monsterid);
		pair<int,int> p(moneyitem->id,addMoney);
		drop_money_map.insert(p);
		
		player->UT_KillMonster(tempId);

		CGameSocket::instance().SendMsg(sock,msg1);
	}
	
	
}

void CScene::PlayerCollectItem(CNetMessage &msg, int sock)
{
	SItem item;
 	msg>>item.type_id>>item.id;
	msg.ReWrite();
	msg.SetType(GC_PACK_COLLECT);
	CNetMessage msg1;
	msg1.SetType(GC_PACK_UPDATA);

	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);

	if (item.type_id == 1)
	{
		if(drop_money_map.find(item.id)!=drop_money_map.end())
		{
			player->money_ +=drop_money_map[item.id];
			msg<<item.type_id<<drop_money_map[item.id]<<player->money_;
			CGameSocket::instance().SendMsg(sock,msg);
			drop_money_map.erase(item.id);
		}	
	}
	else
	{
		if(drop_item_map.find(item.id)!=drop_item_map.end())
		{
			int bpackid;
			if (item.type_id == 100)
			{
				player->PlayerAddSowlStone(drop_item_map[item.id]);
			}
			else
			{
				bpackid = player->PlayerAddItem(*drop_item_map[item.id]);
				player->SendUpdateBackPackData(bpackid);
			}
			msg<<item.type_id<<item.num<<1/*物品等级*/<<item.SItemData.m_nSellPric<<item.SItemData.m_eBind
				<<item.SItemData.m_eOccupation<<item.SItemData.m_nLevel<<item.SItemData.m_fWeight<<item.SItemData.m_nEndurance
				<<100/*当前耐久*/<<item.SItemData.m_nMaxPhAttack<<item.SItemData.m_nMinPhAttack<<item.SItemData.m_nMaxMaAttack
				<<item.SItemData.m_nMinMaAttack<<item.SItemData.m_nPhDefend<<item.SItemData.m_nMaDefend<<8/*洞数量*/;

			for (int i =0;i<8;i++)
			{
				msg<<item.enchase[i];
			}
			//msg<<0/*是否强化*/<<item.strengthen/*强化属性*/;
			CGameSocket::instance().SendMsg(sock,msg);
			drop_item_map.erase(item.id);

			
		}
			
	}
}

void CScene::OnLoop(DWORD dwTime)
{
	monster_map::iterator i = monster_map_.begin();
	for ( ; i != monster_map_.end(); i++ )
	{
		if(i->second) i->second->AI_OnLoop(dwTime);
	}
}

list<int> CScene::GetPlayerSocks()
{
	boost::mutex::scoped_lock lock(mutex_);
	list<int> socklist;
	map<int,CPlayer*>::iterator i = player_map_.begin();
	for ( ; i != player_map_.end(); i++ )
	{
		if(i->second) socklist.push_back(i->second->GetSock());
	}
	return socklist;
}

void CScene::PlayerRevive(CNetMessage &msg,int sock)
{
	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	if ( player )
	{
		msg.ReWrite();
		msg.SetType(GC_PLAYER_REVIVE);
		int hp = (int)(player->GetMaxHP()*0.3);
		int mp = (int)(player->GetMaxMp()*0.3);
		player->SetHP(hp);
		player->SetMP(mp);
		msg << hp << mp;
		CGameSocket::instance().SendMsg(sock,msg);
	}
}