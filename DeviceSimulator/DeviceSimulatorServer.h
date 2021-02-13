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

#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <atomic>

#include <mysql++.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#define _CRT_SECURE_NO_WARINGS


#define PORT 8000
#define DBNAME "device_log"
#define DBHOST "192.168.3.50"
#define DBUSER "root"
#define DBPASSWD "123"


using namespace std;
using namespace mysqlpp;

#define BUFFER_SIZE 1024
#define EPOLLSIZE 100
#define WORKER_SIZE 3

struct PACKET_HEAD{// header of one message, describes the size of following message
    int length;
};


struct QueryInfo{ //infomation of one query
	std::string operation;
	std::string dataType;
	std::string offsetByte;
	std::string offsetBit;
	std::string valueWrite;
	std::string valueRead;
	std::string clientId;
	std::string operationDT;
};


    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    int listen_fd;                        // listener's fd
    int epfdBoss;                             // epoll fd of acceptor
    int epfdWorkers[WORKER_SIZE];                             // epoll fd of receive1               
    struct epoll_event events[EPOLLSIZE]; // placeholder of event list which epoll_wait retruns
    int workerIndex = 0;

    //save lock
    mutex saveMTX;

    MemFileHandler* mem;
    DeviceDataController* dataController;
    QueryInfo queryInfo;

    Connection* conn;
    bool DBConnected = false;
    void ServerInit();
    void ServerDispose();


    void Recv(int epfdWorker);
    string cmdHandlerService(string cmd, int fd);
    bool sqlWriteService(QueryInfo queryInfo);
    void Accept(int workerId);
    void EpollThread(int flag);

    void Bind();
    void Listen(int queue_len = 20);
    
    void Run();




