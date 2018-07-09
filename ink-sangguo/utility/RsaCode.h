#pragma once


#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include <string>

class CRsaCode
{
public:
	CRsaCode();
	~CRsaCode();
	bool SetPubKey(const char *keyFile);
	bool SetPriKey(const char *keyFile);

	bool Decode(bool usePubKey, std::string in, std::string &out);
private:
	mbedtls_pk_context m_pk;
};


