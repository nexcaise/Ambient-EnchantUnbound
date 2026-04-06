#include <nc/api/Logger.h>

namespace nc {

Logger::Logger(std::string tag) : mTag(std::move(tag)) {}

std::unordered_map<std::string, Logger::Ptr>& Logger::registry() {
    static std::unordered_map<std::string, Ptr> map;
    return map;
}

std::mutex& Logger::registryMutex() {
    static std::mutex m;
    return m;
}

Logger::Ptr Logger::getOrCreate(const std::string& tag) {
    std::lock_guard<std::mutex> lock(registryMutex());
    auto& map = registry();
    auto it = map.find(tag);
    if (it != map.end()) return it->second;
    auto logger = std::make_shared<Logger>(tag);
    map.emplace(tag, logger);
    return logger;
}

}