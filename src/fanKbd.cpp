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
