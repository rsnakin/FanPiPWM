#pragma once

class cfg {
    public: 
        bool error;
        char errorMsg[128];
        cfg(const char *cfgFile);
        bool getValue(const char *pType, const char *section, const char *parameter, ...);
        ~cfg();
    private:
        char *__cfgFile;
        FILE *__cfg;
        char **__iniContainer;
        bool __containerEmpty;
        bool readCFG();
        bool getValStr(const char *section, const char *parameter, char *valueStr);
};
