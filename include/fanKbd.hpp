#pragma once
#include <pthread.h>

class fanKbd {
public:
    fanKbd();
    ~fanKbd();

    bool sleep(int delay);

private:
    pthread_t gKbdTr;
    bool firstTime;

    static bool kbhit();
    static void raw_mode(bool enable);
    static void* getKbd(void*);
};
