#pragma once
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <stdint.h>

class CBiography :public boost::serialization::singleton<CBiography>
{
public:
	bool Init();
	uint32_t GetAward(uint32_t heroClass, uint32_t step) const;
private:
	struct BioHero
	{
		uint32_t heroClass;
		uint32_t step;
		uint32_t awardId;
	};
	std::unordered_map<uint32_t, BioHero> m_heroInfos;
};