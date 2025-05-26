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

#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include "platform_info.h"
#include "fanPWM.hpp"

fanPWM::fanPWM() : pwmPIN(0), hardwarePWM(false), wiringPiStatus(wiringPiSetup()), prewPWMValue(0) {
#ifdef ORANGEPI
    orangePIPWMClock = PWM_CLOCK;
#endif
#ifdef DEBUG_LOG
    log.setLogPath(DEBUG_LOG);
#endif
}

fanPWM::~fanPWM() {
    if (hardwarePWM) {
        PWMStop();
    } else {
        softPwmStop(pwmPIN);
    }
}

int fanPWM::PWMmap(int currentTemp) {
    if (currentTemp < tempMin) return pwmRangeMin;
    if (currentTemp > tempMax) return pwmRangeMax;
    return (currentTemp - tempMin) * (pwmRangeMax - pwmRangeMin) / (tempMax - tempMin) + pwmRangeMin;
}

void fanPWM::setPIN(int pin) {
    pwmPIN = pin;
#ifdef ORANGEPI
    orangePIPWMChip = 0;
    if (pwmPIN == 2) {
        orangePIPWMPin = 0;
    }
#endif
}

bool fanPWM::pwmInit() {
    if (!pwmPIN) {
        snprintf(pwmInitError, sizeof(pwmInitError), "pwmPIN undefined!");
        return false;
    }

    prewPWMValue = 0;

    if (!hardwarePWM) {
#ifdef ORANGEPI
        pinMode(pwmPIN, SOFT_PWM_OUTPUT);
#endif
#ifdef RASPBERRYPI
        pinMode(pwmPIN, PWM_OUTPUT);
#endif
        digitalWrite(pwmPIN, LOW);
        return true;
    } else if (pwmMode && pwmClock && pwmRangeMax) {
#ifdef ORANGEPI
        return orangePIHwPWMInit();
#endif
#ifdef RASPBERRYPI
        pinMode(pwmPIN, OUTPUT);
        digitalWrite(pwmPIN, LOW);
        pinMode(pwmPIN, PWM_OUTPUT);
        pwmSetMode(pwmMode);
        pwmSetClock(pwmClock);
        pwmSetRange(pwmRangeMax);
        pwmWrite(pwmPIN, 0);
        return true;
#endif
    }

    snprintf(pwmInitError, sizeof(pwmInitError), "pwmMode: [%u] pwmClock: [%u] pwmRangeMax: [%u]", pwmMode, pwmClock, pwmRangeMax);
    return false;
}

bool fanPWM::PWMStop() {
    if (!pwmPIN) return false;

    prewPWMValue = 0;

    if (hardwarePWM) {
#ifdef ORANGEPI
        return __pinDutyCycle(0) && __pinEnable(true);
#endif
#ifdef RASPBERRYPI
        pwmWrite(pwmPIN, 0);
        return true;
#endif
    } else {
        softPwmStop(pwmPIN);
        return true;
    }

    return false;
}

bool fanPWM::changePWMValue(unsigned int value) {
    if (!pwmPIN || prewPWMValue == value) return value == prewPWMValue;

    prewPWMValue = value;

    if (hardwarePWM) {
#ifdef ORANGEPI
        unsigned int _pwmValue = value * orangePIPWMClock;
        if (_pwmValue > orangePIPWMClock * 100) return false;
        return __pinDutyCycle(_pwmValue) && __pinEnable(true);
#endif
#ifdef RASPBERRYPI
        pwmWrite(pwmPIN, value);
        return true;
#endif
    } else {
        softPwmWrite(pwmPIN, value);
        return true;
    }

    return false;
}

bool fanPWM::setPWMValue(unsigned int value) {
    if (!pwmPIN) return false;

    if (hardwarePWM) {
#ifdef ORANGEPI
        unsigned int _pwmValue = value * orangePIPWMClock;
        if (_pwmValue > orangePIPWMClock * 100) return false;
        return __pinDutyCycle(_pwmValue) && __pinEnable(true);
#endif
#ifdef RASPBERRYPI
        pwmWrite(pwmPIN, value);
        return true;
#endif
    } else {
        if (value > pwmRangeMax) return false;
        softPwmCreate(pwmPIN, value, pwmRangeMax);
        return true;
    }

    return false;
}

#ifdef ORANGEPI
bool fanPWM::setOrangePWMValue(char *fTask, char *value) {
    FILE* task = fopen(fTask, "w");
    if (!task) {
#ifdef DEBUG_LOG
        log.write("[False] %s -> %s", value, fTask);
#endif
        return false;
    }
    fprintf(task, "%s", value);
    fclose(task);
#ifdef DEBUG_LOG
    log.write("[True] %s -> %s", value, fTask);
#endif
    return true;
}

bool fanPWM::orangePIHwPWMInit() {
    _pwmRangeMax = pwmRangeMax * orangePIPWMClock;
    if (_pwmRangeMax > orangePIPWMClock * 100) return false;
    return __pinExport(orangePIPWMPin) && __pinPolarityNormal(true) && __pinPeriod(_pwmRangeMax) && __pinDutyCycle(0) && __pinEnable(true);
}

bool fanPWM::__pinEnable(bool enable) {
    char task[64];
    snprintf(task, sizeof(task), "/sys/class/pwm/pwmchip%d/pwm%d/enable", orangePIPWMChip, orangePIPWMPin);
    return setOrangePWMValue(task, (char *)(enable ? "1" : "0"));
}

bool fanPWM::__pinDutyCycle(unsigned int value) {
    char task[64], valueStr[16];
    snprintf(task, sizeof(task), "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle", orangePIPWMChip, orangePIPWMPin);
    snprintf(valueStr, sizeof(valueStr), "%u", value);
    return setOrangePWMValue(task, valueStr);
}

bool fanPWM::__pinPeriod(unsigned long int value) {
    char task[64], valueStr[32];
    snprintf(task, sizeof(task), "/sys/class/pwm/pwmchip%d/pwm%d/period", orangePIPWMChip, orangePIPWMPin);
    snprintf(valueStr, sizeof(valueStr), "%lu", value);
    return setOrangePWMValue(task, valueStr);
}

bool fanPWM::__pinPolarityNormal(bool normal) {
    char task[64];
    snprintf(task, sizeof(task), "/sys/class/pwm/pwmchip%d/pwm%d/polarity", orangePIPWMChip, orangePIPWMPin);
    return setOrangePWMValue(task, (char *)(normal ? "normal" : "inversed"));
}

bool fanPWM::__pinExport(int orangePIPin) {
    char task[64], pinStr[8];
    snprintf(task, sizeof(task), "/sys/class/pwm/pwmchip%d/export", orangePIPWMChip);
    snprintf(pinStr, sizeof(pinStr), "%d", orangePIPin);
    return setOrangePWMValue(task, pinStr);
}
#endif
