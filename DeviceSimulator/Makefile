CC:=g++-9
CXXFLAGS := -I/usr/include/mysql -I/usr/local/include/mysql++
LDFLAGS := -L/usr/local/lib
LDLIBS := -lmysqlpp -lmysqlclient -pthread -std=c++17

ServerSource:=DataConvertorUtils.cpp DeviceDataController.cpp DeviceSimulatorServer.cpp FileUtils.cpp MemFileHandler.cpp

ServerSource_Active:=DataConvertorUtils.cpp DeviceDataController.cpp DeviceSimulatorServer_Active.cpp FileUtils.cpp MemFileHandler.cpp

.PHONY: Client DeviceSimulator DeviceSimulator_Active
DeviceSimulator:$(ServerSource)
	$(CC) -g $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

DeviceSimulator_Active:$(ServerSource_Active)
	$(CC) -g $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	
Client:ClientMT.cpp
	$(CC) $^ -o $@ -std=c++17 -pthread

