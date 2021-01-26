#include "DeviceDataController.h"

DeviceDataController::DeviceDataController(MemFileHandler* memFile) {
	this->memFile = memFile;
}

DeviceDataController::~DeviceDataController() {
	delete this->memFile;
}

bool DeviceDataController::getBit(int offsetByte, int offsetBit) {
	if (offsetBit >=0 && offsetBit <= 7) {
		return (this->memFile->getByte(offsetByte) & (1 << offsetBit)) != 0;
	}
	else {
		throw;
	}
}

void DeviceDataController::setBit(int offsetByte, int offsetBit, bool value) {
	if (offsetBit >= 0 && offsetBit <= 7) {
		char oriByte = this->memFile->getByte(offsetByte);
		char temp = 1 << offsetBit;
		if ((oriByte & (temp)) == 0) {
			this->memFile->setByte(oriByte + temp, offsetByte);
		}
		else {
			this->memFile->setByte(oriByte - temp, offsetByte);
		}
	}
	else {
		throw;
	}
}

int DeviceDataController::getDWord(int offset) {
	return 0;
}

void DeviceDataController::setDWord(int offset, int value) {
	
}

float DeviceDataController::getFloat(int offset) {
	return 0.0f;
}

void DeviceDataController::setFloat(int offset, float value) {
	
}
