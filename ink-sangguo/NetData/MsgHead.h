#pragma once

#include <stdint.h>

class CMsgHead
{
public:
	CMsgHead()
	{
		memset(m_data, 0, sizeof(m_data));
	}
	void SetType(uint32_t type)
	{
		m_data[0] = type;
	}
	uint32_t GetType()
	{
		return m_data[0];
	}
	void SetSize(uint32_t size)
	{
		m_data[1] = size;
	}
	uint32_t GetSize()
	{
		return m_data[1];
	}
	uint8_t *GetData()
	{
		return (uint8_t*)m_data;
	}
	uint32_t GetHeadSize()
	{
		return sizeof(m_data);
	}
private:
	uint32_t m_data[2];
};