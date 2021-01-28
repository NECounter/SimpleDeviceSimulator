// DeviceSimulator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <vector>
#include <string>
#include <winsock.h>
#include "MemFileHandler.h"
#include "DeviceDataController.h"
#include "FileUtils.h"
#include "DataConvertorUtils.h"
#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARINGS

using namespace std;


void InitSockListener(DeviceDataController* dataController);
DWORD WINAPI ServerThread(LPVOID lpParameter);

struct SocketInfo
{
	DeviceDataController* dataController;
	SOCKET* acceptedSocket;
};

int main() {
    MemFileHandler* mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    DeviceDataController* dataController = new DeviceDataController(mem);
   
	InitSockListener(dataController);
	return 0;
}

void InitSockListener(DeviceDataController* dataController) {
	//初始化套接字库
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
		return;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
		return;
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}

	//填充服务端地址信息
	//定义服务端套接字，接受请求套接字
	SOCKET listenerSocket;
	//服务端地址客户端地址
	SOCKADDR_IN listenerAddr;
	//填充服务端信息
	listenerAddr.sin_family = AF_INET;
	listenerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	listenerAddr.sin_port = htons(8000);
	//创建套接字
	listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(listenerSocket, (SOCKADDR*)&listenerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "套接字绑定失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字绑定成功！" << endl;
	}
	//设置套接字为监听状态
	if (listen(listenerSocket, SOMAXCONN) < 0) {
		cout << "设置监听状态失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "设置监听状态成功！" << endl;
	}
	cout << "服务端正在监听连接，请稍候...." << endl;


	while (true)
	{
		//循环接收客户端连接请求并创建服务线程
		SOCKET* acceptedSocket = new SOCKET;
		//s_accept = (SOCKET*)malloc(sizeof(SOCKET));
		//接收客户端连接请求
		*acceptedSocket = accept(listenerSocket, 0, 0);
		if (*acceptedSocket == SOCKET_ERROR) {
			cout << "获取监听Socket失败！";
		}
		else {
			SocketInfo sockInfo;
			sockInfo.acceptedSocket = acceptedSocket;
			sockInfo.dataController = dataController;
			cout << "一个客户端已连接到服务器，socket是：" << *acceptedSocket << endl;
			CreateThread(NULL, 0, &ServerThread, &sockInfo, 0, NULL);
		}
	}
	//关闭套接字
	closesocket(listenerSocket);
	//释放DLL资源
	WSACleanup();
}

//服务线程
DWORD WINAPI ServerThread(LPVOID lpParameter) {
	SocketInfo* sockInfo = (SocketInfo*)lpParameter;
	SOCKET* clientSocket = sockInfo->acceptedSocket;
	DeviceDataController* daraController = sockInfo->dataController;

	string reg = ",";
	int recvByt = 0;
	int sendSig = 0;
	char* buf;

	int offset = 0;
	int bitOffset = 0;
	int dData = 0;
	float fData = 0.0f;
	bool bData = false;

	char recvBuf[1024];
	char sendBuf[1024];
	vector<string> cmd;
	while (true) {
		string msg = "...";
		recvByt = recv(*clientSocket, recvBuf, sizeof(recvBuf), 0);
		if (recvByt > 0){
			
			char* token = strtok_s(recvBuf, reg.c_str(), &buf);
			while (token != NULL) {
				cmd.push_back(token);
				token = strtok_s(NULL, reg.c_str(), &buf);
			}
			int cmdLen = cmd.size();
			if (cmd[0]=="getb"){
				if (cmdLen == 3) {
					try{
						offset = stoi(cmd[1]);
						bitOffset = stoi(cmd[2]);
						bool bmsg = daraController->getBit(offset,bitOffset);
						if (bmsg){
							msg = "True";
						}
						else{
							msg = "False";
						}
					}
					catch (const std::exception&){
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			} 
			else if (cmd[0] == "setb") {
				if (cmdLen == 4) {
					try {
						offset = stoi(cmd[1]);
						bitOffset = stoi(cmd[2]);
						bData = stoi(cmd[3]);
						daraController->setBit(offset, bitOffset,bData);
						msg = "设置成功";
					
					}
					catch (const std::exception&) {
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			}
			else if (cmd[0] == "getd") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmd[1]);
						msg = to_string(daraController->getDWord(offset));
					}
					catch (const std::exception&) {
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			}
			else if (cmd[0] == "setd") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmd[1]);
						dData = stoi(cmd[2]);
						daraController->setDWord(offset,dData);
						msg = "设置成功";
					}
					catch (const std::exception&) {
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			}
			else if (cmd[0] == "getf") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmd[1]);
						msg = to_string(daraController->getFloat(offset));
					}
					catch (const std::exception&) {
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			}
			else if (cmd[0] == "setf") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmd[1]);
						fData = stof(cmd[2]);
						daraController->setFloat(offset, fData);
						msg = "设置成功";
					}
					catch (const std::exception&) {
						msg = "参数错误";
					}
				}
				else {
					msg = "参数错误";
				}
			}
			else if (cmd[0] == "save") {
				if (cmdLen == 1) {
					daraController->save();
					msg = "保存成功";
				}
			}
			else {
				msg = "参数错误";
			}
		}

		memcpy(sendBuf, msg.c_str(), sizeof(msg));
		sendSig = send(*clientSocket, sendBuf, sizeof(sendBuf), 0);

		cmd.clear();
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(sendBuf, 0, sizeof(sendBuf));
	}
	closesocket(*clientSocket);
	free(clientSocket);
	return 0;
}

