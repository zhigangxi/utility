#pragma once
#include <string>
#include <unordered_map>
#include <boost/any.hpp>

class CGlobalVal
{
public:
	template<typename Type>
	static Type *GetData(std::string name)
	{
		auto iter = m_datas.find(name);
		if (iter == m_datas.end())
			return nullptr;

		return boost::any_cast<Type*>((*iter).second);
	}

	template<typename Type>
	static void SetData(std::string name, Type *data)
	{
		m_datas.insert(std::make_pair(name, data));
	}
private:
	static std::unordered_map<std::string, boost::any> m_datas;
};
