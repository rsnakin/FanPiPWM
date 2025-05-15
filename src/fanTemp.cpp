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
