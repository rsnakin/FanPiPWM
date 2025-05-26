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

#include <cstdlib>
#include <csignal>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctime>
#include "shmMem.hpp"

shmMem::shmMem() : pwmStopped(true), readResult(false), writeResult(false) {}

shmMem::~shmMem() = default;

const char* shmMem::getObjName() {
    return SHARED_MEMORY_OBJECT_NAME;
}

void shmMem::setFanVersion(const char* version) {
    snprintf(fanVersion, sizeof(fanVersion), "%5s", version);
}

void shmMem::read() {
    int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDONLY, 0777);
    if (shm == -1) {
        readResult = false;
        return;
    }

    void* map = mmap(nullptr, SHARED_MEMORY_OBJECT_SIZE, PROT_READ, MAP_SHARED, shm, 0);
    if (map == MAP_FAILED) {
        close(shm);
        readResult = false;
        return;
    }

    const char* msg = static_cast<const char*>(map);
    sscanf(msg, MEMORY_OBJECT_TEMPLATE, &fanmode, &temperature, &pwmRange, &unixTime, &fanPID, fanVersion);
    snprintf(fanMode, sizeof(fanMode), "%s", getFanMode(fanmode));
    get_time(unixTime, dataTime);

    munmap(map, SHARED_MEMORY_OBJECT_SIZE);
    close(shm);
    readResult = true;
}

void shmMem::write() {
    int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT | O_RDWR, 0777);
    if (shm == -1) {
        writeResult = false;
        return;
    }

    if (ftruncate(shm, SHARED_MEMORY_OBJECT_SIZE) == -1) {
        close(shm);
        writeResult = false;
        return;
    }

    void* map = mmap(nullptr, SHARED_MEMORY_OBJECT_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);
    if (map == MAP_FAILED) {
        close(shm);
        writeResult = false;
        return;
    }

    char* addr = static_cast<char*>(map);
    int written = snprintf(addr, SHARED_MEMORY_OBJECT_SIZE, MEMORY_OBJECT_TEMPLATE,
                           b2i(pwmStopped), temperature,
                           (pwmStopped ? 0 : pwmValue), std::time(nullptr), fanPID, fanVersion);
    if (written >= 0 && written < SHARED_MEMORY_OBJECT_SIZE) {
        addr[written] = '\0';
        writeResult = true;
    }

    munmap(map, SHARED_MEMORY_OBJECT_SIZE);
    close(shm);
}

void shmMem::unlink() {
    shm_unlink(SHARED_MEMORY_OBJECT_NAME);
}

void shmMem::get_time(time_t unix_timestamp, char* time_buf) {
    struct tm ts = *localtime(&unix_timestamp);
    strftime(time_buf, 21, "%d-%m-%Y %H:%M:%S", &ts);
}
