#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <nc/api/Macro.h>

struct ModuleInfo {
    uintptr_t base;
    std::string path;
};

API ModuleInfo findModule(const std::string& name);

API ModuleInfo waitForLibrary(const std::string& name);

CAPI bool isLibraryReady(const std::string& name);

CAPI int getAppId(char *outId, size_t outSize);