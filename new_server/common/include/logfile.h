#ifndef _LOG_FILE_H_
#define _LOG_FILE_H_
#include <iostream>
#include <stdio.h> 
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <assert.h>
using namespace std;

class CLogs
{
public:
    CLogs():m_pFile(NULL)
    {
    }
    ~CLogs()
    {
        if(m_pFile != NULL)
            fclose(m_pFile);
    }
    bool SetLogFileName(string &);          //设置日志文件名
    void WriteLog(const char *fmt, ...);                //写日志，会在写入字符串后面自动加上写日志的时间
private:
    FILE *m_pFile;
};

#endif
