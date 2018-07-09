#include "MailInfo.h"
#include "ReadWriteUser.h"

bool CMailInfo::Init()
{
	std::list<CReadData> datas;
	std::vector<const char*> fieldNames = {"id","title","content","sender","award"};
	if (!CReadWriteUser::ReadData(datas, fieldNames,"system_mail"))
	{
		return false;
	}
	for (auto i = datas.begin(); i != datas.end(); i++)
	{
		MailInfo info;
		info.title = i->GetVal(1);
		info.content = i->GetVal(2);
		info.sender = i->GetVal(3);
		info.awardId = i->GetVal<uint32_t>(4);
		m_infos.insert(std::make_pair(i->GetVal<uint32_t>(0), info));
	}
	return true;
}

const CMailInfo::MailInfo *CMailInfo::GetInfo(uint32_t id) const
{
	auto iter = m_infos.find(id);
	if (iter == m_infos.end())
		return nullptr;
	return &(iter->second);
}
