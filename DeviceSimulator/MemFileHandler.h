#pragma once

#include <string>


class MemFileHandler {
private:
	/// <summary>
	/// simulate the RAM
	/// </summary>
	char* mem;
	/// <summary>
	/// size of the RAM
	/// </summary>
	int memByteCount;
	/// <summary>
	/// the presisitance file
	/// </summary>
	std::string memFileName;
	/// <summary>
	/// avoid invalid memory access
	/// </summary>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <returns></returns>
	bool indexCheck(int left, int right);
	/// <summary>
	/// none para constructor is not allowed
	/// </summary>
	MemFileHandler();
	

public:
	/// <summary>
	/// use file name and file size to construct
	/// </summary>
	/// <param name="memFileName"></param>
	/// <param name="memFiileCount"></param>
	MemFileHandler(std::string memFileName, int memFileSize);
	~MemFileHandler();
	/// <summary>
	/// get the pointer of the memfile
	/// </summary>
	/// <returns></returns>
	char* getMem();
	/// <summary>
	/// presistent the mem to the local disk
	/// </summary>
	/// <returns></returns>
	bool saveToMemFile();
	/// <summary>
	/// get byte via index
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	char getByte(int index);
	/// <summary>
	/// get bytes via range [start, start + nums)
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <returns></returns>
	char* getByte(int start, int nums);
	/// <summary>
	/// set byte via index
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	bool setByte(char c, int index);
	/// <summary>
	/// set bytes via range [start, start + nums)
	/// </summary>
	/// <param name="start"></param>
	/// <param name="nums"></param>
	/// <returns></returns>
	bool setByte(char* c, int start, int nums);
};
