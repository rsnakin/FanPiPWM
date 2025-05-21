#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "platform_info.h"
#include "shmMem.hpp"
#include "Log.hpp"
#include "fanPWM.hpp"
#include "cfgini.hpp"
#include "fanTemp.hpp"

int getUname(char* Uname) {
    FILE* pipe = popen("whoami", "r");
    if (!pipe) return -1;
    fgets(Uname, 1024, pipe);
    pclose(pipe);
    Uname[strcspn(Uname, "\r\n")] = '\0';
    return 0;
}

void killOtherInstances(const char *app_name, int timeout_ms) {
    char buffer[512];
    pid_t my_pid = getpid();

    char command[128];
    snprintf(command, sizeof(command), "pidof %s", app_name);

    FILE *pipe = popen(command, "r");
    if (!pipe || !fgets(buffer, sizeof(buffer), pipe)) {
        if (pipe) pclose(pipe);
        fprintf(stderr, "Failed to get PID list for %s\n", app_name);
        return;
    }
    pclose(pipe);

    char *token = strtok(buffer, " \n");
    while (token) {
        pid_t pid = strtoul(token, NULL, 10);
        if (pid > 0 && pid != my_pid) {
            printf("Old PID found, kill \e[31m%d\e[0m ... ", pid);
            if (kill(pid, SIGTERM) == 0) {
                printf("\e[33m[term]\e[0m ");
                usleep(timeout_ms * 1000);
                if (kill(pid, 0) == 0) {
                    if (kill(pid, SIGKILL) == 0) {
                        printf("\e[31m[kill]\e[0m\n");
                    } else {
                        printf("\e[31m[kill error]\e[0m\n");
                    }
                } else {
                    printf("\e[32m[done]\e[0m\n");
                }
            } else {
                printf("\e[31m[term error]\e[0m\n");
            }
        }
        token = strtok(NULL, " \n");
    }
}

int main() {
    char hostname[1024];
    char uName[32];
    shmMem shmObj;
    Log log;
    fanPWM fPWM;

    if (fPWM.wiringPiStatus != 0) {
        std::cerr << "Error: wiringPiStatus = " << fPWM.wiringPiStatus << std::endl;
        return 0;
    }

    cfg config(CONFIG);
    if (config.error) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }

    shmObj.setFanVersion(VERSION);
    gethostname(hostname, sizeof(hostname));
    getUname(uName);

    if (strcmp(uName, "root") != 0) {
        std::cerr << "\n\e[31mOnly root can run " << APP_NAME << "!\e[0m\n\n";
        return 1;
    }

    pid_t myPID = getpid();
    printf("\n* \e[33m%s@%s : %s V%s\e[0m * \n", uName, hostname, APP_NAME, VERSION);
    printf("\nConfig file: \e[33m%s\e[0m\n\n", CONFIG);

    int pin = 0;
    if (!config.getValue("%d", "hardware", "PIN", &pin)) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }
    fPWM.setPIN(pin);

    if (!config.getValue("%u", "hardware", "PWM_RANGE_MAX", &fPWM.pwmRangeMax) ||
        !config.getValue("%u", "hardware", "PWM_RANGE_MIN", &fPWM.pwmRangeMin)) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }

    int initPWMIntensity = 0;
    config.getValue("%d", "hardware", "INIT_PWM_INTENSITY", &initPWMIntensity);

    int tMax, tMin, tRange;
    if (!config.getValue("%d", "values", "TEMPERATURE_MAX", &tMax) ||
        !config.getValue("%d", "values", "TEMPERATURE_MIN", &tMin) ||
        !config.getValue("%d", "values", "TEMPERATURE_RANGE", &tRange)) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }

    fPWM.tempMax = static_cast<long unsigned int>(tMax * 1000);
    fPWM.tempMin = static_cast<long unsigned int>(tMin * 1000);
    unsigned int tempRange = tRange * 1000;

    char path2TempData[64];
    if (!config.getValue("%s", "files", "PATH_TO_TEMP_DATA", path2TempData)) {
        std::cerr << config.errorMsg << std::endl;
        return 0;
    }

    char path2Log[64];
    if (config.getValue("%s", "files", "PATH_TO_LOG", path2Log)) {
        log.setLogPath(path2Log);
    }

    if (config.getValue("%b", "hardware", "HARDWARE_PWM", &fPWM.hardwarePWM) && fPWM.hardwarePWM) {
        config.getValue("%u", "hardware", "HARDWARE_PWM_CLOCK", &fPWM.pwmClock);
        char hPWMMode[14];
        if (config.getValue("%s", "hardware", "HARDWARE_PWM_MODE", hPWMMode)) {
            fPWM.pwmMode = strcmp(hPWMMode, "PWM_MODE_BAL") == 0 ? 1 : 0;
        }
    }

#ifdef RASPBERRYPI
    printf("\e[36mBOARD:\t\t\t\e[35m%s\e[0m\n", RASPBERRYPI);
#endif
#ifdef ORANGEPI
    printf("\e[36mBOARD:\t\t\t\e[35m%s\e[0m\n", ORANGEPI);
