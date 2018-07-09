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
    bool SetLogFileName(string &);          //������־�ļ���
    void WriteLog(const char *fmt, ...);                //д��־������д���ַ��������Զ�����д��־��ʱ��
private:
    FILE *m_pFile;
};

#endif
