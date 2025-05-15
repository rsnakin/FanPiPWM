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
        char pwmInitError[64];

        #ifdef ORANGEPI
            int orangePIPWMChip;
            unsigned long int _pwmRangeMax;
            bool setOrangePWMValue(char *fTask, char *value);
        #endif

        void setPIN(int pin);
        bool pwmInit();
        bool setPWMValue(unsigned int value);
        bool changePWMValue(unsigned int value);
        bool PWMStop();

        fanPWM();
        ~fanPWM();
    
};

