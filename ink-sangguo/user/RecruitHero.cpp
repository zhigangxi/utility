#include "RecruitHero.h"
#include "ReadWriteUser.h"
#include <list>
using namespace std;

bool CRecruitHero::Init()
{
	list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "id","hero","contribution","city","price_type","price" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "recruit","where in_use=1"))
		return false;

	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		RecruitInfo info;
		info.id = i->GetVal<uint32_t>(0);
		info.heroId = i->GetVal<uint32_t>(1);
		info.contribution = i->GetVal<uint32_t>(2);
		info.city = i->GetVal<uint32_t>(3);
		info.priceType = i->GetVal<uint32_t>(4);
		info.price = i->GetVal<uint32_t>(5);
		m_recruitInfos.insert(make_pair(info.id, info));
	}
	return true;
}

const RecruitInfo * CRecruitHero::GetRecruitInfo(uint32_t id) const
{
	auto iter = m_recruitInfos.find(id);
	if (iter != m_recruitInfos.end())
		return &(iter->second);
	return nullptr;
}
