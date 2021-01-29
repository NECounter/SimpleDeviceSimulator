// DeviceSimulator.cpp : "main" included
//

#include "DeviceSimulator.h"

using namespace std;
using namespace mysqlpp;


int main() {
    MemFileHandler* mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    DeviceDataController* dataController = new DeviceDataController(mem);
   
	InitSockListener(dataController);
	return 0;
}

void InitSockListener(DeviceDataController* dataController) {
	//Init socket lib
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) {
		cout << "Failed to init socket" << endl;
		return;
	}
	else {
		cout << "Socket initialized" << endl;
	}
	//Check socket version
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2) {
		cout << "Invalid socket version" << endl;
		WSACleanup();
		return;
	}
	else {
		cout << "Socket version valid" << endl;
	}

	//Socket listener
	SOCKET listenerSocket;
	//Socket listener address
	SOCKADDR_IN listenerAddr;
	//Socket config
	listenerAddr.sin_family = AF_INET;
	listenerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	listenerAddr.sin_port = htons(port);
	//Create socket listener 
	listenerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(listenerSocket, (SOCKADDR*)&listenerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "Socket binding failed" << endl;
		WSACleanup();
	}
	else {
		cout << "Socket binding success" << endl;
	}
	//Enable listening
	if (listen(listenerSocket, SOMAXCONN) < 0) {
		cout << "Listener failed to enable" << endl;
		WSACleanup();
	}
	else {
		cout << "Listener enabled" << endl;
	}
	cout << "Servier is running, waiting for connection..." << endl;


	while (true)
	{
		//Accept incoming socket
		SOCKET* acceptedSocket = new SOCKET;
		*acceptedSocket = accept(listenerSocket, 0, 0);

		if (*acceptedSocket == SOCKET_ERROR) {
			cout << "Failed to accept incoming socket";
		}
		else {
			SocketInfo sockInfo;
			sockInfo.acceptedSocket = acceptedSocket;
			sockInfo.dataController = dataController;
			cout << "A client has connected，socket：" << *acceptedSocket << endl;
			CreateThread(NULL, 0, &ServerThread, &sockInfo, 0, NULL);
		}
	}
	//Close socket
	closesocket(listenerSocket);
	//release DLL
	WSACleanup();
}

//Service Thread
DWORD WINAPI ServerThread(LPVOID lpParameter) {
	Connection conn(false);
	SetCharsetNameOption* opt = new SetCharsetNameOption("utf8");
	conn.set_option(opt);
	Query query = NULL;
	

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
		bool success = false;
		QueryInfo queryInfo = { "-1" ,"-1" , "-1" , "-1" , "-1" , "-1" , "-1" , "-1" };
		queryInfo.clientId = to_string(*clientSocket);
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
						success = true;
						queryInfo.dataType = "Bit";
						queryInfo.offsetBit = cmd[2];
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "setb") {
				if (cmdLen == 4) {
					try {
						offset = stoi(cmd[1]);
						bitOffset = stoi(cmd[2]);
						bData = stoi(cmd[3]);
						daraController->setBit(offset, bitOffset,bData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "Bit";
						queryInfo.offsetBit = cmd[2];
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "getd") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmd[1]);
						msg = to_string(daraController->getDWord(offset));
						success = true;
						queryInfo.dataType = "DWord";
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "setd") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmd[1]);
						dData = stoi(cmd[2]);
						daraController->setDWord(offset,dData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "DWord";
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "getf") {
				if (cmdLen == 2) {
					try {
						offset = stoi(cmd[1]);
						msg = to_string(daraController->getFloat(offset));
						success = true;
						queryInfo.dataType = "Float";
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "setf") {
				if (cmdLen == 3) {
					try {
						offset = stoi(cmd[1]);
						fData = stof(cmd[2]);
						daraController->setFloat(offset, fData);
						msg = "Set";
						success = true;
						queryInfo.dataType = "Float";
						queryInfo.offsetByte = cmd[1];
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
			else if (cmd[0] == "save") {
				if (cmdLen == 1) {
					daraController->save();
					msg = "Saved";
					success = true;
					queryInfo.operation = "save";
				}
			}
			else {
				msg = "Invalid parameter";
			}
		}

		memcpy(sendBuf, msg.c_str(), sizeof(msg));
		sendSig = send(*clientSocket, sendBuf, sizeof(sendBuf), 0);
		cmd.clear();
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(sendBuf, 0, sizeof(sendBuf));

		if (success)
		{
			time_t timep;
			time(&timep);
			char tmp[256];
			strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
			queryInfo.operationDT = tmp;

			if (conn.connect(databaseName, databaseHostName, databaseUserName, databasePasswd))
			{
				string sql = "INSERT INTO `device_log`.`operation_log` (`operation`, `data_type`, `offset_byte`, `offset_bit`, `value_write`, `value_read`, `client_id`, `operation_dt`) \
VALUES('" + queryInfo.operation + "', '" + queryInfo.dataType + "', " + queryInfo.offsetByte + ", " + queryInfo.offsetBit + ", " + queryInfo.valueWrite + ", " + queryInfo.valueRead + ", " + queryInfo.clientId + ", '" + queryInfo.operationDT + "')";
				query = conn.query(sql);
				query.execute();
				cout << "A record has been inserted into the database" << endl;
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
	}
	closesocket(*clientSocket);
	free(clientSocket);
	return 0;
}

