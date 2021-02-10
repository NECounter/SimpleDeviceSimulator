// DeviceSimulator.cpp : "main" included
//

#include "DeviceSimulator.h"

using namespace std;

int main() {
	DeviceSimulatorServer server(port, databaseHostName, databaseName, databaseUserName, databasePasswd);
    server.Bind();
    server.Listen();
    server.Run();
    return 0;
}