#pragma once
#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include "NetMsg.pb.h"

class CHeroTmpl;
class CUser;

class CBanquet :public boost::serialization::singleton<CBanquet>
{
public:
	bool Init();
	bool Create(CUser *user);
	bool GetPrice(uint32_t id, uint32_t &priceType, uint32_t &price,float &discount) const;
private:
	enum GuestType {
		FIRST_HERO = 0,
		GOVERNOR,
		LEADER,
		FIVE_STAR,
		NUM
	};
	struct BanquetInfo
	{
		uint32_t id;
		uint32_t force;
		uint32_t rank;
		uint32_t heroNum;
		uint32_t priceType;
		uint32_t price;
		float discount;
		uint32_t guestOfHonour;
		std::vector<uint32_t> fiveStarNum;
		std::vector<uint32_t> fiveStarChance;
		uint32_t fourStarChance;
		uint32_t threeStarChance;
		float selfForce;
		uint32_t duraTime;
	};
	struct SelectedHeros
	{
		BanquetInfo *config;
		//待选的英雄
		std::vector<uint32_t> selfFiveStar;//自己势力的
		std::vector<uint32_t> selfFourStar;
		std::vector<uint32_t> selfThreeStar;
		std::vector<uint32_t> otherFiveStar;//其他势力的
		std::vector<uint32_t> otherFourStar;
		std::vector<uint32_t> otherThreeStar;
	};
	void SelectHeros(CUser *user, BanquetInfo *info,std::vector<uint32_t> &heros);
	uint32_t SelFirstHero(CUser *user, SelectedHeros &selHeros);
	void GetSelectedHeros(CUser *user, SelectedHeros &selHeros);
	void DelSelHero(CUser *user, uint32_t heroId, SelectedHeros &selHeros)
	{
		if (!DelSelfHero(user, heroId, selHeros))
		{
			DelOtherHero(user, heroId, selHeros);
		}
	}
	bool DelSelfHero(CUser *user, uint32_t heroId,SelectedHeros &selHeros);
	bool DelOtherHero(CUser *user, uint32_t heroId,SelectedHeros &selHeros);
	uint32_t SelFiveStar(CUser *user, SelectedHeros &selHeros);
	uint32_t SelFourStar(CUser *user, SelectedHeros &selHeros);
	uint32_t SelThreeStar(CUser *user, SelectedHeros &selHeros);

	std::unordered_map<uint32_t, BanquetInfo*> m_banquetInfos;
	std::unordered_map<uint32_t, BanquetInfo*> m_banquetInfosById;
};