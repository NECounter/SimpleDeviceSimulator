#include "DeviceSimulatorServer.h"

int main() {
	ServerInit();
    Bind();
    Listen();
    Run();
    ServerDispose();
    return 0;
}

void ServerInit(){
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // create socket to listen
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0){
        cout << "Create Socket Failed!!\n";
        exit;
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // load something
    mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    dataController = new DeviceDataController(mem);
    queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };

    // create connection to DB 
    SetCharsetNameOption charsetOpt("utf8");
    try{
        conn = new Connection(false);
        conn->set_option(&charsetOpt);
        conn->connect(DBNAME, DBHOST, DBUSER, DBPASSWD, 3306);
        if (conn->connected())
        {
            DBConnected = true;
        }  
    }
    catch(const exception& e){

        cerr << e.what() << "\n";
    }
}

void ServerDispose(){
    delete[] mem;
    delete[] dataController;
    if (conn->connected()){
        conn->disconnect();
        delete[] conn;
    }
}


void Bind(){
    if (-1 == (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)))){
        cout << "Server Bind Failed!\n";
        exit;
    }
    cout << "Bind Successfully.\n";
}

void Listen(int queue_len){
    if (-1 == listen(listen_fd, queue_len)){
        cout << "Server Listen Failed!\n";
        exit;
    }
    cout << "Listen Successfully.\n";
}

void Run(){
    epfdBoss = epoll_create(EPOLLSIZE); // create epoll handler and return its fd
    for (int i = 0; i < WORKER_SIZE; i++){
        epfdWorkers[i] = epoll_create(EPOLLSIZE);
    }


    // register the listen_fd to epfd
    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLLIN;
    epoll_ctl(epfdBoss, EPOLL_CTL_ADD, listen_fd, &event); // register listeners fd to epfdBoss

    // init epoll threads (boss(0) + workers(1,2...))
    thread epollThread[WORKER_SIZE+1];
    for (int i = 0; i < WORKER_SIZE+1; i++){
        epollThread[i] = thread(EpollThread, i);
    }

    for (auto& th : epollThread) th.join(); // join to the mian thread
}

void EpollThread(int flag){
    while (1){
        if (flag == 0){ // boss
            //cout << "accept\n";
            Accept(++workerIndex % WORKER_SIZE); // assign incoming socket connections to workers by Round Robin 
        }
        else{ //workers
            //cout << "recv\n";
            Recv(flag-1);   
        }
    }
}


void Accept(int workerId){
    int numsBoss = epoll_wait(epfdBoss, acceptEvents, EPOLLSIZE, -1); //waiting for new connections (blocking)
    if (numsBoss < 0){
        cout << "Boss Error!\n";
    }

    if (numsBoss > 0){ // incoming connections
        for (int i = 0; i < numsBoss; i++){
            if (acceptEvents[i].events == EPOLLIN){
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);

                int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len); // accept new connections
                if (new_fd < 0){
                    cout << "Server Accept Failed!\n";
                }
                else{
                    cout << "new connection was accepted.\n";

                    // register the incomming fd to epfd
                    struct epoll_event event;
                    event.data.fd = new_fd;
                    event.events = EPOLLIN;

                    epoll_ctl(epfdWorkers[workerId], EPOLL_CTL_ADD, new_fd, &event); // register workers' fd to epfdWorker (epoll is thread-safe)

                }
            }  
        }          
    }
}

void Recv(int workerId){
    int epfdWorker = epfdWorkers[workerId];
    int numsWorker = epoll_wait(epfdWorker, recvEvents[workerId], EPOLLSIZE, -1); // waiting for socket readable events (blocking)
    
    if (numsWorker < 0){
         cout << "Worker Error!\n";
    }

     bool close_conn = false; // an indicator of the status of this fd

    if (numsWorker > 0){ 
        for (int i = 0; i < numsWorker; i++){
    
            int fd = recvEvents[workerId][i].data.fd;
            if (recvEvents[workerId][i].events == EPOLLIN){ // incoming messages
                int len = recv(fd, recvBuffer[workerId], sizeof(recvBuffer[workerId]), 0);
            
                if (len <= 0){
                    cout << "recv() error!\n";
                    close_conn = true;  
                }
                else{
                    string msg = cmdHandlerService(recvBuffer[workerId], fd); // the main service of this server
                
                    msg += "\n";
                    memcpy(sendBuffer[workerId], msg.c_str(), strlen(msg.c_str()));
            
                    int ret1 = send(fd, sendBuffer[workerId], strlen(msg.c_str()), 0); // return results to clients
                    if (ret1 <= 0){
                        cout << "send() error!\n";
                        close_conn = true;
                    }else{
                        //sqlWriteService(queryInfo);
                    }
                } 
                memset(recvBuffer[workerId], 0, sizeof(recvBuffer[workerId]));    
                memset(sendBuffer[workerId], 0, sizeof(sendBuffer[workerId]));      
            }    
            else if(recvEvents[workerId][i].events != EPOLLIN){ // other events, close this connection
                close_conn = true;

            }
            if (close_conn){ // bad cnnection, close it
                struct epoll_event event;
                event.data.fd = fd;
                event.events = EPOLLIN;
                epoll_ctl(epfdWorker, EPOLL_CTL_DEL, fd, &event); // use epoll_ctl to del a fd from epoll
                close(fd);
                cout << "A client has disconneted\n";
            }
        }
    }
}

void Executor(int workerId, int numsEvent){
    while (true)
    {
        /* code */
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
        msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
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
                msg = "Invalid";
            }
        }
        else {
            msg = "Invalid";
        }
    }
    else if (cmds[0] == "save") {
        if (cmdLen == 1) {
            unique_lock<mutex> saveLock(saveMTX); // should write codes which are thread-safe
            dataController->save();
            msg = "Saved";
            queryInfo.operation = "save";
            saveLock.unlock();
        }
    }
    else {
        msg = "Invalid";
    }

    time_t timep;
    time(&timep);
    char tmp[256];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    queryInfo.operationDT = tmp;

    return msg;
}

bool sqlWriteService(QueryInfo queryInfo){
    if (DBConnected){
        string sql = "INSERT INTO `device_log`.`operation_log` (`operation`, `data_type`, `offset_byte`, `offset_bit`, `value_write`, `value_read`, `client_id`, `operation_dt`) \
VALUES('" + queryInfo.operation + "', '" + queryInfo.dataType + "', " + queryInfo.offsetByte + ", " + queryInfo.offsetBit + ", " + queryInfo.valueWrite + ", " + queryInfo.valueRead + ", " + queryInfo.clientId + ", '" + queryInfo.operationDT + "')";
        Query query = conn->query(sql);
        query.exec();
        return true;
    }
    else{
        //cout << "DB connection failed: " << conn->error() << "\n";
        return false;
    }
}
 
