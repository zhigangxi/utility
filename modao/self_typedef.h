#ifndef _SELF_TYPEDEF_H_
#define _SELF_TYPEDEF_H_
#include <boost/shared_ptr.hpp>
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned int    uint32;
typedef long long       int64;
typedef unsigned long long uint64;

//#define NO_REG_USER
#ifndef QQ
#define LOST_2_MSG
#endif

#define CHATCH_PET_HUO_DONG 26
const int MAX_INT = 0x7fffffff;

const uint8 MAX_NAME_LEN = 32;

class CUser;
typedef boost::shared_ptr<CUser> ShareUserPtr;

#endif
