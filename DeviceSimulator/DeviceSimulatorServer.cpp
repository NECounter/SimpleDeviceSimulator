#include "DeviceSimulatorServer.h"

int main() {
	ServerInit();
    Bind();
    Listen();
    Run();
    return 0;
}

void ServerInit()
{
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // create socket to listen
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        cout << "Create Socket Failed!!" << endl;
        exit;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    dataController = new DeviceDataController(mem);
    queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };

    try
    {
        SetCharsetNameOption* opt = new SetCharsetNameOption("utf8");
        conn = new Connection(false);
        conn->set_option(opt);
        conn->connect(DBNAME, DBHOST, DBUSER, DBPASSWD, 3306);
    }
    catch(const exception& e)
    {
        cerr << e.what() << endl;
    }
    
    
}


void Bind()
{
    if (-1 == (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))))
    {
        cout << "Server Bind Failed!" << endl;
        exit(1);
    }
    cout << "Bind Successfully." << endl;
}

void Listen(int queue_len)
{
    if (-1 == listen(listen_fd, queue_len))
    {
        cout << "Server Listen Failed!" << endl;
        exit(1);
    }
    cout << "Listen Successfully." << endl;
}

void Accept(int workerId)
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

    // register the incomming fd to epfd
    struct epoll_event event;
    event.data.fd = new_fd;
    event.events = EPOLLIN;

    epoll_ctl(epfdWorkers[workerId], EPOLL_CTL_ADD, new_fd, &event);
}

void Run()
{
    epfdBoss = epoll_create(1); // create epoll handler and return its fd
    for (int i = 0; i < WORKER_SIZE; i++)
    {
        epfdWorkers[i] = epoll_create(EPOLLSIZE);
    }


    // register the listen_fd to epfd
    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLLIN;
    epoll_ctl(epfdBoss, EPOLL_CTL_ADD, listen_fd, &event); 
    thread epollThread[3];
    for (int i = 0; i < WORKER_SIZE+1; i++)
    {
        epollThread[i] = thread(EpollThread);
    }

    for (auto& th : epollThread) th.join();
}

void EpollThread(){
    while (1)
    {
        int numsBoss = epoll_wait(epfdBoss, events, EPOLLSIZE, 0);
        if (numsBoss < 0)
        {
            cout << "Boss Error!" << endl;
        }

        if (numsBoss > 0)
        {
            for (int i = 0; i < numsBoss; i++)
            {
                if (events[i].events == EPOLLIN)
                {
                    Accept(++workerIndex % 2);
                }  
            }          
        }

        for (int i = 0; i < WORKER_SIZE; i++)
        {
            Recv(epfdWorkers[i]);
        }
    }

}

void Recv(int epfdWorker)
{
    int numsWorker = epoll_wait(epfdWorker, events, EPOLLSIZE, 0);
    if (numsWorker < 0)
    {
         cout << "Worker Error!" << endl;
    }

    if (numsWorker > 0)
    {
        for (int i = 0; i < numsWorker; i++)
        {
            if (events[i].events == EPOLLIN){
                int fd = events[i].data.fd;
                bool close_conn = false; // an indicator of the status of this fd

                PACKET_HEAD head;
                int recvByte = recv(fd, &head, sizeof(head), 0); // receive the messae header first
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

                    if (total == head.length) // finished receiving, prepare to send messages to the clinet 
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
                        }else{
                            sqlWriteService(queryInfo);
                        }
                    }
                }
                else{
                    close_conn = true;
                }

                

                if (close_conn) // bad cnnection, close it
                {
                    close(fd);
                    struct epoll_event event;
                    event.data.fd = fd;
                    event.events = EPOLLIN;
                    epoll_ctl(epfdWorker, EPOLL_CTL_DEL, fd, &event); // use epoll_ctl to del a fd from epoll
                    cout << "A client has disconneted" << endl;
                }


            }
            
        }
    }
}

string cmdHandlerService(string cmd, int fd){
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

string sqlWriteService(QueryInfo queryInfo){
    if (conn->connected())
    {
        string sql = "INSERT INTO `device_log`.`operation_log` (`operation`, `data_type`, `offset_byte`, `offset_bit`, `value_write`, `value_read`, `client_id`, `operation_dt`) \
VALUES('" + queryInfo.operation + "', '" + queryInfo.dataType + "', " + queryInfo.offsetByte + ", " + queryInfo.offsetBit + ", " + queryInfo.valueWrite + ", " + queryInfo.valueRead + ", " + queryInfo.clientId + ", '" + queryInfo.operationDT + "')";
        Query query = conn->query(sql);
        query.exec();
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
        cout << "DB connection failed: " << conn->error() << endl;
    }
    return " ";
}
 
