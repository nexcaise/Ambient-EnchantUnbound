#include <nc/utility/module.h>

ModuleInfo findModule(const std::string& name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line)) {
        if (line.find(name) != std::string::npos) {
            std::istringstream iss(line);

            std::string address, perms, offset, dev, inode, path;
            iss >> address >> perms >> offset >> dev >> inode;
            std::getline(iss, path);

            size_t dash = address.find('-');
            uintptr_t base = std::stoull(address.substr(0, dash), nullptr, 16);

            return { base, path };
        }
    }

    return { 0, "" };
}

bool isLibraryReady(const std::string& name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line)) {
        if (line.find(name) != std::string::npos &&
            line.find("r-xp") != std::string::npos) {
            return true;
        }
    }
    return false;
}

ModuleInfo waitForLibrary(const std::string& name) {
    ModuleInfo mod{};

    while (true) {
        mod = findModule(name);

        if (mod.base != 0 && isLibraryReady(name)) {
            break;
        }

        usleep(100 * 1000); 
    }

    usleep(500 * 1000); 
    return mod;
}

int getAppId(char *outId, size_t outSize) {
    if (!outId || outSize == 0) return 1;
    FILE *f = fopen("/proc/self/cmdline", "r");
    if (!f) return 1;
    if (!fgets(outId, outSize, f)) {
        fclose(f);
        return 1;
    }
    fclose(f);
    outId[outSize - 1] = '\0';

    return 0;
}