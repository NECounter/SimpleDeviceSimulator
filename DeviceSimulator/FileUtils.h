#pragma once

#include <string>

class FileUtils {
public:
	static bool isFileExist(std::string fileName);
	static int getFileSize(std::string fileName);
	static void createFileFromCharArray(std::string fileName, char* charArr, int fileSize);
	static void createEmptyFile(std::string fileName, int fileSize);
	static char* readByteArrFromFile(std::string fileName);
};
