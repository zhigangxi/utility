#include "RsaCode.h"

CRsaCode::CRsaCode()
{
	mbedtls_pk_init(&m_pk);
}

CRsaCode::~CRsaCode()
{
	mbedtls_pk_free(&m_pk);
}

bool CRsaCode::SetPubKey(const char * keyFile)
{
	return 0 == mbedtls_pk_parse_public_keyfile(&m_pk, keyFile);
}

bool CRsaCode::SetPriKey(const char * keyFile)
{
	return false;
}

bool CRsaCode::Decode(bool usePubKey, std::string in, std::string & out)
{
	mbedtls_rsa_context *rsaContext = mbedtls_pk_rsa(m_pk);
	if (rsaContext == nullptr)
		return false;

	std::string base64Decode;
	size_t len = in.size();
	base64Decode.resize(len);
	int ret = mbedtls_base64_decode((unsigned char*)base64Decode.c_str(), len, &len, (unsigned char*)in.c_str(), len);

	if (ret != 0)
		return false;

	int mod = MBEDTLS_RSA_PUBLIC;
	if (!usePubKey)
		mod = MBEDTLS_RSA_PRIVATE;

	base64Decode.resize(len);
	out.resize(2 * len);
	return 0 == mbedtls_rsa_pkcs1_decrypt(rsaContext,nullptr,nullptr,mod, &len,(unsigned char*)base64Decode.c_str(),(unsigned char*)out.c_str(), 2*len);
}