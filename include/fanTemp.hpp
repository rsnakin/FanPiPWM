#pragma once

class fanTemp {
    private:
        int tempFiled;
    public:
        bool isError;
        char errorMsg[64];
        fanTemp(const char *tempPath);
        ~fanTemp();
        unsigned int get();
};

