#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "FileUtils.h"

bool FileUtils::isFileExist(std::string fileName) {
    struct stat buffer;
    return stat(fileName.c_str(), &buffer) == 0;
}

int FileUtils::getFileSize(std::string fileName) {
    struct stat buffer;
    stat(fileName.c_str(), &buffer);
    return buffer.st_size;  
}

void FileUtils::createFileFromCharArray(std::string fileName, char* charArr, int fileSize) {
    std::ofstream binIO(fileName, std::ios::binary);
    binIO.write(charArr, fileSize);
    binIO.close();
}

void FileUtils::createEmptyFile(std::string fileName, int fileSize) {
    char* initZero = new char[fileSize] {0};
    std::ofstream binIO(fileName, std::ios::binary);
    binIO.write(initZero, fileSize);
    delete[] initZero;
    binIO.close();
}

char* FileUtils::readByteArrFromFile(std::string fileName) {
    std::ifstream binIO(fileName, std::ios::binary);
    struct stat buffer;
    stat(fileName.c_str(), &buffer);
    const int fileSize = buffer.st_size;

    char* s = new char[fileSize] {0};
    binIO.read(reinterpret_cast<char*>(s), fileSize);
  
    binIO.close();
    return s;
}
