#pragma once
static class DataConvertorUtils {
public:
	static char* float2byte(float f);
	static float byte2float(char* c);
	static char* int2byte(int num);
	static int byte2int(char* c);
};
