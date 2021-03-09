#pragma once

#include <string>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

class FileUtils
{
public:
	static bool isFileExist(std::string fileName);
	static int getFileSize(std::string fileName);
	static void createFileFromCharArray(std::string fileName, char *charArr, int fileSize);
	static void createEmptyFile(std::string fileName, int fileSize);
	static char *readByteArrFromFile(std::string fileName);
	static void logWrite(std::string msg, std::string logPath);
};
