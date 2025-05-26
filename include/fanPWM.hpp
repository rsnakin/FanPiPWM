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

#ifdef ORANGEPI
    //#define DEBUG_LOG "/var/log/FanPWMDebug.log"
    #define PWM_CLOCK 99000
#endif

class fanPWM {

    private:

        unsigned int prewPWMValue;
        #ifdef ORANGEPI
            int orangePIPWMPin;
            unsigned long int orangePIPWMClock;
            bool setOrangePWMValue(const char *fTask, const char *value);
            bool orangePIHwPWMInit();
            bool __pinEnable(bool enable);
            bool __pinDutyCycle(unsigned int value);
            bool __pinPeriod(unsigned long int value);
            bool __pinPolarityNormal(bool normal);
            bool __pinExport(int orangePIPin);
        #endif
        #ifdef DEBUG_LOG
            fanLog log;
        #endif
    public:

        int pwmPIN;
        bool hardwarePWM;
        int wiringPiStatus;
        unsigned int pwmMode;
        unsigned int pwmClock;
        unsigned int pwmRangeMax;
        unsigned int pwmRangeMin;
        long unsigned int tempMin;
        long unsigned int tempMax;
        char pwmInitError[64];

        #ifdef ORANGEPI
            int orangePIPWMChip;
            unsigned long int _pwmRangeMax;
            bool setOrangePWMValue(char *fTask, char *value);
        #endif

        void setPIN(int pin);
        bool pwmInit();
        int PWMmap(int currentTemp);
        bool setPWMValue(unsigned int value);
        bool changePWMValue(unsigned int value);
        bool PWMStop();

        fanPWM();
        ~fanPWM();
    
};

