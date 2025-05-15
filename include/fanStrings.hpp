#pragma once

class fanString {

    private:

        char *__fanString;
        size_t fanStringLen;

    public:

        fanString();
        char* repCharsStr(size_t amt, const char *sim);
        char* cnvTimeLine(unsigned long unixTime);
        ~fanString();

};

