#include "ReadWriteData.h"
#include <iostream>
#include <sstream>
#include "database.h"
using namespace std;

CReadWriteData::CReadWriteData()
{
	m_db = new CDataBase;
}

CReadWriteData::~CReadWriteData()
{
	delete m_db;
	m_db = nullptr;
}

bool CReadWriteData::InitDb(const char *conInfo)
{
	return m_db->Connect(conInfo);
}

void CReadWriteData::GetSelectFormat(DBData &data, FormatData &fmtData)
{
	fmtData.type = SelectFormat;
	stringstream ss;
	ss << "select ";
	for (size_t i = 0; i < data.fields.size(); i++)
	{
		if (i != 0)
			ss << ",";
		ss << data.fields[i];
		fmtData.fieldsId[data.fields[i]] = i;
	}

	ss << " from " << data.tables;
	
	for (size_t i = 0; i < data.where.size(); i++)
	{
		if (i == 0)
			ss << " where ";
		else
			ss << ",";
		if (data.useQuotes.size() > i && !data.useQuotes[i])
			ss << data.where[i]<<"=%"<<i+1<<"%";
		else
			ss << data.where[i] << "='%" << i + 1 << "%'";
	}
	if (!data.other.empty())
		ss << " " << data.other;
	fmtData.fmt = boost::format(ss.str());
}

void CReadWriteData::GetUpdateFormat(DBData &data, FormatData &fmtData)
{
	fmtData.type = UpdateFormat;
	stringstream ss;
	ss << "update " << data.tables << " set ";

	int num = 0;
	for (size_t i = 0; i < data.fields.size(); i++)
	{
		if (i != 0)
			ss << ",";
		num ++;
		ss << data.fields[i] << "=%" << num << "%";
		fmtData.fieldsId[data.fields[i]] = i;
	}

	for (size_t i = 0; i < data.where.size(); i++)
	{
		if (i == 0)
			ss << " where ";
		else
			ss << ",";
		num++;
		if (data.useQuotes.size() > i && !data.useQuotes[i])
			ss << data.where[i] << "=%" << num << "%";
		else
			ss << data.where[i] << "='%" << num << "%'";
	}
	if (!data.other.empty())
		ss << " " << data.other;
	fmtData.fmt = boost::format(ss.str());
}

void CReadWriteData::GetInsertFormat(DBData &data, FormatData &fmtData)
{
	fmtData.type = InsertFormat;

	stringstream ss;
	ss << "insert into " << data.tables << "(";

	for (size_t i = 0; i < data.fields.size(); i++)
	{
		if (i != 0)
			ss << ",";
		ss << data.fields[i];
	}
	ss << ") values (";
	int num = 0;
	for (size_t i = 0; i < data.fields.size(); i++)
	{
		if (i != 0)
			ss << ",";
		num++;
		if (data.useQuotes.size() > i && !data.useQuotes[i])
			ss <<"%" << num << "%";
		else
			ss << "'%" << num << "%'";
		//fmtData.fieldsId[data.fields[i]] = i;
	}
	ss << ")";
	if (!data.other.empty())
		ss << " " << data.other;
	fmtData.fmt = boost::format(ss.str());
}

void CReadWriteData::SetQueryData(DBData & data, QueryType type)
{
	switch (type)
	{
	case CReadWriteData::SelectFormat:
		GetSelectFormat(data, m_curFormat);
		break;
	case CReadWriteData::UpdateFormat:
		GetUpdateFormat(data, m_curFormat);
		break;
	case CReadWriteData::InsertFormat:
		GetInsertFormat(data, m_curFormat);
		break;
	default:
		break;
	}
}

bool CReadWriteData::Query()
{
	return m_db->Query(m_curFormat.fmt.str().c_str());
}

char *CReadWriteData::GetVal(uint32_t tupNum, uint32_t fieldNum)
{
	return m_db->GetVal(tupNum, fieldNum);
}
int CReadWriteData::GetIntVal(uint32_t tupNum, uint32_t fieldNum)
{
	return m_db->GetVal<int>(tupNum, fieldNum);
}

uint32_t CReadWriteData::GetUIntVal(uint32_t tupNum, uint32_t fieldNum)
{
	char *val = GetVal(tupNum, fieldNum);
	if (val == nullptr)
		return 0;
	char *end;
	return strtoul(val, &end, 0);
}

int64_t CReadWriteData::GetInt64Val(uint32_t tupNum, uint32_t fieldNum)
{
	char *val = GetVal(tupNum, fieldNum);
	if (val == nullptr)
		return 0;
	char *end;
	return strtoll(val, &end, 0);
}
uint64_t CReadWriteData::GetUInt64Val(uint32_t tupNum, uint32_t fieldNum)
{
	char *val = GetVal(tupNum, fieldNum);
	if (val == nullptr)
		return 0;
	char *end;
	return strtoull(val, &end, 0);
}

char *CReadWriteData::GetVal(uint32_t fieldNum)
{
	return GetVal(0, fieldNum);
}
int CReadWriteData::GetIntVal(uint32_t fieldNum)
{
	return GetIntVal(0,fieldNum);
}
uint32_t CReadWriteData::GetUIntVal(uint32_t fieldNum)
{
	return GetUIntVal(0,fieldNum);
}
int64_t CReadWriteData::GetInt64Val(uint32_t fieldNum)
{
	return GetInt64Val(0,fieldNum);
}
uint64_t CReadWriteData::GetUInt64Val(uint32_t fieldNum)
{
	return GetUInt64Val(0,fieldNum);
}

char *CReadWriteData::GetVal(uint32_t tupNum, char *fieldName)
{
	auto iter = m_curFormat.fieldsId.find(fieldName);
	if (iter == m_curFormat.fieldsId.end())
		return nullptr;
	return GetVal(tupNum,(*iter).second);
}

int CReadWriteData::GetIntVal(uint32_t tupNum, char *fieldName)
{
	auto iter = m_curFormat.fieldsId.find(fieldName);
	if (iter == m_curFormat.fieldsId.end())
		return 0;
	return GetIntVal(tupNum, (*iter).second);
}

uint32_t CReadWriteData::GetUIntVal(uint32_t tupNum, char *fieldName)
{
	auto iter = m_curFormat.fieldsId.find(fieldName);
	if (iter == m_curFormat.fieldsId.end())
		return 0;
	return GetUIntVal(tupNum, (*iter).second);
}
int64_t CReadWriteData::GetInt64Val(uint32_t tupNum, char *fieldName)
{
	auto iter = m_curFormat.fieldsId.find(fieldName);
	if (iter == m_curFormat.fieldsId.end())
		return 0;
	return GetInt64Val(tupNum, (*iter).second);
}
uint64_t CReadWriteData::GetUInt64Val(uint32_t tupNum, char *fieldName)
{
	auto iter = m_curFormat.fieldsId.find(fieldName);
	if (iter == m_curFormat.fieldsId.end())
		return 0;
	return GetUInt64Val(tupNum, (*iter).second);
}

char *CReadWriteData::GetVal(char *fieldName)
{
	return GetVal(0,fieldName);
}
int CReadWriteData::GetIntVal(char *fieldName)
{
	return GetIntVal(0,fieldName);
}
uint32_t CReadWriteData::GetUIntVal(char *fieldName)
{
	return GetUIntVal(0, fieldName);
}
int64_t CReadWriteData::GetInt64Val(char *fieldName)
{
	return GetInt64Val(0, fieldName);
}
uint64_t CReadWriteData::GetUInt64Val(char *fieldName)
{
	return GetUInt64Val(0, fieldName);
}