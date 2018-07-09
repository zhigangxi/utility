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
	~CLogs();
	static bool Init();
    static bool SetLogFileName(string &);          //������־�ļ���
	static void WriteLog(const char *fmt, ...);                //д��־������д���ַ��������Զ�����д��־��ʱ��
private:
    static FILE *m_pFile;
};

#endif
