
#ifndef _DATABASE_H_
#define _DATABASE_H_
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <map>
#include <stdlib.h>
#include "self_typedef.h"
struct st_mysql;
struct st_mysql_res;
class CDbPool;
class CDbPoolData;
class CGetDbConnect;
using namespace std;

typedef map<string,const char*> FieldsRow;

class CDatabaseSql  
{
public:
	bool		Query(const char *sqlSentence);
	char	    **GetRow();
    bool        GetResult(FieldsRow&);

	const char	*GetErrMsg();	
	int         GetRowNum();
    CDatabaseSql();
    ~CDatabaseSql();
    bool		Connect(const char *user,const char *passwd,const char *host,const char *database,int port);
    uint32      InsertId();

private:
    CDatabaseSql(const CDatabaseSql &){};
	st_mysql_res        *m_result;
	st_mysql	        *m_mysql;
};

class CGetDbConnect
{
public:
    CGetDbConnect():m_pDbConnect(NULL)
    {
    }
    ~CGetDbConnect();
    CDatabaseSql    *GetDbConnect();
private:
    void *operator new(size_t t){return malloc(t);};
    CDatabaseSql *m_pDbConnect;
};

class CDbPool
{
public:
    void            SetDbConfigure(string user,string passwd,string host,string dbname,string port);
    bool            AddDbConnect();
    static CDbPool  *CreateInstance();
private:
    CDbPool();
    ~CDbPool();
    static CDbPoolData *m_pData;
    friend class CGetDbConnect;
};

#endif
