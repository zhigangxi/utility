#pragma once

#include "game_map.h"
#include "singleton.h"
#include "game_player.h"

#include <hash_map>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/unordered_map.hpp>

//typedef std::vector<scene_ptr>  scene_vector;
//typedef boost::unordered_map<int,scene_vector> scene_group;

class scene_manager:public singleton<scene_manager>
{
public:

	scene_manager(void);
	~scene_manager(void);
	//void start();
	//void close();
	scene_ptr create_scene(int sceneid);
	int close_scene(int sceneid);
	void AddCommandDeal();
	void ProcessMsg(CNetMessage &msg,int sock);
	void ProcessMonsterMsg(CNetMessage &msg,int sock);
	scene_ptr find_scene(int scene_id)
	{
		if (scene_map_.find(scene_id) != scene_map_.end())
		{
			return scene_map_[scene_id];
		}
		else
		{
			scene_ptr sp;
			return sp;
		}
	}
	//int find_scene_by_town(int town_id)//return scence id;
	//{
	//	for (scene_map::iterator iter = scene_map_.begin();
	//		iter != scene_map_.end();
	//		iter++)
	//	{
	//		if (iter->second->town_->town_id_ == town_id)
	//		{
	//			return iter->second->scene_id_;
	//		}
	//	}
	//	return 0;
	//}
	int jump_scene(CPlayer* player,int from,int to);
	void OnLoop(DWORD dwTime);
private:
	const int create_id();
private:
	boost::mutex			mutex_;
	//scene_group				ready_group_;
	//scene_group				live_group_;
public:
	int scene_type;
	string scene_name;
	string map_name;
	scene_ptr scene_;
	map<int,scene_ptr>	scene_map_;
};
