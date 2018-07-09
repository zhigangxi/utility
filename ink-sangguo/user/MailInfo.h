#pragma once
#include <boost/serialization/singleton.hpp>
#include <stdint.h>
#include <unordered_map>

class CMailInfo :public boost::serialization::singleton<CMailInfo>
{
public:
	bool Init();
	struct MailInfo
	{
		std::string sender;
		uint32_t awardId;
		std::string title;
		std::string content;
	};
	const MailInfo *GetInfo(uint32_t id) const;
private:
	std::unordered_map<uint32_t, MailInfo> m_infos;
};