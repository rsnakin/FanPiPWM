#include "fanKbd.hpp"
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <termios.h>
#include <atomic>

static std::atomic<bool> active{true};

fanKbd::fanKbd() : firstTime(true) {
    raw_mode(true);
    pthread_create(&gKbdTr, nullptr, getKbd, nullptr);
}

fanKbd::~fanKbd() {
    pthread_cancel(gKbdTr);
    pthread_join(gKbdTr, nullptr);
    raw_mode(false);
}

bool fanKbd::sleep(int delay) {
    if (firstTime) {
        firstTime = false;
        return true;
    }
    for (int i = 0; i < delay; ++i) {
        if (!active.load()) return false;
        usleep(100000);
    }
    return true;
}

bool fanKbd::kbhit() {
    int byteswaiting = 0;
    ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

void fanKbd::raw_mode(bool enable) {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    if (enable) {
        term.c_lflag &= ~(ICANON | ECHO);
    } else {
        term.c_lflag |= (ICANON | ECHO);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void* fanKbd::getKbd(void*) {
    while (true) {
        if (kbhit()) {
            int ch = getchar();
            if (ch == 'q') {
                active.store(false);
                break;
            }
        }
        usleep(100000);
    }
    return nullptr;
}
