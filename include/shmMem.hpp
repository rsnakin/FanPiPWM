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

