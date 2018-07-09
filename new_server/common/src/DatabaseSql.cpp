#include "stdafx.h"
#include "DatabaseSql.h"
#include <mysql.h>
#include <boost/thread/mutex.hpp>
#include <stack>

int CDatabaseSql::GetRowNum()
{
    return (int)mysql_num_rows(m_result);
}

CDatabaseSql::CDatabaseSql()
{
    m_mysql = new MYSQL;
    m_result = NULL;
    mysql_init(m_mysql);
}

CDatabaseSql::~CDatabaseSql()
{
    if(m_result != NULL)
    {
        mysql_free_result(m_result);
        m_result = NULL;
    }
    mysql_close(m_mysql);
    delete m_mysql;
}

bool CDatabaseSql::Connect(const char *user,const char *passwd,const char *host,const char *database,int port)
{
    unsigned int time_out = 5;
    mysql_options(m_mysql,MYSQL_OPT_CONNECT_TIMEOUT,(char*)&time_out);
    my_bool reconnect = 1;
    mysql_options(m_mysql,MYSQL_OPT_RECONNECT,&reconnect);
    
    if ( !mysql_real_connect(m_mysql, host, user, passwd, NULL, port, NULL,  CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS) )
    {
        return false;
    }
    if( mysql_select_db(m_mysql,database) )
    {
        return false;
    }
	if(!mysql_set_character_set(m_mysql,"gb2312"))
	{
		cout<<GetErrMsg()<<endl;
	}
    return true;
}

const char *CDatabaseSql::GetErrMsg()
{
    return mysql_error(m_mysql);
}

bool CDatabaseSql::Query(const char *sqlSentence)
{
    if(m_result != NULL)
    {
        do
        {
            mysql_free_result(m_result);
        }
        while (mysql_next_result(m_mysql) > 0);
        
        m_result = NULL;
    } 
    
    if (0 == mysql_query(m_mysql,sqlSentence))
    {
        m_result = mysql_store_result(m_mysql);
        return true;
    }
    return false;
}

char **CDatabaseSql::GetRow()
{
    if(m_result == NULL)
        return NULL;
    return mysql_fetch_row(m_result);
}

bool CDatabaseSql::GetResult(FieldsRow &fieldsRow)
{
    if(m_result == NULL)
        return false;

    char **row = mysql_fetch_row(m_result);
    if(row == NULL)
        return false;
    int num = mysql_num_fields(m_result);
    fieldsRow.clear();
    
    MYSQL_FIELD *pField;
    for (int i = 0; i < num; i++)
    {
        pField = mysql_fetch_field_direct(m_result,i);
        if(pField != NULL)
            fieldsRow[pField->name] = row[i];
    }
    return true;
}

class CDbPoolData
{
public:
    CDatabaseSql *PopDbConnect()
    {
        boost::mutex::scoped_lock lk(m_poolMutex);
        if (m_dbStack.size() <= 0)
        {
            return NULL;
        }
        CDatabaseSql *p = m_dbStack.top();
        m_dbStack.pop();
        return p;
    }

    void PushDbConnect(CDatabaseSql *db)
    {
        boost::mutex::scoped_lock lk(m_poolMutex);
        m_dbStack.push(db);
    }

    CDbPool                 *m_pDbPool;
    boost::mutex            m_poolMutex;
    stack<CDatabaseSql*>    m_dbStack;
    string                  m_user;
    string                  m_password;
    string                  m_host;
    string                  m_dbName;
    int                     m_port;
};

CDbPoolData *CDbPool::m_pData = NULL;

void CDbPool::SetDbConfigure(string user,string passwd,string host,string dbname,string port)
{
    if(m_pData != NULL)
    {
        m_pData->m_user = user;
        m_pData->m_password = passwd;
        m_pData->m_host = host;
        m_pData->m_dbName = dbname;
        m_pData->m_port = atoi(port.c_str());
    }
}

CDbPool::CDbPool()
{
    if(m_pData == NULL)
    {
        m_pData = new CDbPoolData;
        m_pData->m_pDbPool = this;
    }
}

CDbPool::~CDbPool()
{
    delete m_pData;
    m_pData = NULL;
}

CDbPool *CDbPool::CreateInstance()
{
    if (m_pData == NULL)
    {
        return new CDbPool;
    }    
    else
    {
        return m_pData->m_pDbPool;
    }
}

bool CDbPool::AddDbConnect()
{
    if (m_pData == NULL)
    {
        return false;
    }
    CDatabaseSql *pDb = new CDatabaseSql;
    if(!pDb->Connect(m_pData->m_user.c_str(),m_pData->m_password.c_str(),m_pData->m_host.c_str(),m_pData->m_dbName.c_str(),m_pData->m_port))
    {
        cout<<pDb->GetErrMsg()<<endl;
        delete pDb;
        return false;
    }
    m_pData->PushDbConnect(pDb);
    return true;
}

CGetDbConnect::~CGetDbConnect()
{
    if((m_pDbConnect != NULL) && CDbPool::m_pData != NULL)
        CDbPool::m_pData->PushDbConnect(m_pDbConnect);
}

CDatabaseSql *CGetDbConnect::GetDbConnect()
{
    if(CDbPool::m_pData == NULL)
        return NULL;
    CDbPoolData *pPoolData = CDbPool::m_pData;    
    CDatabaseSql *pDb = pPoolData->PopDbConnect();
    if (pDb == NULL)
    {
        pPoolData->m_pDbPool->AddDbConnect();
        pDb = pPoolData->PopDbConnect();
    }    
    m_pDbConnect = pDb;
    return pDb;
}