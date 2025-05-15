#include <fstream>
#include <string.h>
#include <stdarg.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "cfgini.hpp"

cfg::cfg(const char *cfgFile) {
    error = false;
    __containerEmpty = true;
    size_t len = strlen((char *)cfgFile) + 1;
    __cfgFile = (char *)malloc(len);
    if(!__cfgFile) {
        error = true;
        sprintf(errorMsg, "Error! memory not allocated (__cfgFile).");
    } else {
        sprintf(__cfgFile, "%s", cfgFile);
        __cfg = fopen(__cfgFile, "r");
        if(!__cfg) {
            error = true;
            sprintf(errorMsg, "Error! Config %s not found", __cfgFile);
        } else {
            if(!readCFG()) error = true;
            fclose(__cfg);
        }
    }
}

bool cfg::getValue(const char *pType, const char *section, const char *parameter, ...) {
    int i = 0;
    size_t len = strlen((char *)section) + strlen((char *)parameter) + 4;
    char *key = (char *)malloc(len);
    if(!key) {
        sprintf(errorMsg, "Error! memory not allocated (key).");
        return(false);
    }
    if(strlen((char *)section)) {
        sprintf(key, "[%s]:%s", section, parameter);
    } else {
        sprintf(key, "%s", parameter);
    }

    va_list aPtr;
    va_start(aPtr, parameter);

    while(__iniContainer[i]) {
        if(strcmp(key, __iniContainer[i]) == 0) {
            if(strcmp(pType, "%s") == 0) {
                char *value = va_arg(aPtr, char *);
                sprintf(value, "%s", __iniContainer[i + 1]);
            } else if(strcmp(pType, "%d") == 0) {
                int *value = va_arg(aPtr, int *);
                *value = atoi(__iniContainer[i + 1]);
            } else if(strcmp(pType, "%lu") == 0) {
                unsigned long int *value = va_arg(aPtr, unsigned long int *);
                *value = strtoul(__iniContainer[i + 1], nullptr, 10);
            } else if(strcmp(pType, "%u") == 0) {
                unsigned int *value = va_arg(aPtr, unsigned int *);
                *value = (unsigned int)strtoul(__iniContainer[i + 1], nullptr, 10);
            } else if(strcmp(pType, "%b") == 0) {
                bool *value = va_arg(aPtr, bool *);
                if(strcmp(__iniContainer[i + 1], "true")) {
                    *value = false;
                } else {
                    *value = true;
                }
            } else {
                sprintf(errorMsg, "Undefined value type [%s]%s (%s).", section, parameter, pType);
                va_end(aPtr);
                free(key);
                return(false);
            }
            va_end(aPtr);
            free(key);
            return(true);
        }
        i ++;
    }
    sprintf(errorMsg, "[%s] %s not found!", section, parameter);
    va_end(aPtr);
    free(key);
    return(false);
}

bool cfg::getValStr(const char *section, const char *parameter, char *valueStr) {
    int i = 0;
    size_t len = strlen((char *)section) + strlen((char *)parameter) + 4;
    char *key = (char *)malloc(len);
    if(!key) {
        sprintf(errorMsg, "Error! memory not allocated (key).");
        return(false);
    }
    if(strlen((char *)section)) {
        sprintf(key, "[%s]:%s", section, parameter);
    } else {
        sprintf(key, "%s", parameter);
    }

    while(__iniContainer[i]) {
        if(strcmp(key, __iniContainer[i]) == 0) {
            sprintf(valueStr, "%s", __iniContainer[i + 1]);
            free(key);
            return(true);
        }
        i ++;
    }
    sprintf(errorMsg, "[%s] %s not found!", section, parameter);
    free(key);
    return(false);
}

cfg::~cfg() {
    free(__cfgFile);
    if(!__containerEmpty) {
        int i = 0;
        while(__iniContainer[i]) {
            free(__iniContainer[i]);
            i ++;
        }
        free(__iniContainer);
    }
}

bool cfg::readCFG() {
    const int recordSize = 128;
    char buffer[1024];
    char section[32] = {0};
    int itemNum = 0;
    int is = 0;
    bool is_section = false;

    __iniContainer = (char **)malloc(sizeof(char *) * 2);
    if (!__iniContainer) {
        sprintf(errorMsg, "Error! memory not allocated (__iniContainer).\n");
        return false;
    }

    for (int i = 0; i < 2; i++) {
        __iniContainer[i] = (char *)malloc(recordSize);
        if (!__iniContainer[i]) {
            sprintf(errorMsg, "Error! memory not allocated (__iniContainer[%d]).", i);
            return false;
        }
    }

    __containerEmpty = false;

    while (fgets(buffer, sizeof(buffer), __cfg)) {
        int i = 0;
        int ip = 0, iv = 0;
        char param[64] = {0}, value[64] = {0};
        bool is_param = true, is_value = false;

        while (buffer[i]) {
            if (buffer[i] == ';' || buffer[i] == '\r' || buffer[i] == '\n') break;
            if (buffer[i] != ' ' && buffer[i] != '\t') {
                if (buffer[i] == '[') {
                    is_section = true;
                    i++;
                    continue;
                }
                if (is_section && buffer[i] != ']') {
                    if (is >= sizeof(section) - 1) {
                        sprintf(errorMsg, "Section name too long");
                        return false;
                    }
                    section[is++] = buffer[i++];
                    section[is] = '\0';
                    continue;
                } else if (buffer[i] == ']') {
                    is_section = false;
                    is = 0;
                    i++;
                    continue;
                }
                if (is_param && buffer[i] != '=') {
                    if (ip >= sizeof(param) - 1) {
                        sprintf(errorMsg, "Parameter name too long");
                        return false;
                    }
                    param[ip++] = buffer[i++];
                    param[ip] = '\0';
                    continue;
                } else if (buffer[i] == '=') {
                    is_param = false;
                    is_value = true;
                    i++;
                    continue;
                }
                if (is_value) {
                    if (iv >= sizeof(value) - 1) {
                        sprintf(errorMsg, "Value too long");
                        return false;
                    }
                    value[iv++] = buffer[i];
                    value[iv] = '\0';
                }
            }
            i++;
        }

        if (strlen(section) && strlen(param)) {
            snprintf(__iniContainer[itemNum], recordSize, "[%s]:%s", section, param);
        } else if (strlen(param)) {
            snprintf(__iniContainer[itemNum], recordSize, "%s", param);
        }

        if (strlen(param) && strlen(value)) {
            itemNum++;
            snprintf(__iniContainer[itemNum], recordSize, "%s", value);
            itemNum++;

            char **newContainer = (char **)realloc(__iniContainer, sizeof(char *) * (itemNum + 2));
            if (!newContainer) {
                sprintf(errorMsg, "Error reallocating memory (__iniContainer).\n");
                return false;
            }
            __iniContainer = newContainer;

            for (int k = 0; k < 2; ++k) {
                __iniContainer[itemNum + k] = (char *)malloc(recordSize);
                if (!__iniContainer[itemNum + k]) {
                    sprintf(errorMsg, "Error! memory not allocated (__iniContainer[%d]).", itemNum + k);
                    return false;
                }
            }
        }
    }
    return true;
}


