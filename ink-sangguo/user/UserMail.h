#pragma once
#include "macros.h"
#include <stdint.h>
#include <string>
#include "NetMsg.pb.h"

class CUserMail
{
public:
	CUserMail() :m_id(0), m_isRead(false),m_delMail(false), m_isReceived(false)
	{

	}
	DEFINE_PROPERTY(uint32_t, m_id, Id);
	DEFINE_PROPERTY(uint32_t, m_fromId, FromId);
	DEFINE_PROPERTY_REF(std::string, m_fromName, FromName);
	DEFINE_PROPERTY(uint32_t, m_fromIcon, FromIcon);
	DEFINE_PROPERTY(uint32_t, m_fromTitle, FromTitle);
	DEFINE_PROPERTY(uint32_t, m_toId, ToId);
	DEFINE_PROPERTY_REF(std::string, m_title, Title);
	DEFINE_PROPERTY_REF(std::string, m_content, Content);
	DEFINE_PROPERTY_REF(NetMsg::Award, m_award, Award);
	DEFINE_PROPERTY(time_t, m_sendTime, SendTime);
	DEFINE_PROPERTY(bool, m_isRead, IsRead);
	DEFINE_PROPERTY(bool, m_delMail, DelMail);
	DEFINE_PROPERTY(bool, m_isReceived,IsReceived);
	DEFINE_PROPERTY(uint32_t, m_type, Type);

	void WriteToDb();
	void UpdateIsRead();
	void UpdateGiveAward();
};