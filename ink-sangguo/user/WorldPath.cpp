#include "WorldPath.h"
#include "ReadWriteUser.h"

bool CWorldPathMgr::Init()
{
	std::list<CWorldPath*> paths;
	CReadWriteUser::ReadWorldPath(paths);
	if (paths.size() <= 0)
		return false;
	for (auto i = paths.begin(); i != paths.end(); i++)
	{
		m_paths.push_back(*i);
	}

	for (auto i = paths.begin(); i != paths.end(); i++)
	{
		uint32_t ind = GetInd((*i)->GetStartPoint(), (*i)->GetEndPoint());
		m_worldPaths.insert(std::make_pair(ind, *i));
	}
	return true;
}

CWorldPath * CWorldPathMgr::GetPath(uint32_t startPoint, uint32_t endPoint) const
{
	uint32_t ind = GetInd(startPoint, endPoint);
	auto iter = m_worldPaths.find(ind);
	if (iter == m_worldPaths.end())
		return nullptr;

	return iter->second;
}

void CWorldPathMgr::GetPaths(uint32_t point, std::vector<uint32_t>& paths) const
{
	for (auto i = m_paths.begin(); i != m_paths.end(); i++)
	{
		CWorldPath *p = *i;
		if (p->GetCanHaveMonster() && (p->GetStartPoint() == point || p->GetEndPoint() == point))
			paths.push_back(p->GetId());
	}
}

void CWorldPathMgr::GetPaths(std::vector<uint32_t>& paths) const
{
	for (auto i = m_paths.begin(); i != m_paths.end(); i++)
	{
		CWorldPath *p = *i;
		if(p->GetCanHaveMonster())
			paths.push_back(p->GetId());
	}
}
