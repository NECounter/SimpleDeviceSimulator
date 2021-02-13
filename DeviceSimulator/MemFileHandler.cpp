#include <iostream>
#include <fstream>
#include <string>

#include "MemFileHandler.h"
#include "FileUtils.h"

using namespace std;

bool MemFileHandler::indexCheck(int left, int right) {
	if (left < 0 || right >= this->memByteCount) {
		return false;
	}
	else {
		return true;
	}
}

MemFileHandler::~MemFileHandler() {
	try {
		FileUtils::createFileFromCharArray(this->memFileName, this->mem, this->memByteCount);
	}
	catch (const std::exception&) {
	}
	delete[] this->mem;
}

MemFileHandler::MemFileHandler(string memFileName, int memFileSize) {
	this->memFileName = memFileName;
	this->memByteCount = memFileSize;
	if (FileUtils::isFileExist(memFileName)) {
		if (FileUtils::getFileSize(memFileName) == memFileSize) {
			this->mem = FileUtils::readByteArrFromFile(memFileName);
			return;
		}
	}
	this->mem = new char[memFileSize] {0};
	FileUtils::createFileFromCharArray(memFileName, this->mem, memFileSize);
}

char* MemFileHandler::getMem() {
	return this->mem;
}

bool MemFileHandler::saveToMemFile() {
	try {
		FileUtils::createFileFromCharArray(this->memFileName, this->mem, this->memByteCount);
		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

char MemFileHandler::getByte(int index) {
	if (!this->indexCheck(index, index)) {
		return NULL;
	}
	return this->mem[index];
}

char* MemFileHandler::getByte(int start, int nums) {
	if (!this->indexCheck(start, start + nums - 1)) {
		return 0;
	}
	char* bytes = new char[nums];
	for (int i = start; i < start + nums; i++) {
		bytes[i - start] = this->mem[i];
	}
	return bytes;
}

bool MemFileHandler::setByte(char c, int index) {
	if (!this->indexCheck(index, index)) {
		return false;
	}
	try {
		this->mem[index] = c;
		return true;
	}
	catch (const std::exception&) {
		return false;
	}
}

bool MemFileHandler::setByte(char* c, int start, int nums) {
	if (!this->indexCheck(start, start + nums - 1)) {
		return false;
	}
	try {
		for (int i = start; i < start + nums; i++) {
			this->mem[i] = c[i - start];
		}
		return true;

	}
	catch (const std::exception&) {
		return false;
	}
}
