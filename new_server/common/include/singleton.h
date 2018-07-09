#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <assert.h>

#pragma warning(disable:4312)

template <typename T>  
class singleton
{
	static T* ms_singleton;

public:
	singleton(void)
	{
		assert(!ms_singleton);
		int offset = (int)reinterpret_cast<__int64>((T*)1) - (int)reinterpret_cast<__int64>((singleton <T>*)(T*)1);
		ms_singleton = (T*)((int)reinterpret_cast<__int64>(this) + offset); //vio portable warning
	}

	~singleton(void)
	{
		assert(ms_singleton);
		ms_singleton = 0;
	}

	static T& instance(void)
	{
		assert(ms_singleton); 
		return (*ms_singleton);
	}

	static T* instance_ptr(void)
	{
		assert(ms_singleton);
		return ms_singleton;
	}
};

template <typename T> T* singleton <T>::ms_singleton = 0;

#pragma warning(default:4312)

#endif
