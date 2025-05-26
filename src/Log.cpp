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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include "Log.hpp"

Log::Log() : On(false), len(0), writeResult(false), __logPath(nullptr) {}

Log::~Log() {
    free(__logPath);
}

void Log::setLogPath(const char *logPath) {
    free(__logPath);
    len = strlen(logPath) + 1;
    __logPath = static_cast<char *>(malloc(len));
    if (__logPath) {
        strcpy(__logPath, logPath);
        On = true;
    } else {
        On = false;
    }
}

void Log::write(const char *format, ...) {
    writeResult = false;
    if (!On || !__logPath) return;

    FILE* flog = fopen(__logPath, "a");
    if (!flog) return;

    char cDate[21];
    time_t unixTime = time(nullptr);
    struct tm ts = *localtime(&unixTime);
    strftime(cDate, sizeof(cDate), "%d-%m-%Y %H:%M:%S", &ts);
    fprintf(flog, "[%s]: ", cDate);

    va_list args;
    va_start(args, format);

    for (const char *p = format; *p; ++p) {
        if (*p != '%') {
            fputc(*p, flog);
            continue;
        }

        switch (*++p) {
            case 'd':
                fprintf(flog, "%d", va_arg(args, int));
                break;
            case 'f':
                fprintf(flog, "%2.3f", va_arg(args, double));
                break;
            case 's': {
                const char *sval = va_arg(args, const char *);
                fputs(sval ? sval : "(null)", flog);
                break;
            }
            default:
                fputc(*p, flog);
                break;
        }
    }

    va_end(args);
    fputc('\n', flog);
    fclose(flog);
    writeResult = true;
}
