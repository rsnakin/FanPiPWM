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
