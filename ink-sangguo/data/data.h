#ifndef _DATA_H_
#define _DATA_H_
#include <stdint.h>

template<uint16_t maxUint8,uint16_t maxUint16,uint16_t maxUint32>
class CData
{
public:
	CData()
	{
		memset(m_uint8Arr,0,sizeof(m_uint8Arr));
		memset(m_uint16Arr,0,sizeof(m_uint16Arr));
		memset(m_uint32Arr,0,sizeof(m_uint32Arr));
	}
	void SetUint8(uint16_t ind,uint8_t val)
	{
		if(ind < MAX_UINT8_SIZE)
		{
			OnSetUint8(ind);
			m_uint8Arr[ind] = val;
		}
	}
	uint8_t GetUint8(uint16_t ind)
	{
		if(ind >= MAX_UINT8_SIZE)
			return 0;
		return m_uint8Arr[ind];
	}

	void SetUint16(uint16_t ind,uint16_t val)
	{
		if(ind < MAX_UINT16_SIZE)
		{
			OnSetUint16(ind);
			m_uint16Arr[ind] = val;
		}
	}
	uint16_t GetUint16(uint16_t ind)
	{
		if(ind >= MAX_UINT16_SIZE)
			return 0;
		return m_uint16Arr[ind];
	}

	void SetUint32(uint16_t ind,uint32_t val)
	{
		if(ind < MAX_UINT32_SIZE)
		{
			OnSetUint32(ind);
			m_uint32Arr[ind] = val;
		}
	}
	uint32_t GetUint32(uint16_t ind)
	{
		if(ind >= MAX_UINT32_SIZE)
			return 0;
		return m_uint32Arr[ind];
	}
protected:
	virtual void OnSetUint8(uint16_t)
	{
	}
	virtual void OnSetUint16(uint16_t)
	{
	}
	virtual void OnSetUint32(uint16_t)
	{
	}
	const static uint16_t MAX_UINT8_SIZE=maxUint8;
	const static uint16_t MAX_UINT16_SIZE=maxUint16;
	const static uint32_t MAX_UINT32_SIZE=maxUint32;
	uint8_t m_uint8Arr[MAX_UINT8_SIZE];
	uint16_t m_uint16Arr[MAX_UINT16_SIZE];
	uint32_t m_uint32Arr[MAX_UINT32_SIZE];
};

#endif

