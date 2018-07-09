#pragma once
#include <string>
#include <vector>
#include <boost/format.hpp>
#include <unordered_map>

class CDataBase;

class CReadWriteData
{
public:
	CReadWriteData();
	~CReadWriteData();
	bool InitDb(const char *conInfo);
	
	struct DBData
	{
		std::vector<std::string> fields;
		std::vector<bool> useQuotes;
		std::string tables;
		std::vector<std::string> where;
		std::string other;
	};
	enum QueryType
	{
		SelectFormat, UpdateFormat, InsertFormat
	};
	struct FormatData
	{
		boost::format fmt;
		QueryType type;
		std::unordered_map<std::string, int> fieldsId;
	};

	void SetQueryData(DBData &data, QueryType type);

	bool Query();

	template<typename T>
	CReadWriteData &operator%(const T &t)
	{
		m_curFormat.fmt%t;
		return this;
	}

	char *GetVal(uint32_t tupNum, uint32_t fieldNum);
	int GetIntVal(uint32_t tupNum, uint32_t fieldNum);
	uint32_t GetUIntVal(uint32_t tupNum, uint32_t fieldNum);
	int64_t GetInt64Val(uint32_t tupNum, uint32_t fieldNum);
	uint64_t GetUInt64Val(uint32_t tupNum, uint32_t fieldNum);

	char *GetVal(uint32_t fieldNum);
	int GetIntVal(uint32_t fieldNum);
	uint32_t GetUIntVal(uint32_t fieldNum);
	int64_t GetInt64Val(uint32_t fieldNum);
	uint64_t GetUInt64Val(uint32_t fieldNum);

	char *GetVal(uint32_t tupNum, char *fieldName);
	int GetIntVal(uint32_t tupNum, char *fieldName);
	uint32_t GetUIntVal(uint32_t tupNum, char *fieldName);
	int64_t GetInt64Val(uint32_t tupNum, char *fieldName);
	uint64_t GetUInt64Val(uint32_t tupNum, char *fieldName);

	char *GetVal(char *fieldName);
	int GetIntVal(char *fieldName);
	uint32_t GetUIntVal(char *fieldName);
	int64_t GetInt64Val(char *fieldName);
	uint64_t GetUInt64Val(char *fieldName);

	//INSERT INTO account (name,password) VALUES ('Smith', 'John') RETURNING id;
private:	
	
	void GetSelectFormat(DBData &, FormatData &);
	void GetUpdateFormat(DBData &, FormatData &);
	void GetInsertFormat(DBData &, FormatData &);

	FormatData m_curFormat;
	CDataBase *m_db;
};

