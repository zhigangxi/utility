#pragma once

#include <string>

class DecodeEncode
{
public:
	DecodeEncode();
	~DecodeEncode();
	bool Init(char *key);
	bool DoDecodeEncode(char *buf, int bufLen);
	bool ToStr(const char *buf,int bufLen, std::string &str);
	int ToBuf(const char *str, char *buf);
private:
	static const int _keyLen = 4096;
	char _key[_keyLen];
};

