// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

#define _CRT_SECURE_NO_WARNINGS
#define  _CRT_RAND_S
#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <string>
using namespace std;
using namespace stdext;
#define CONFIG_FILE_NAME    "game_server.ini"

#include <stdlib.h>
// TODO: 在此处引用程序需要的其他头文件
