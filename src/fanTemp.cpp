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
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "fanTemp.hpp"

fanTemp::fanTemp(const char* tempPath) : isError(false), tempFiled(-1) {
    tempFiled = open(tempPath, O_RDONLY);
    if (tempFiled < 0) {
        isError = true;
        snprintf(errorMsg, sizeof(errorMsg), "Can't open %s", tempPath);
    }
}

fanTemp::~fanTemp() {
    if (tempFiled >= 0) close(tempFiled);
}

unsigned int fanTemp::get() {
    if (tempFiled < 0 || lseek(tempFiled, 0, SEEK_SET) < 0) return static_cast<unsigned int>(-1);

    char buf[16];
    ssize_t len = read(tempFiled, buf, sizeof(buf) - 1);
    if (len <= 0) return static_cast<unsigned int>(-1);
    buf[len] = '\0';

    buf[strcspn(buf, "\r\n")] = '\0';

    errno = 0;
    char* endptr;
    long raw = strtol(buf, &endptr, 10);
    if (buf == endptr || *endptr != '\0' || errno != 0 || raw <= 0) return static_cast<unsigned int>(-1);

    return static_cast<unsigned int>((raw < 1000) ? raw * 1000 : raw);
}
