// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <string>
using namespace std;
using namespace stdext;
#define CONFIG_FILE_NAME    "login_server.ini"
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
