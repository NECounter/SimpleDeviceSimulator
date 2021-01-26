// DeviceSimulator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include "MemFileHandler.h"
#include "DeviceDataController.h"
#include "FileUtils.h"
#include "DataConvertorUtils.h"

using namespace std;

int main() {
    MemFileHandler* mem = new MemFileHandler("mem.dat", 2 * 1024 * 1024);
    DeviceDataController* dataController = new DeviceDataController(mem);
    dataController->setBit(0, 1, 0);

    cout << dataController->getBit(0, 1) << endl;
    mem->saveToMemFile();

    //char* a = mem->getByte(0,10);
    //cout << (int)a[0] << endl;
    //a[0] = 10;
    //a[1] = 100;
    //mem->setByte(a, 0, 10);
    //char aa = mem->getByte(1);
    //cout << (int)aa << endl;

    //mem->saveToMemFile();
    //char* b = DataConvertorUtils::float2byte(1.234f);
    //cout << DataConvertorUtils::byte2float(b) << endl;
    //delete[] b;
    //char* c = DataConvertorUtils::int2byte(-1234234);
    //cout << DataConvertorUtils::byte2int(c) << endl;
    //delete[] c;


}

