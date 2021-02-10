#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "MemFileHandler.h"
#include "DeviceDataController.h"
#include "FileUtils.h"
#include "DataConvertorUtils.h"
#include <time.h>
#include <netinet/in.h> // sockaddr_in
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h> // epoll
#include <sys/ioctl.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#define _CRT_SECURE_NO_WARINGS

using namespace std;

#define BUFFER_SIZE 1024
#define EPOLLSIZE 100

struct PACKET_HEAD
{
    int length;
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

class DeviceSimulatorServer
{
private:
    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    int listen_fd;                        // 监听的fd
    int epfd;                             // epoll fd
    struct epoll_event events[EPOLLSIZE]; // epoll_wait返回的就绪事件

    MemFileHandler* mem;
    DeviceDataController* dataController;
    QueryInfo queryInfo;

    string databaseName;
    string databaseHostName;
    string databaseUserName;
    string databasePasswd;

    
public:
    DeviceSimulatorServer(int port, string databaseHostName, string databaseName, string databaseUserName, string databasePasswd);
    ~DeviceSimulatorServer();
    void Bind();
    void Listen(int queue_len = 20);
    void Accept();
    void Run();
    void Recv(int fd);
    string cmdHandlerService(string cmd, int fd);
    string sqlWriteService(QueryInfo queryInfo);
};


