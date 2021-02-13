#include "DataConvertorUtils.h"
#include <cstring>

char* DataConvertorUtils::float2byte(float f) {
    char* b = new char[4];
    char* c = (char*)(&f);
    for (size_t i = 0; i < 4; i++) {
        b[i] = c[i];
    }
    return b;
}

float DataConvertorUtils::byte2float(char* c) {
    float f;
    memcpy(&f, c, 4);
    return f;
}

char* DataConvertorUtils::int2byte(int num) {
    char* bytes = new char[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (char)(num >> (i * 8));
    }
    return bytes;
}

int DataConvertorUtils::byte2int(char* c) {
    return ((c[3] & 0xFF) << 24) |
        ((c[2] & 0xFF) << 16) |
        ((c[1] & 0xFF) << 8) |
        ((c[0] & 0xFF) << 0);
}
