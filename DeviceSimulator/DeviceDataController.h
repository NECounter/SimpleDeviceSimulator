#pragma once


#include "MemFileHandler.h"


class DeviceDataController {
private:
	MemFileHandler* memFile;
	DeviceDataController();

public:
	DeviceDataController(MemFileHandler* memFile);
	~DeviceDataController();

	bool getBit(int offsetByte, int offsetBit);
	void setBit(int offsetByte, int offsetBit, bool value);

	int getDWord(int offset);
	void setDWord(int offset, int value);

	float getFloat(int offset);
	void setFloat(int offset, float value);









};

