#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <winsock.h>
#include "MemFileHandler.h"
#include "DeviceDataController.h"
#include "FileUtils.h"
#include "DataConvertorUtils.h"
#include <time.h>

#include "mysql++.h"

#ifdef _DEBUG
#pragma  comment(lib,"mysqlpp_d.lib")
#else
#pragma  comment(lib,"mysqlpp.lib")
#endif

#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARINGS
void InitSockListener(DeviceDataController* dataController);
DWORD WINAPI ServerThread(LPVOID lpParameter);

struct SocketInfo
{
	DeviceDataController* dataController;
	SOCKET* acceptedSocket;
};

struct QueryInfo
{
	std::string operation;
	std::string dataType;
	std::string offsetByte;
	std::string offsetBit;
	std::string valueWrite;
	std::string valueRead;
	std::string clientId;
	std::string operationDT;
};

#define port  8000
#define databaseName "device_log"
#define databaseHostName "127.0.0.1"
#define databaseUserName "root"
#define databasePasswd "1234"

