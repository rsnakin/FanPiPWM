/*
MIT License

Copyright (c) 2025 Richard Snakin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
