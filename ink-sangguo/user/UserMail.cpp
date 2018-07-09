#include "UserMail.h"
#include "ReadWriteUser.h"

void CUserMail::WriteToDb()
{
	CReadWriteUser::WriteMail(this);
}

void CUserMail::UpdateIsRead()
{
	SetIsRead(true);
	CReadWriteUser::UpdateMailIsRead(m_id);
}

void CUserMail::UpdateGiveAward()
{
	SetIsReceived(true);
	CReadWriteUser::UpdateMailGiveAward(m_id);
}
