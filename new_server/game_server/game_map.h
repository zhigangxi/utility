#pragma once
#pragma warning(disable:4244)
#include "stdafx.h"
#include "monster.h"
#include "game_player.h"
#include "net_msg.h"
#include "logicdef.h"
#include "online_user.h"

#include <hash_map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>



class CScene
{
public:
	CScene(int id,int stype,std::string mapname,std::string scenename);
	CScene();
	~CScene(void);

	void PlayerEnterScene(int player_id);
	void PlayerEnterScene(CPlayer* user);
	void PlayerEnterNotify(int role_id);
	void PlayerExitScene(int role_id);
	void PlayerExitNotify(int role_id);
	void PlayerJumpMap(CNetMessage &msg,int sock);
	void PlayerMove(CNetMessage&msg,int sock);
	void SendMonsterList(string mapname,int sock);
	void PlayerAttackMonster(CNetMessage &msg, int sock);
	void PlayerCollectItem(CNetMessage &msg, int sock);
	void PlayerDropItem(CNetMessage &msg,int sock);
	void PlayerRevive(CNetMessage &msg,int sock);
	void ProcessMsg(CNetMessage &msg,int sock);
	void OnLoop(DWORD dwTime);
	list<int> GetPlayerSocks();
	const int scene_id()
	{
		return scene_id_;
	}
private:
	vector3d    start_pos_;
	map<int,CPlayer*> player_map_;
	map<int,int> drop_money_map;
	int addMoney;
	COnlineUserList *m_pOnlineUser;
	boost::mutex mutex_;

public:
	int id_;
	int scene_id_;
	int scene_type_;
	string scene_name_;
	string map_name_;
	monster_map monster_map_;
	map<int,SItem*> drop_item_map;
	//map<int,Logic::ItemEData_t> drop_item_show_map;


};
typedef boost::shared_ptr<CScene> scene_ptr;
typedef map<int,scene_ptr> scene_map;







