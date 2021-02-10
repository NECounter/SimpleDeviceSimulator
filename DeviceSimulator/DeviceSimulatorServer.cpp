#include "DeviceSimulatorServer.h"

DeviceSimulatorServer::DeviceSimulatorServer(int port, string databaseHostName, string databaseName, string databaseUserName, string databasePasswd)
{
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);
    // create socket to listen
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        cout << "Create Socket Failed!" << endl;
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    dataController = new DeviceDataController(mem);
    queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };
}

DeviceSimulatorServer::~DeviceSimulatorServer()
{
    close(epfd);
    delete[] mem;
    delete[] dataController;
}

void DeviceSimulatorServer::Bind()
{
    if (-1 == (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))))
    {
        cout << "Server Bind Failed!" << endl;
        exit(1);
    }
    cout << "Bind Successfully." << endl;
}

void DeviceSimulatorServer::Listen(int queue_len)
{
    if (-1 == listen(listen_fd, queue_len))
    {
        cout << "Server Listen Failed!" << endl;
        exit(1);
    }
    cout << "Listen Successfully." << endl;
}

void DeviceSimulatorServer::Accept()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (new_fd < 0)
    {
        cout << "Server Accept Failed!" << endl;
        exit(1);
    }

    cout << "new connection was accepted." << endl;

    // 在epfd中注册新建立的连接
    struct epoll_event event;
    event.data.fd = new_fd;
    event.events = EPOLLIN;

    epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &event);
}

void DeviceSimulatorServer::Run()
{
    epfd = epoll_create(1); // 创建epoll句柄

    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &event); // 注册listen_fd

    while (1)
    {
        int nums = epoll_wait(epfd, events, EPOLLSIZE, -1);
        if (nums < 0)
        {
            cout << "poll() error!" << endl;
            exit(1);
        }

        if (nums == 0)
        {
            continue;
        }

        for (int i = 0; i < nums; ++i) // 遍历所有就绪事件
        {
            int fd = events[i].data.fd;
            if ((fd == listen_fd) && (events[i].events & EPOLLIN))
                Accept(); // 有新的客户端请求
            else if ((fd != listen_fd) && (events[i].events & EPOLLIN))
                Recv(fd); // 读数据
        }
    }
}

void DeviceSimulatorServer::Recv(int fd)
{
    bool close_conn = false; // 标记当前连接是否断开了

    PACKET_HEAD head;
    int recvByte = recv(fd, &head, sizeof(head), 0); // 先接受包头，即数据总长度
    if (recvByte > 0){
        char recvBuffer[head.length];
        bzero(recvBuffer, head.length);
        int total = 0;
        while (total < head.length)
        {
            int len = recv(fd, recvBuffer + total, head.length - total, 0);
            if (len < 0)
            {
                cout << "recv() error!" << endl;
                close_conn = true;
                break;
            }
            total = total + len;
        }

        if (total == head.length) // 将收到的消息原样发回给客户端
        {
            string msg = cmdHandlerService(recvBuffer, fd);
            head.length = msg.size() + 1;
            char* sendBuffer[head.length];
            memcpy(sendBuffer, msg.c_str(), head.length);
            
            int ret1 = send(fd, &head, sizeof(head), 0);
            int ret2 = send(fd, sendBuffer, head.length, 0);
            if (ret1 < 0 || ret2 < 0)
            {
                cout << "send() error!" << endl;
                close_conn = true;
            }
        }
    }
    else{
        close_conn = true;
    }

    

    if (close_conn) // 当前这个连接有问题，关闭它
    {
        close(fd);
        struct epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event); // Delete一个fd
        cout << "A client has disconneted" << endl;
    }
}

