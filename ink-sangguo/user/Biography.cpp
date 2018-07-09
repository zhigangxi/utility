#include "Biography.h"
#include "ReadWriteUser.h"
#include "utility.h"

bool CBiography::Init()
{
	std::list<CReadData> datas;
	std::vector<const char *> fieldNames;

	fieldNames = { "hero_id","id","award_id" };
	if (!CReadWriteUser::ReadData(datas, fieldNames, "biography_hero"))
		return false;
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		BioHero bio;
		bio.heroClass = i->GetVal<uint32_t>(0);
		bio.step = i->GetVal<uint32_t>(1);
		bio.awardId = i->GetVal<uint32_t>(2);
		m_heroInfos.insert(std::make_pair(CUtility::MakeMask(bio.heroClass,bio.step), bio));
	}
	return true;
}

uint32_t CBiography::GetAward(uint32_t heroClass, uint32_t step) const
{
	auto iter = m_heroInfos.find(CUtility::MakeMask(heroClass, step));
	if (iter == m_heroInfos.end())
		return 0;
	return iter->second.awardId;
}
