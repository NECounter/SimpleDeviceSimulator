CC:=g++-9
CXXFLAGS := -I/usr/include/mysql -I/usr/local/include/mysql++
LDFLAGS := -L/usr/local/lib
LDLIBS := -lmysqlpp -lmysqlclient -pthread -std=c++17

ServerSource:=DataConvertorUtils.cpp DeviceDataController.cpp DeviceSimulatorServer.cpp FileUtils.cpp MemFileHandler.cpp

.PHONY: Client.out DeviceSimulator.out
DeviceSimulator.out:$(ServerSource)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	
Client.out:Client.cpp
	$(CC) $^ -o $@

