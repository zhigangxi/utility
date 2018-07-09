#pragma once

#include <hash_map>
#include <boost/shared_ptr.hpp>
class CPlayer
{
public:
	int userId;
	int roleId;
	int sock;
	int model_id_;
	int action_;
//	vector3d pos_;
	int dir_;
	std::string name_;
	std::string nick_;
	__int64    experence;
	__int64    next_exp;
	int race_;
	int occupation_;
	int level_;
	int power_;
	int quick_;
	int intelligence_;
	int energy_;
	int maxhp_;
	int maxmp_;
	int phyattack_;
	int maattack_;
	int phydefend_;
	int madefend_;
	int attackspeed_;
	int hitrate_;
	int movespeed_;
	int evadePro_;
	int releasePro_;
	int MPRevert_;
	int HPRevert_;
	int CarryWeight_;
	int RigidTime_;
	int resistance_;
	int FrozenRessis_;
	int BurnRessis_;
	int PalsyRessis_;
	int ToxinRessis_;
	int VertigoRessis_;
	int skillRessis_;
	int MARessis_;
	int weakRessis_;
	int reputation_;
	int AttriPoint_;
	int SkillPoint_;
	int AttackStorm_;
	int lucky_;
	int money_;
	int gold_;
	int hairColor;
	int	glass;
	int neckchain;
	int cape;
	int belt;
	int mask;
	int wig;
	int headwear;
	int shoulderpad;
	int armor;
	int armguard;
	int legguard;
	int shoes;
	int roleplace;
	int head;
	int weapon;
	int hp_;
	int mp_;

	int equippack_id;
	int expendpack_id;
	int materialpack_id;
	int taskpack_id;
	int decorpack_id;
	int backpack_weight;

	int equip_data[52];
	int expend_data[52];
	int material_data[52];
	int task_data[52];
	int decor_data[52];

	int curSceneId;
	CPlayer():userId(0),roleId(0),sock(0)
	{
	}

};

typedef hash_map<int,CPlayer*>::iterator HashIter;
typedef boost::shared_ptr<CPlayer> player_ptr;