string DeviceSimulatorServer::cmdHandlerService(string cmd, int fd){
	string reg = ",";
	int offset = 0;
	int bitOffset = 0;
	int dData = 0;
	float fData = 0.0f;
	bool bData = false;

	vector<string> cmds;

    string msg = "...";

    queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };
    queryInfo.clientId = to_string(fd);
    if(cmd == ""){
        msg = "Invalid parameter";
        return msg;
    }
    
    char* buf;
    char* token = strtok_r((char*)cmd.c_str(), reg.c_str(), &buf);
    while (token != NULL) {
        cmds.push_back(token);
        token = strtok_r(NULL, reg.c_str(), &buf);
    }
    int cmdLen = cmds.size();
    if (cmds[0]=="getb"){
        if (cmdLen == 3) {
            try{
                offset = stoi(cmds[1]);
                bitOffset = stoi(cmds[2]);
                bool bmsg = dataController->getBit(offset,bitOffset);
                queryInfo.dataType = "Bit";
                queryInfo.offsetBit = cmds[2];
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Get";
                queryInfo.valueRead = to_string(bmsg);
                
                if (bmsg){
                    msg = "True";
                }
                else{
                    msg = "False";
                }
            }
            catch (...){
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    } 
    else if (cmds[0] == "setb") {
        if (cmdLen == 4) {
            try {
                offset = stoi(cmds[1]);
                bitOffset = stoi(cmds[2]);
                bData = stoi(cmds[3]);
                dataController->setBit(offset, bitOffset,bData);
                msg = "Set";
                queryInfo.dataType = "Bit";
                queryInfo.offsetBit = cmds[2];
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Set";
                queryInfo.valueWrite = to_string(bData);	
            }
            catch (...) {
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    }
    else if (cmds[0] == "getd") {
        if (cmdLen == 2) {
            try {
                offset = stoi(cmds[1]);
                msg = to_string(dataController->getDWord(offset));
                queryInfo.dataType = "DWord";
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Get";
                queryInfo.valueRead = msg;
            }
            catch (...) {
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    }
    else if (cmds[0] == "setd") {
        if (cmdLen == 3) {
            try {
                offset = stoi(cmds[1]);
                dData = stoi(cmds[2]);
                dataController->setDWord(offset,dData);
                msg = "Set";
                queryInfo.dataType = "DWord";
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Set";
                queryInfo.valueWrite = to_string(dData);
            }
            catch (...) {
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    }
    else if (cmds[0] == "getf") {
        if (cmdLen == 2) {
            try {
                offset = stoi(cmds[1]);
                msg = to_string(dataController->getFloat(offset));
                queryInfo.dataType = "Float";
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Get";
                queryInfo.valueRead = msg;
            }
            catch (...) {
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    }
    else if (cmds[0] == "setf") {
        if (cmdLen == 3) {
            try {
                offset = stoi(cmds[1]);
                fData = stof(cmds[2]);
                dataController->setFloat(offset, fData);
                msg = "Set";
                queryInfo.dataType = "Float";
                queryInfo.offsetByte = cmds[1];
                queryInfo.operation = "Set";
                queryInfo.valueWrite = to_string(fData);
            }
            catch (...) {
                msg = "Invalid parameter";
            }
        }
        else {
            msg = "Invalid parameter";
        }
    }
    else if (cmds[0] == "save") {
        if (cmdLen == 1) {
            dataController->save();
            msg = "Saved";
            queryInfo.operation = "save";
        }
    }
    else {
        msg = "Invalid parameter";
    }

    time_t timep;
    time(&timep);
    char tmp[256];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    queryInfo.operationDT = tmp;

    return msg;
}

string DeviceSimulatorServer::sqlWriteService(QueryInfo queryInfo){

}

    /**

	int clientFD = 0;

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
	vector<string> cmds;
	while (true) {
		string msg = "...";
		bool success = false;
		QueryInfo queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };
		queryInfo.clientId = to_string(*clientSocket);
		recvByt = recv(*clientSocket, recvBuf, sizeof(recvBuf), 0);

		if (recvByt > 0){
			
			char* token = strtok_s(recvBuf, reg.c_str(), &buf);
			while (token != NULL) {
				cmds.push_back(token);
				token = strtok_s(NULL, reg.c_str(), &buf);
			}
			int cmdLen = cmds.size();
			if (cmds[0]=="getb"){
				if (cmdLen == 3) {
					try{
						offset = stoi(cmds[1]);
						bitOffset = stoi(cmds[2]);
						bool bmsg = dataController->getBit(offset,bitOffset);
						success = true;
						queryInfo.dataType = "Bit";
						queryInfo.offsetBit = cmds[2];
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Get";
						queryInfo.valueRead = to_string(bmsg);
						
						if (bmsg){
							msg = "True";
						}
						else{
							msg = "False";
						}
					}
					catch (...){
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			} 
			else if (cmds[0] == "setb") {
				if (cmdLen == 4) {
					try {
						offset = stoi(cmds[1]);
						bitOffset = stoi(cmds[2]);
						bData = stoi(cmds[3]);
						dataController->setBit(offset, bitOffset,bData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "Bit";
						queryInfo.offsetBit = cmds[2];
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Set";
						queryInfo.valueWrite = to_string(bData);	
					}
					catch (...) {
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			}
			else if (cmds[0] == "getd") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmds[1]);
						msg = to_string(dataController->getDWord(offset));
						success = true;
						queryInfo.dataType = "DWord";
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Get";
						queryInfo.valueRead = msg;
					}
					catch (...) {
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			}
			else if (cmds[0] == "setd") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmds[1]);
						dData = stoi(cmds[2]);
						dataController->setDWord(offset,dData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "DWord";
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Set";
						queryInfo.valueWrite = to_string(dData);
					}
					catch (...) {
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			}
			else if (cmds[0] == "getf") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmds[1]);
						msg = to_string(dataController->getFloat(offset));
						success = true;
						queryInfo.dataType = "Float";
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Get";
						queryInfo.valueRead = msg;
					}
					catch (...) {
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			}
			else if (cmds[0] == "setf") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmds[1]);
						fData = stof(cmds[2]);
						dataController->setFloat(offset, fData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "Float";
						queryInfo.offsetByte = cmds[1];
						queryInfo.operation = "Set";
						queryInfo.valueWrite = to_string(fData);
					}
					catch (...) {
						msg = "Invalid parameter";
					}
				}
				else {
					msg = "Invalid parameter";
				}
			}
			else if (cmds[0] == "save") {
				if (cmdLen == 1) {
					dataController->save();
					msg = "Saved";
					success = true;
					queryInfo.operation = "save";
				}
			}
			else {
				msg = "Invalid parameter";
			}
		}
		else {
			break;
		}

		if (success)
		{
            SetCharsetNameOption* opt = new SetCharsetNameOption("utf8");
	        Connection conn(false);
	        conn.set_option(opt);
			time_t timep;
			time(&timep);
			char tmp[256];
			strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
			queryInfo.operationDT = tmp;

			if (conn.connect(databaseName, databaseHostName, databaseUserName, databasePasswd))
			{
				string sql = "INSERT INTO `device_log`.`operation_log` (`operation`, `data_type`, `offset_byte`, `offset_bit`, `value_write`, `value_read`, `client_id`, `operation_dt`) \
VALUES('" + queryInfo.operation + "', '" + queryInfo.dataType + "', " + queryInfo.offsetByte + ", " + queryInfo.offsetBit + ", " + queryInfo.valueWrite + ", " + queryInfo.valueRead + ", " + queryInfo.clientId + ", '" + queryInfo.operationDT + "')";
				Query query = conn.query(sql);
				query.exec();
				conn.disconnect();
				//Query query = conn.query("select * from book");
				//UseQueryResult res = query.use();
				//if (res)
				//{
				//	while (Row row = res.fetch_row())
				//	{
				//		cout << setw(9) << "BookName:" << row["bookname"] << endl;
				//		cout << setw(9) << "Size:" << row["size"] << endl;
				//	}
				//}
				//else
				//{
				//	cerr << "Failed to get item list: " << query.error() << endl;
				//	return 1;
				//}
			}
			else
			{
				cout << "DB connection failed: " << conn.error() << endl;
			}
		}
		msg = msg + "\n";

		memcpy(sendBuf, msg.c_str(), sizeof(msg));
		sendSig = send(*clientSocket, sendBuf, sizeof(msg), 0);
		cmds.clear();
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(sendBuf, 0, sizeof(sendBuf));
	}
	int socketNo = *clientSocket;
	closesocket(*clientSocket);
	free(clientSocket);
	cout << "A client has disconnected，socket：" << socketNo << endl;
	return 0;
	**/
 