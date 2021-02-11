CC:=g++-9
CXXFLAGS := -I/usr/include/mysql -I/usr/local/include/mysql++
LDFLAGS := -L/usr/local/lib
LDLIBS := -lmysqlpp -lmysqlclient

ServerSource:=DataConvertorUtils.cpp DeviceDataController.cpp DeviceSimulator.cpp DeviceSimulatorServer.cpp FileUtils.cpp MemFileHandler.cpp

.PHONY: Client.out DeviceSimulator.out
DeviceSimulator.out:$(ServerSource)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	
Client.out:Client.cpp
	$(CC) $^ -o $@

