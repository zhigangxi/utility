#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <list>
#include <string>
using namespace std;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned int    uint32;
typedef long long       int64;
typedef unsigned long long uint64;

int SplitLine(char **templa, int templatecount, char *pkt,char space);

bool StrEndCmp(const char *str1,const char *str2);

//file格式为".cpp;.h;.java……"
bool FindFile(const char *path,const char *fileType,list<string> &findFile);

//查找filename中文件，结果在fullname中(包含路径)
bool FindFile(const char *path,list<string> &fileName,list<string> &fullName);

void Trim(char *str,const char *pkt = " \n\r\t");

#endif
