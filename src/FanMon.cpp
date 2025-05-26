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
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <csignal>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <chrono>
#include <termios.h>
#include <sys/ioctl.h>
#include "common.h"
#include "platform_info.h"
#include "shmMem.hpp"
#include "fanStrings.hpp"
#include "fanKbd.hpp"
#include "cfgini.hpp"

#define CUR_LINE_LEN 66
#define getTempColor(x) ((x) ? 31 : 32)

pid_t getFanPID() {
    char PIDSbuf[512] = {0};
    pid_t myPID = getpid();

    FILE* cmd_pipe = popen("pidof " APP_NAME, "r");
    if (!cmd_pipe) return -1;
    fgets(PIDSbuf, sizeof(PIDSbuf), cmd_pipe);
    pclose(cmd_pipe);

    char* token = strtok(PIDSbuf, " ");
    while (token) {
        pid_t pid = strtoul(token, nullptr, 10);
        if (pid != myPID) return pid;
        token = strtok(nullptr, " ");
    }
    return -1;
}

void ctrlC_handler(int) {}

int getUname(char* Uname) {
    FILE* cmdu_pipe = popen("whoami", "r");
    if (!cmdu_pipe) return -1;
    fgets(Uname, 1024, cmdu_pipe);
    pclose(cmdu_pipe);
    Uname[strcspn(Uname, "\r\n")] = '\0';
    return 0;
}

int main() {
    char dataTime[21];
    unsigned int temp_max, temp_min;
    unsigned long unixTimeFanOn = 0, unixTimeFanOff = 0;
    unsigned long unixTimeFanPause = 0, unixTimeFanRunn = 0;
    int fanModeFlagPrev = 0;
    pid_t fanPid;
    int color = 32;
    char hostname[1024];
    char userName[21];
    char dateMin[21], dateMax[21];
    struct sigaction sigIntHandler;
    static char curLine[CUR_LINE_LEN];
    float lineScale;
    bool appStart = true;
    int pColor = 30, rColor = 30;

    shmMem shmObj;
    fanString fStr;
    fanKbd fKbd;
    cfg config(CONFIG);

    if (config.error) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }

    fanPid = getFanPID();
    if (fanPid < 0) {
        std::cerr << "\e[31mCannot find process " << APP_NAME << "!\e[0m" << std::endl;
        return 0;
    }

    shmObj.read();
    if (!shmObj.readResult) {
        std::cerr << "\e[31mCannot find shared memory object " << shmObj.getObjName() << "!\e[0m" << std::endl;
        return 0;
    }

    int tMin = 0, tRange = 0;
    if (!config.getValue("%d", "values", "TEMPERATURE_MIN", &tMin) ||
        !config.getValue("%d", "values", "TEMPERATURE_RANGE", &tRange)) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }
    temp_max = tMin * 1000;
    temp_min = temp_max - tRange * 1000;

    shmObj.get_time(time(nullptr), dataTime);
    strncpy(dateMax, dataTime, sizeof(dateMax));
    strncpy(dateMin, dataTime, sizeof(dateMin));

    sigIntHandler.sa_handler = ctrlC_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    system("clear");
    gethostname(hostname, sizeof(hostname));
    getUname(userName);

#ifdef RASPBERRYPI
    printf("# \e[1m\e[33m%s@%s\e[0m * FanMonitor V%s /%s/ *\e[?25l \n",
           userName, hostname, VERSION, RASPBERRYPI);
#endif
#ifdef ORANGEPI
    printf("# \e[1m\e[33m%s@%s\e[0m * FanMonitor V%s /%s/ *\e[?25l \n",
           userName, hostname, VERSION, ORANGEPI);
#endif

    printf("\e[0m┌%s┐\n", fStr.repCharsStr(107, "─"));

    while (fKbd.sleep(10)) {
        shmObj.read();
        if (!shmObj.readResult) continue;

        if (shmObj.fanmode != fanModeFlagPrev || appStart) {
            unixTimeFanOn = shmObj.unixTime;
            unixTimeFanOff = shmObj.unixTime;
            appStart = false;
            fanModeFlagPrev = shmObj.fanmode;
        }

        if (shmObj.temperature >= temp_max) {
            strncpy(dateMax, shmObj.dataTime, sizeof(dateMax));
            temp_max = shmObj.temperature;
        }

        if (shmObj.temperature <= temp_min) {
            strncpy(dateMin, shmObj.dataTime, sizeof(dateMin));
            temp_min = shmObj.temperature;
        }

        color = getTempColor(shmObj.fanmode);

        if (shmObj.fanmode > 0) {
            unixTimeFanOn = shmObj.unixTime;
            unixTimeFanRunn = unixTimeFanOn - unixTimeFanOff;
            rColor = 31;
            pColor = 30;
        } else {
            unixTimeFanOff = shmObj.unixTime;
            unixTimeFanPause = unixTimeFanOff - unixTimeFanOn;
            pColor = 32;
            rColor = 30;
        }

        int temp_range = temp_max - temp_min;
        if (temp_range > 0) {
            lineScale = static_cast<float>(CUR_LINE_LEN - 1) / temp_range;
            for (int i = 0; i < CUR_LINE_LEN - 1; ++i) {
                curLine[i] = (i <= static_cast<int>(lineScale * (shmObj.temperature - temp_min))) ? '|' : ' ';
            }
        }
        curLine[CUR_LINE_LEN - 1] = '\0';

        printf("│ %s -> TMax: \e[30m\e[41m %2.3f°C \e[0m ┌%s┤\n",
               dateMax, temp_max / 1000.0, fStr.repCharsStr(65, "─"));
        printf("│ %s -> Temp: \e[%dm\e[47m %2.3f°C \e[0m │\e[%dm\e[1m%s\e[0m│\n",
               shmObj.dataTime, color, shmObj.temperature / 1000.0, color, curLine);
        printf("│ %s -> TMin: \e[30m\e[42m %2.3f°C \e[0m └%s┤\n",
               dateMin, temp_min / 1000.0, fStr.repCharsStr(65, "─"));
        printf("├%s┤\n", fStr.repCharsStr(107, "─"));

        printf("│ \e[40m\e[38mMode: \e[47m\e[30m %3s \e[0m\e[40m\e[38m Power: \e[47m\e[30m %3d%%",
               shmObj.fanMode, shmObj.pwmRange);
        printf(" \e[0m\e[40m\e[38m Run: \e[47m\e[%dm %12s \e[0m\e[40m\e[38m",
               rColor, fStr.cnvTimeLine(unixTimeFanRunn));
        printf(" Pause: \e[47m\e[%dm %12s \e[0m\e[40m\e[38m FanVersion: \e[47m\e[30m %5s",
               pColor, fStr.cnvTimeLine(unixTimeFanPause), shmObj.fanVersion);
        printf(" \e[0m\e[40m\e[38m FanPID: \e[47m\e[30m %7d \e[0m │\n", shmObj.fanPID);
        printf("└%s┘\n\e[6F", fStr.repCharsStr(107, "─"));
    }

    tcflush(0, TCIFLUSH);
    system("clear");
    printf("\e[?25h");
    return 0;
}