#endif
    log.write("***");
    log.write("*** %s V%s started ***", APP_NAME, VERSION);
    log.write("***");
    log.write("PIN: %d", fPWM.pwmPIN);
    printf("\e[36mPIN:\t\t\t\e[35m%d\e[0m\n", fPWM.pwmPIN);
    printf("\e[36mHARDWARE_PWM:\t\t\e[35m%s\e[0m\n", fPWM.hardwarePWM ? "True" : "False");
    if (fPWM.hardwarePWM) {
      printf("\e[36mHARDWARE_PWM_CLOCK:\t\e[35m%d\e[0m\n", fPWM.pwmClock);
      printf("\e[36mHARDWARE_PWM_MODE:\t\e[35m%s\e[0m\n", fPWM.pwmMode ? "PWM_MODE_BAL" : "PWM_MODE_MS");
      log.write("HARDWARE_PWM: True");
      log.write("HARDWARE_PWM_CLOCK: %d", fPWM.pwmClock);
      log.write("HARDWARE_PWM_MODE: %s", (fPWM.pwmMode ? "PWM_MODE_BAL" : "PWM_MODE_MS"));
    } else {
      printf("\e[36mHARDWARE_PWM:\t\t\e[35mFalse\e[0m\n");
      log.write("HARDWARE_PWM: False");
    }
    printf("\e[36mPWM_RANGE_MAX:\t\t\e[35m%d\e[0m\n", fPWM.pwmRangeMax);
    printf("\e[36mPWM_RANGE_MIN:\t\t\e[35m%d\e[0m\n", fPWM.pwmRangeMin);
    printf("\e[36mINIT_PWM_INTENSITY:\t\e[35m%d\e[0m\n", initPWMIntensity);
    printf("\e[36mTEMPERATURE_MAX:\t\e[35m%lu\e[0m\n", fPWM.tempMax);
    printf("\e[36mTEMPERATURE_MIN:\t\e[35m%lu\e[0m\n", fPWM.tempMin);
    printf("\e[36mTEMPERATURE_RANGE:\t\e[35m%d\e[0m\n", tempRange);
    printf("\e[36mPATH_TO_TEMP_DATA:\t\e[35m%s\e[0m\n", path2TempData);
    if (log.On) {
        printf("\e[36mPATH_TO_LOG:\t\t\e[35m%s\e[0m\n", path2Log);
        log.write("PWM_RANGE_MAX: %d", fPWM.pwmRangeMax);
        log.write("PWM_RANGE_MIN: %d", fPWM.pwmRangeMin);
        log.write("INIT_PWM_INTENSITY: %d", initPWMIntensity);
        log.write("TEMPERATURE_MAX: %d", fPWM.tempMax);
        log.write("TEMPERATURE_MIN: %d", fPWM.tempMin);
        log.write("TEMPERATURE_RANGE: %d", tempRange);
        log.write("PATH_TO_TEMP_DATA: %s", path2TempData);
    }

    killOtherInstances(APP_NAME, 1000);

    fanTemp fTemp(path2TempData);
    if (fTemp.isError) {
        std::cerr << "Error: " << fTemp.errorMsg << "...\nExit..." << std::endl;
        return 1;
    }

    // Fork
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "\e[31mError: Start Daemon failed\e[0m\n";
        log.write("Error: Start Daemon failed\n");
        return -1;
    } else if (pid == 0) {
        setsid();
        chdir("/");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        config.~cfg();

        shmObj.fanPID = getpid();
        log.write("PID: %d", shmObj.fanPID);

        try {
            if (!fPWM.pwmInit()) {
                log.write("Error pwmInit!");
                log.write("%s", fPWM.pwmInitError);
                log.write("Exit...");
                return 0;
            }

            shmObj.unlink();
            while (true) {
                shmObj.temperature = fTemp.get();
                if (shmObj.temperature < 0) {
                    usleep(1000000);
                    continue;
                }
                //shmObj.pwmValue = PWMmap(shmObj.temperature, TEMPERATURE_MIN, TEMPERATURE_MAX, fPWM.pwmRangeMin, fPWM.pwmRangeMax);
                shmObj.pwmValue = fPWM.PWMmap(shmObj.temperature);
                shmObj.write();

                if (shmObj.temperature >= fPWM.tempMin && shmObj.pwmStopped) {
                    useconds_t initTimeout = 0;
                    if (initPWMIntensity) {
                        shmObj.pwmValue = std::min(100U, static_cast<unsigned int>(shmObj.pwmValue + initPWMIntensity));
                        initTimeout = 200000;
                    }
                    fPWM.setPWMValue(shmObj.pwmValue);
                    log.write("Fan is ON, temp: %f°C, PWMValue: %d", shmObj.temperature / 1000.0, shmObj.pwmValue);
                    shmObj.pwmStopped = false;
                    usleep(initTimeout);
                } else if (shmObj.temperature >= (fPWM.tempMin - tempRange) && !shmObj.pwmStopped) {
                    fPWM.changePWMValue(shmObj.pwmValue);
                } else if (!shmObj.pwmStopped) {
                    fPWM.PWMStop();
                    log.write("Fan is OFF, temp: %f°C", shmObj.temperature / 1000.0);
                    shmObj.pwmStopped = true;
                }
                usleep(1000000);
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    char PIDSbuf[512];
    FILE *cmd_pipe_new = popen("pidof " APP_NAME, "r");
    if (cmd_pipe_new && fgets(PIDSbuf, sizeof(PIDSbuf), cmd_pipe_new)) {
        pclose(cmd_pipe_new);

        char *token = strtok(PIDSbuf, " \n");
        while (token) {
            pid_t pid = strtoul(token, NULL, 10);
            if (pid != myPID) {
              printf("New PID \e[32m%d\e[0m\n", pid);
            }
            token = strtok(NULL, " \n");
        }
    } else {
        if (cmd_pipe_new) pclose(cmd_pipe_new);
        fprintf(stderr, "Failed to read PID list\n");
    }

    printf("\n");
    return 0;
}
