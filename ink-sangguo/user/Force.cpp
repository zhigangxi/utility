#include "Force.h"
#include "ReadWriteUser.h"
#include "WorldCity.h"

bool CForceMgr::Init()
{
	if (!CWorldCityMgr::get_mutable_instance().Init())
		return false;

	std::list<CForce*> forces;
	CReadWriteUser::ReadForce(forces);

	if(forces.size() <= 0)
		return false;

	for (auto i = forces.begin(); i != forces.end(); i++)
	{
		m_forces.insert(std::make_pair((*i)->GetId(), *i));
	}
	return true;
}

void CForce::AddCity(CWorldCity *city)
{
	if (city != nullptr)
	{
		city->SetForce(m_id);
		m_citys.push_back(city);
	}
}

CForce * CForceMgr::GetForce(uint32_t id) const
{
	auto iter = m_forces.find(id);
	if(iter == m_forces.end())
		return nullptr;
	return iter->second;
}
