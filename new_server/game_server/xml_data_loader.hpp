
#pragma once
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include "LogicEngine.h"
#include "logicdef.h"
#include "net_msg.h"
#include "server_data_base.h"
#pragma  warning(disable:4996)

class CNetMessage;

#ifdef _DEBUG
#define LOGINENGINE_DLL	_T("LogicEngineND.dll")
#else
#define LOGINENGINE_DLL	_T("LogicEngineN.dll")
#endif

class xml_data_loader
{
public:
	xml_data_loader()
	{

	};  
	~xml_data_loader()
	{

	};
	//virtual void fetch_data(int data_id,CNetMessage &msg) = 0;
public:
	cWorld*	m_pTheWorld;
	cBaseNode*  m_pWorldRoot;
	cLeafNode * pLeafNode;
	HMODULE m_hLogicEngine;
	UINT nRow;

};

class txt_data_loader
{
public:
	txt_data_loader(){};
	~txt_data_loader(){};
	void fetch_data(const char* filename,CNetMessage &msg)
	{
		try
		{
			if (NULL == (pFile = fopen(filename,"rt")))
			{
				std::cout<<"打开"<<filename<<".txt失败"<<std::endl;
			}
			srand((unsigned)time(NULL)); 
			memset(m_buf,0,sizeof(m_buf));
			count = atoi(fgets(m_buf,20,pFile));
			for (int i=0;i<count;i++)		
			{
				fgets(m_buf,20,pFile);
				m_data[i]=(int)atoi(m_buf);
			}
			
		}
		catch (...)
		{
			std::cout<<"读取怪物掉落异常"<<std::endl;
		}
		fclose(pFile);
		num = rand()%count;
		msg<<m_data[num];
		/*memset(m_buf,0,sizeof(m_buf));
		count = atoi(fgets(m_buf,20,pFile));	
		srand(count);
		int i = rand()%count;
		fseek(pFile,i,SEEK_SET);
		fgets(m_buf,20,pFile);
		pack<<(int)atoi(m_buf);	*/
	};
private:
	FILE* pFile;
	char m_buf[20];
	int count;
	int m_data[20];
	int num;
};

template<class LogicType>
class data_table_loader:public xml_data_loader
{
public:
	data_table_loader(){};
	~data_table_loader()
	{
	};
	LogicType* fetch_data(int row,int unit)
	{
		pLeafNode = server_data::instance().m_pTheWorld->GetUnitTemplate(unit);
		int count = pLeafNode->GetRows();
		if (row<count)
		{
			pItemT=(LogicType *)pLeafNode->GetRow(row)->GetBuffer();
			return pItemT;
		}
		return NULL;
	};
private:
	LogicType * pItemT;
};

typedef boost::shared_ptr<txt_data_loader> txt_data_loader_ptr;
