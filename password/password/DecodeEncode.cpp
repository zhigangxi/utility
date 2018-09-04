#include "stdafx.h"
#include "DecodeEncode.h"
#include <iostream>
using namespace std;

DecodeEncode::DecodeEncode()
{
	for (int i = 0; i < _keyLen; i++)
	{
		_key[i] = i * i + (i+1) << 3;
	}
}


DecodeEncode::~DecodeEncode()
{
}

int DecodeEncode::ToBuf(const char *ibuf, char *buf)
{
	unsigned char c, c2;
	unsigned char *obuf = (unsigned char*)buf;
	int len = strlen(ibuf) / 2;

	while (*ibuf != 0)
	{
		c = *ibuf++;
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
			c -= 'A' - 10;

		c2 = *ibuf++;
		if (c2 >= '0' && c2 <= '9')
			c2 -= '0';
		else if (c2 >= 'a' && c2 <= 'f')
			c2 -= 'a' - 10;
		else if (c2 >= 'A' && c2 <= 'F')
			c2 -= 'A' - 10;

		*obuf++ = (c << 4) | c2;
	}

	return len;
}


bool DecodeEncode::ToStr(const char *ibuf, int len, std::string &str)
{
	unsigned char l, h;
	str.resize(len * 2);
	unsigned char *obuf = (unsigned char*)str.c_str();
	while (len != 0)
	{
		h = (*ibuf) / 16;
		l = (*ibuf) % 16;

		if (h < 10)
			*obuf++ = '0' + h;
		else
			*obuf++ = 'a' + h - 10;

		if (l < 10)
			*obuf++ = '0' + l;
		else
			*obuf++ = 'a' + l - 10;

		++ibuf;
		len--;
	}
	return true;
}

void GetKey(char *key)
{
	std::string tmp = key;
	for (int i = 0; i < tmp.length(); i++)
	{
		char c = i;
		for (int j = 0; j < tmp.length(); j++)
		{
			c += tmp.at(j);
			c ^= tmp.at(j);
		}
		key[i] = c;
	}
}

bool DecodeEncode::Init(char *key)
{
	int pos = 0;
	int len = strlen(key);
	GetKey(key);

	for (int i = 0; i < _keyLen; i++)
	{
		_key[i] ^= key[pos];
		pos++;
		if (pos >= len)
			pos = 0;
	}
	return true;
}


bool DecodeEncode::DoDecodeEncode(char * buf, int bufLen)
{
	int pos = 0;
	for (int i = 0; i < bufLen; i++)
	{
		buf[i] ^= _key[pos];
		pos++;
		if (pos >= _keyLen)
			pos = 0;
	}
	return true;
}

