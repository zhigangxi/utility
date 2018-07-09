#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED
#include <libpq-fe.h>
#include <stdint.h>
#include <sstream>
#include <boost/lexical_cast.hpp>

class CDataBase
{
public:
    CDataBase()
    {
        m_pCon = NULL;
        m_pResult = NULL;
    }
    ~CDataBase()
    {
        if(m_pResult != NULL)
        {
            PQclear(m_pResult);
            m_pResult = NULL;
        }
        if(m_pCon != NULL)
        {
            PQfinish(m_pCon);
            m_pCon = NULL;
        }
    }
    bool Connect(const char *conInfo);
    bool Query(const char *command);
    
	int GetRowsNum();
    int GetColumnsNum();
    
	char *GetVal(int tupNum,int fieldNum);	
	char *GetVal(int tupNum, const char *fieldName);

	char *GetVal(int fieldNum)
	{
		return GetVal(0, fieldNum);
	}
	char *GetVal(const char *fieldName)
	{
		return GetVal(0, fieldName);
	}

	template<typename Type>
	Type GetVal(int tupNum, int fieldNum)
	{
		char *val = GetVal(tupNum, fieldNum);
		Type t = (Type)0;
		try
		{
			t = boost::lexical_cast<Type>(val);
		}
		catch (...)
		{
			return t;
		}
		return t;
	}

	template<typename Type>
	Type GetVal(int fieldNum)
	{
		return GetVal<Type>(0, fieldNum);
	}

	template<typename Type>
	Type GetVal(int tupNum, const char *fieldName)
	{
		int fieldNum = PQfnumber(m_pResult, fieldName);
		if (fieldNum < 0)
		{
			Type t = (Type)0;
			return t;
		}
		return GetVal<Type>(tupNum, fieldNum);
	}

	template<typename Type>
	Type GetVal(const char *fieldName)
	{
		return GetVal<Type>(0, fieldName);
	}
	
    char *GetErrMsg();//获取查询错误信息

	char *GetFieldName(int fieldNum)
	{
		return PQfname(m_pResult, fieldNum);
	}

private:
    PGconn  *m_pCon;
    PGresult *m_pResult;
};
#endif // DATABASE_H_INCLUDED
