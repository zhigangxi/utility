#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <iconv.h>
#include <string>
#include "self_typedef.h"

#define ISSPACE(c) isspace((unsigned char)(c))

using namespace std;

/*
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif
*/

class CUser;
class CNetMessage;
class CDatabaseSql;
class CCallScript;
class CScene;
struct SItemInstance;

int StrToHex(const char *str,uint8 *pHex,int hexLen);
void HexToStr(uint8 *pHex,int hexLen,string &str);
int SplitLine(char **templa, int templatecount, char *pkt);
int split_line (char **tem,int temcount, char *pkt);
int Random(int min,int max);
void GetTime(time_t t,uint8 &month,uint8 &day,uint8 &hour,uint8 &min);

uint64 GetTime();

void SendSysInfo(CUser*,const char *info);
void SendSysInfo1(CUser*,const char *info);

void SendUserPos(CUser*);
void SendPopMsg(CUser *pUser,const char *info);
void SendSysChannelMsg(CUser *pUser,const char *info);

void ReadMonster();
void ReadItem();

void InitMission();
CCallScript *GetScript();

char *GetMissionName(uint32 id);

uint32 GetLevelUpExp(uint8 level);

bool MakeItemInfo(SItemInstance *item,CNetMessage &msg);

void SetSysTime(time_t t);
time_t GetSysTime();

void ReadHuoDongDrop(CDatabaseSql*);

/***************
    TYPE=1 状态
   TYPE=2 属性点
   TYPE=3 武学
   TYPE=4 经验
   TYPE=5 气血
   TYPE=6 法力
   TYPE=7 体质
   TYPE=8 力量
   TYPE=9 敏捷
   TYPE=10 灵力
   TYPE=11 耐力
   TYPE=12 寿命
   TYPE=13 忠诚度
   TYPE=14 亲密度
    ****************/
void UpdatePetInfo(uint8 petPos,uint8 type,int val);

void MD5String(string &str);

bool UserCanGetPet(CUser *pUser,uint16 petId);
const char *GetScriptDir();

//产生物品个人店铺物品id，每次+1
uint32 GetShopItemId();

template<typename Type>
inline Type RandSelect(Type *arr,int num)
{
    return arr[Random(0,num-1)];
}

template<typename Type>
inline Type CalculateRate(Type src,Type numerator,Type denominator)
{
    double temp = numerator;
    temp /= denominator;
    return (Type)(src * temp);
}

template<typename Bit>
inline void BitsetToHex(Bit &bit,uint8 *hex)
{
    for (int i = 0; i < (int)bit.size(); i++)
    {
        if(bit.test(i))
            hex[i/8] |= 1<<(i%8);
    }
}

template<typename Bit>
inline void HexToBitset(uint8 *hex,Bit &bit)
{
    for(int i = 0; i < (int)bit.size(); i++)
    {
        if(hex[i/8] & (1<<(i%8)))
            bit.set(i);
    }
}

template<typename Type>
inline void HexToStr(Type &data,string &toStr)
{
    HexToStr((uint8*)&data,sizeof(data),toStr);
}

void SaveTrade(uint32 user1,int money1,string &item1,string &pet1,
               uint32 user2,int money2,string &item2,string pet2);

void SaveUserShopItem(uint32 buyer,uint32 seller,int money,string &item);
void SaveUserShopPet(uint32 buyer,uint32 seller,int money,string &pet);
void SaveBuyShopItem(uint32 userId,int itemId,int num,int itemLevel,int useTongbao,int leftTongbao);
void SaveUseItem(uint32 userId,SItemInstance &item,const char *reason,
                uint8 num,string before = "",string end = "");

struct SPet;
void SaveDelPet(uint32 userId,SPet *pPet);
void SaveCatchPet(uint32 roleId,SPet *pPet);
void SaveJieBangPet(uint32 roleId,SPet &pPet);

bool IllegalStr(string &str);

void SysInfoToAllUser(const char *msg);

void BeginFightHuoDong();
void EndFightHuoDong();
bool InFightHuoDong();

int GetLeftDropNum();
void SetLeftDropNum(int num);

int GbkToUnicode(char *to,size_t toLen,char *from,size_t fromLen);
int UnicodeToGbk(char *to,size_t toLen,char *from,size_t fromLen);

int GetScriptVal(int id);
void SetScriptVal(int id,int val);
void ReadScriptVal();
void WriteScriptVal();

void SetClearTime(time_t t);
time_t GetClearTime();
uint8 GetPetQuality(SPet*);

void AddDiscipleAward(CUser *pUser);
//void AddMasterAward(CUser *pUser,int type);
void SendSysMail(CUser *pUser,const char *info);
void SendSysMail(uint32 roleId,const char *info);

bool IsIllegalMsg(const char *msg);
void SendSysInfoToGroup(int sceneGroup,const char *info);
void SysInfoToGroupUser(int sceneGroup,const char *info);

//void Compress(uint8 *p,uint32 len,string &compress);
//void UnCompress(char *src,uint8 *p,uint32 *pLen);

void SaveGuiJieJl(CUser *pUser,int item);

uint8 GetQuality(int fen,uint8 num);

uint8 GetChongKaiQuality(SItemInstance *pItem);

void SendMsgToAllUser(CNetMessage &msg);
void SendSceneMsg(CNetMessage &msg,int sceneId);
void SendSceneMsg(CNetMessage &msg,CScene *pScene);

void AddTongBao(uint32 roleId,int tongbao);

uint8 GetPetSpeed(int qinmi);

uint8 GetRoleName(uint32 id,char *name);

uint32 GetRoleId(const char *name,uint8 &level);

void DelMission(uint32 roleId,int missId);

void AddMoney(uint32 roleId,int money);

//字符串转换十六进制
int UnHexify(unsigned char *obuf, const char *ibuf);

//十六进制转化字符串
void Hexify(unsigned char *obuf, const unsigned char *ibuf, int len);

bool Compress(uint8 *pInBuf,uint32 inLen,string &compress);

bool UnCompress(const char *inStr,uint8 *pOutBuf,uint32 &outLen);

bool AddPackage(uint32 roleId,SItemInstance &item);

inline int atomic_exchange_and_add( int * pw, int dv )
{
    // int r = *pw;
    // *pw += dv;
    // return r;

    int r;

    __asm__ __volatile__
    (
        "lock\n\t"
        "xadd %1, %0":
        "=m"( *pw ), "=r"( r ): // outputs (%0, %1)
        "m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
        "memory", "cc" // clobbers
    );

    return r;
}

inline void atomic_increment( int * pw )
{
    //atomic_exchange_and_add( pw, 1 );

    __asm__
    (
        "lock\n\t"
        "incl %0":
        "=m"( *pw ): // output (%0)
        "m"( *pw ): // input (%1)
        "cc" // clobbers
    );
}

inline int atomic_conditional_increment( int * pw )
{
    // int rv = *pw;
    // if( rv != 0 ) ++*pw;
    // return rv;

    int rv, tmp;

    __asm__
    (
        "movl %0, %%eax\n\t"
        "0:\n\t"
        "test %%eax, %%eax\n\t"
        "je 1f\n\t"
        "movl %%eax, %2\n\t"
        "incl %2\n\t"
        "lock\n\t"
        "cmpxchgl %2, %0\n\t"
        "jne 0b\n\t"
        "1:":
        "=m"( *pw ), "=&a"( rv ), "=&r"( tmp ): // outputs (%0, %1, %2)
        "m"( *pw ): // input (%3)
        "cc" // clobbers
    );

    return rv;
}

#endif
