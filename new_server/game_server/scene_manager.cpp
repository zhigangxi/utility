#include "stdafx.h"
#include "scene_manager.h"
#include "game_player.h"
#include "despatch_command.h"
#include "server_data_base.h"
#include "protocol.h"
#include <boost/bind.hpp>
scene_manager::scene_manager(void)
{

}

scene_manager::~scene_manager(void)
{
}
scene_ptr scene_manager::create_scene(int sceneid)
{
	if (server_data::instance().sceinfo_map.find(sceneid) != server_data::instance().sceinfo_map.end())
	{
		scene_type = server_data::instance().sceinfo_map[sceneid].scene_type_;
		scene_name = server_data::instance().sceinfo_map[sceneid].scene_name_;
		map_name = server_data::instance().sceinfo_map[sceneid].map_name_;
		/*if(scene_type == 0)
			if ((scene_ = find_scene(homeid))!=NULL)
			{
				return scene_;
			}*/
			if (scene_type == 1)
			{
				if ((scene_ = find_scene(sceneid))!=NULL)
				{
					return scene_;
				}
			}

			if (scene_type == 3)
			{

				if ((scene_ = find_scene(sceneid))!=NULL)
				{
					return scene_;
				}
				int id_ = create_id();
				scene_ptr sp(new CScene(id_,scene_type,map_name,scene_name));
				
			//	sp->AddCommandDeal();
			//	boost::mutex::scoped_lock lock(mutex_);
				scene_map_.insert(make_pair(sceneid,sp));

				return sp;
			}

			int id_ = create_id();
			scene_ptr sp(new CScene(id_,scene_type,map_name,scene_name));
			//sp->AddCommandDeal();
			//boost::mutex::scoped_lock lock(mutex_);
			scene_map_.insert(make_pair(id_,sp));

			return sp;
	}
	return scene_;
}

void scene_manager::AddCommandDeal()
{
	SCommand comDeal[] = {
		CG_ROLE_JUMPSENCE,boost::bind(&scene_manager::ProcessMsg,this,_1,_2),
		CG_PLAYER_MOVE,boost::bind(&scene_manager::ProcessMsg,this,_1,_2),
		CG_MONSTER_BATTLE,boost::bind(&scene_manager::ProcessMsg,this,_1,_2),
		CG_PACK_COLLECT,boost::bind(&scene_manager::ProcessMsg,this,_1,_2),
		CG_USE_DORP_ITEM,boost::bind(&scene_manager::ProcessMsg,this,_1,_2),
		CG_PLAYER_REVIVE,boost::bind(&scene_manager::ProcessMsg,this,_1,_2)
	};
	CDespatchCommand::instance().AddCommandDeal(comDeal,sizeof(comDeal)/sizeof(SCommand));
}

void scene_manager::ProcessMsg(CNetMessage &msg,int sock)
{
	int type = msg.GetType();
	CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	if (player == NULL)
		return;
	if (player->curSceneId>0)
	{
		if (scene_map_[player->curSceneId] == NULL)
			return;
		scene_map_[player->curSceneId]->ProcessMsg(msg,sock);
	}
	
}

void scene_manager::ProcessMonsterMsg(CNetMessage &msg, int sock)
{
	/*CPlayer* player = COnlineUserList::instance().GetUserBySock(sock);
	if (player!=NULL)
	{
		if (player->curSceneId>0)
		{
			if ((scene_map_[player->curSceneId]->monster_map_[monsterid])!=NULL)
			{
				scene_map_[player->curSceneId]->monster_map_[monsterid]->ProcessMsg(msg,sock);
			}	
		}
	}*/
}


int scene_manager::jump_scene(CPlayer* player,int from,int to)
{
	scene_ptr from_scene = find_scene(from);
	scene_ptr to_scene   = create_scene(to);
	boost::mutex::scoped_lock lock(mutex_);
	from_scene->PlayerExitScene(player->roleId);
	to_scene->PlayerEnterScene(player);
	return to_scene->scene_id();
	//return 0;
}
int scene_manager::close_scene(int sceneid)
{
	if (scene_map_.find(sceneid) != scene_map_.end())
	{
		//scene_map_[sceneid]->close();
		scene_map_.erase(sceneid);
	}
	return 1;
}
const int scene_manager::create_id()
{
	boost::mutex::scoped_lock lock(mutex_);
	static int id_ = 11;
	do 
	{
		id_++;
		if (id_ > 0x7FFFFFFE)
		{
			id_ = 1;
		}
	} while(scene_map_.end() != scene_map_.find(id_));
	return id_;
}

//void scene_manager::start()
//{}
//void scene_manager::close()
//{}

void scene_manager::OnLoop(DWORD dwTime)
{
	map<int,scene_ptr>::iterator i = scene_map_.begin();
	for (; i != scene_map_.end(); i++)
	{
		if(i->second) i->second->OnLoop(dwTime);
	}
}