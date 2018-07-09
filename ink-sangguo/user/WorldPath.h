#pragma once
#include "macros.h"
#include <stdint.h>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <list>
#include <vector>

class CWorldPath
{
public:
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_startPoint, StartPoint);
	DEFINE_PROPERTY(uint32_t, m_endPoint, EndPoint);
	DEFINE_PROPERTY(uint32_t, m_length, Length);
	DEFINE_PROPERTY(uint32_t, m_weight, Weight);
	DEFINE_PROPERTY(bool, m_inUse, InUse);
	DEFINE_PROPERTY(bool, m_canHaveMonster, CanHaveMonster);
private:
};

class CWorldPathMgr :public boost::serialization::singleton<CWorldPathMgr>
{
public:
	bool Init();
	CWorldPath *GetPath(uint32_t startPoint, uint32_t endPoint) const;
	void GetPaths(uint32_t point, std::vector<uint32_t> &paths) const;
	void GetPaths(std::vector<uint32_t> &paths) const;
private:
	uint32_t GetInd(uint32_t starPoint, uint32_t endPoint) const
	{
		if (starPoint > endPoint)
			std::swap(starPoint, endPoint);
		return (starPoint << 16) | endPoint;
	}
	std::unordered_map<uint32_t, CWorldPath*> m_worldPaths;
	std::vector<CWorldPath*> m_paths;
};
