#pragma once
#include <stdint.h>
#include <string>
#include <list>
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <unordered_map>
#include "user.h"

class CGeneralSoulMgr :public boost::serialization::singleton<CGeneralSoulMgr>
{
public:
	bool Init();
	int GetStarByHeroClass(uint32_t heroClass)const;
	bool GeneralSoulExchangeHeroSuccess(uint32_t heroClass, CUser::GeneralSoul & soul)const;
	void doExchangeSoul(CUser*user, uint32_t heroClass)const;
private:
	std::unordered_map<uint32_t, GeneralSoulCost> m_exchangeGeneralSoulCost;
};

