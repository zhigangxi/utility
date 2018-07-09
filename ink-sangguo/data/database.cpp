#include "database.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

bool CDataBase::Connect(const char *conInfo)
{
    if(m_pCon != NULL)
    {
        PQfinish(m_pCon);
    }
    m_pCon = PQconnectdb(conInfo);
    if (PQstatus(m_pCon) != CONNECTION_OK)
	{
		cout<< "Connection to database failed: " <<PQerrorMessage(m_pCon);
		PQfinish(m_pCon);
		m_pCon = NULL;
		return false;
	}
	PQsetClientEncoding(m_pCon,"UTF8");
	return true;
}
bool CDataBase::Query(const char *command)
{
    if(m_pCon == NULL)
        return false;

    if(PQstatus(m_pCon) != CONNECTION_OK)
    {
        PQreset(m_pCon);
        if(PQstatus(m_pCon) != CONNECTION_OK)
            return false;
    }
    if(m_pResult != NULL)
    {
        PQclear(m_pResult);
        m_pResult = NULL;
    }
    m_pResult = PQexec(m_pCon,command);
    ExecStatusType state = PQresultStatus(m_pResult);
    if((state == PGRES_COMMAND_OK) || (state == PGRES_TUPLES_OK))
        return true;
    return false;
}

char *CDataBase::GetErrMsg()
{
	if (m_pResult == NULL)
	{
		static char msg[] = "result = null";
		return msg;
	};
    return PQresultErrorMessage(m_pResult);
}

int CDataBase::GetRowsNum()
{
    if(m_pResult == NULL)
        return 0;
    return PQntuples(m_pResult);
}
int CDataBase::GetColumnsNum()
{
    if(m_pResult == NULL)
        return 0;
    return PQnfields(m_pResult);
}
char *CDataBase::GetVal(int row,int column)
{
    if(m_pResult == NULL)
        return NULL;
    if((row >= GetRowsNum()) || (column >= GetColumnsNum()))
        return NULL;
    return PQgetvalue(m_pResult,row,column);
}

char *CDataBase::GetVal(int tupNum, const char *fieldName)
{
	int fieldNum = PQfnumber(m_pResult, fieldName);
	return GetVal(tupNum, fieldNum);
}