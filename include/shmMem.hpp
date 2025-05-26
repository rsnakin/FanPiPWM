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

#pragma once

#define SHARED_MEMORY_OBJECT_NAME "FanPiPWMld"
#define SHARED_MEMORY_OBJECT_SIZE 36
#define MEMORY_OBJECT_TEMPLATE "%1d%06lu%03d%10lu%10d%5s"
#define getFanMode(x) ((x) ? "On " : "Off")
#define b2i(x) ((x) ? 0 : 1)

class shmMem {
    public:
        char fanMode[4];
        long unsigned int temperature;
        int pwmRange;
        int fanmode;
        char dataTime[21];
        unsigned long unixTime;
        char fanVersion[6];
        pid_t fanPID;
        bool pwmStopped;
        int pwmValue;
        bool writeResult;
        bool readResult;
        shmMem();
        ~shmMem();
        const char *getObjName();
        void setFanVersion(const char *version);
        void read();
        void write();
        void unlink();
        void get_time(time_t unix_timestamp, char *time_buf);
    private:
        ;
};

