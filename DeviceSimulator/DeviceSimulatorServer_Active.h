#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "MemFileHandler.h"
#include "DeviceDataController.h"
#include "FileUtils.h"
#include "DataConvertorUtils.h"
#include "FileUtils.h"
#include <time.h>
#include <netinet/in.h> // sockaddr_in
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h> // epoll
#include <sys/ioctl.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <atomic>

//#include <mysql++.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#define MAXFILE 65535

volatile sig_atomic_t _running = 1;

#define _CRT_SECURE_NO_WARINGS

#define PORT 8000
#define DBNAME "device_log"
#define DBHOST "192.168.3.50"
#define DBUSER "root"
#define DBPASSWD "123"

using namespace std;
//using namespace mysqlpp;

#define BUFFER_SIZE 1024 //size of recv and send buffer
#define EPOLLSIZE 8196   //size of epoll lists
#define WORKER_SIZE 4    //number of workers (1 boss, n workers)

struct QueryInfo
{ //infomation of one query
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
int listen_fd;                              // listener's fd
int epfdBoss;                               // epoll fd of boss
int epfdDispatcher;                         // epoll fds of workers
struct epoll_event acceptEvents[EPOLLSIZE]; // placeholder of event list which epoll_wait retruns (for boss)
struct epoll_event recvEvents[EPOLLSIZE];   // placeholder of event list which epoll_wait retruns (for workers)
atomic<int> numsWorks = 0;
char recvBuffer[WORKER_SIZE][BUFFER_SIZE];
char sendBuffer[WORKER_SIZE][BUFFER_SIZE];
//save lock
mutex saveMTX;

//worker lock
mutex exeMTX;
condition_variable exeCV;
atomic<bool> taskReady = false;

MemFileHandler *mem;
DeviceDataController *dataController;
QueryInfo queryInfo;

//Connection* conn;
bool DBConnected = false;

string logPath = "/tmp/device_service.log";

void sigterm_handler(int arg);

void ServerInit(); // init
void Bind();
void Listen(int queue_len = 128);
void Run();
void ServerDispose(); //dispose

void EpollThread(); // An epoll thread

void Accept();   // (jobs of boss)
void Dispatch(); // (jobs oof workers)
void Worker(int workerId);

string cmdHandlerService(string cmd, int fd); // server service
bool sqlWriteService(QueryInfo queryInfo);
