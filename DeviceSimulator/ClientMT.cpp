#include<netinet/in.h>   // sockaddr_in
#include<sys/types.h>    // socket
#include<sys/socket.h>   // socket
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<iostream>
#include<string>
#include<cstdlib>
#include<cstdio>
#include<cstring>
#include<random>
#include<thread>

using namespace std;
#define BUFFER_SIZE 1024


struct sockaddr_in server_addr;
socklen_t server_addr_len;

char recvBuf[BUFFER_SIZE];




int ClientInit(string ip, int port);
void Connect(int fd);
void Send(string str, int fd);
string Recv(int fd);


int ClientInit(string ip, int port)
{
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) == 0)
    {
        cout << "Server IP Address Error!\n";
        exit(-1);
    }
    server_addr.sin_port = htons(port);
    server_addr_len = sizeof(server_addr);
    // create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
    {
        cout << "Create Socket Failed!\n";
        exit(-1);
    }
    return fd;
}


void Connect(int fd)
{
    cout << "Connecting......\n";
    if(connect(fd, (struct sockaddr*)&server_addr, server_addr_len) < 0)
    {
        cout << "Can not Connect to Server IP!";
        exit(-1);
    }
    cout << "Connect to Server successfully.\n";
}

void Send(string str, int fd)
{
    cout << str << endl;
    int ret1 = send(fd, str.c_str(), sizeof(str), 0);
    if(ret1 < 0)
    {
        cout << "Send Message Failed!\n";
        exit;
    }
}

string Recv(int fd)
{
    int len = recv(fd, recvBuf, sizeof(recvBuf), 0);
    if(len < 0)
    {
        cout << "recv() error!\n";
    }
    string result(recvBuf);
    return result;
}

void ClientThread(int id){
    int fd = ClientInit("127.0.0.1", 8000);
    Connect(fd);
    default_random_engine e;
    int count = 0;
    while(1)
    {
        //string cmds[8] = {"getb,0,0", "setb,0,0,1", "getd,4", "setd,4,100", "getf,8", "setf,8,3.14", "save", "?"};
        //Send(cmds[(e() + id) %8], fd);

        Send("save", fd);

        cout << Recv(fd) << " :"<< id <<": "<< count++ << "\n";
    }
}

int main()
{
    string msg;
    cout << "Threads??\n";
    getline(cin, msg);
    int nThreads = stoi(msg);
    thread clientThreads[nThreads];
    for (int i = 0; i < nThreads; i++){
        clientThreads[i] = thread(ClientThread, i);
    }
    
    for(auto& th:clientThreads){
        th.join();
    }
    return 0;
}
