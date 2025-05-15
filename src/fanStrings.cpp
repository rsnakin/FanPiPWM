#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "fanStrings.hpp"

fanString::fanString() : __fanString(nullptr), fanStringLen(0) {}

char* fanString::repCharsStr(size_t amt, const char *sim) {
    if (__fanString) free(__fanString);
    
    size_t simLen = strlen(sim);
    fanStringLen = simLen * amt + 1; // +1 for null terminator
    __fanString = static_cast<char *>(malloc(fanStringLen));
    if (!__fanString) return nullptr;

    char *ptr = __fanString;
    for (size_t i = 0; i < amt; ++i) {
        memcpy(ptr, sim, simLen);
        ptr += simLen;
    }
    *ptr = '\0';
    return __fanString;
}

char* fanString::cnvTimeLine(unsigned long unixTime) {
    if (__fanString) free(__fanString);
    
    fanStringLen = 30;
    __fanString = static_cast<char *>(malloc(fanStringLen));
    if (!__fanString) return nullptr;

    int fMin = unixTime / 60;
    int fSec = unixTime % 60;

    snprintf(__fanString, fanStringLen, "%3dmin %02dsec", fMin, fSec);
    return __fanString;
}

fanString::~fanString() {
    if (__fanString) free(__fanString);
}
