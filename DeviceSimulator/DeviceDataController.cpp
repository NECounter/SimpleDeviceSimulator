#include "DeviceDataController.h"
#include "DataConvertorUtils.h"

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
		return NULL;
	}
}

void DeviceDataController::setBit(int offsetByte, int offsetBit, bool value) {
	if (offsetBit >= 0 && offsetBit <= 7) {
		char oriByte = this->memFile->getByte(offsetByte);
		char temp = 1 << offsetBit;
		if ((oriByte & (temp)) == 0) {
			if (value) {
				this->memFile->setByte(oriByte + temp, offsetByte);
			}
			
		}
		else {
			if (!value) {
				this->memFile->setByte(oriByte - temp, offsetByte);
			}	
		}
	}
}

int DeviceDataController::getDWord(int offset) {
	return DataConvertorUtils::byte2int(this->memFile->getByte(offset, 4));
}

void DeviceDataController::setDWord(int offset, int value) {
	this->memFile->setByte(DataConvertorUtils::int2byte(value), offset, 4);
}

float DeviceDataController::getFloat(int offset) {
	return DataConvertorUtils::byte2float(this->memFile->getByte(offset, 4));
}

void DeviceDataController::setFloat(int offset, float value) {
	this->memFile->setByte(DataConvertorUtils::float2byte(value), offset, 4);
}

bool DeviceDataController::save()
{
	return this->memFile->saveToMemFile();
}
