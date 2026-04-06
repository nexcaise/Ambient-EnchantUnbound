#pragma once
#include <android/log.h>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace nc {

class Logger {
public:
    using Ptr = std::shared_ptr<Logger>;

    static Ptr getOrCreate(const std::string& tag);

    explicit Logger(std::string tag);

    template <typename... Args>
    void v(const std::string& fmt, Args&&... args) { log(ANDROID_LOG_VERBOSE, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void d(const std::string& fmt, Args&&... args) { log(ANDROID_LOG_DEBUG, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void i(const std::string& fmt, Args&&... args) { log(ANDROID_LOG_INFO, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void w(const std::string& fmt, Args&&... args) { log(ANDROID_LOG_WARN, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void e(const std::string& fmt, Args&&... args) { log(ANDROID_LOG_ERROR, fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void verbose(const std::string& fmt, Args&&... args) { v(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void Verbose(const std::string& fmt, Args&&... args) { v(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void VERBOSE(const std::string& fmt, Args&&... args) { v(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void debug(const std::string& fmt, Args&&... args) { d(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void Debug(const std::string& fmt, Args&&... args) { d(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void DEBUG(const std::string& fmt, Args&&... args) { d(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void info(const std::string& fmt, Args&&... args) { i(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void Info(const std::string& fmt, Args&&... args) { i(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void INFO(const std::string& fmt, Args&&... args) { i(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void warn(const std::string& fmt, Args&&... args) { w(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void Warn(const std::string& fmt, Args&&... args) { w(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void WARN(const std::string& fmt, Args&&... args) { w(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void error(const std::string& fmt, Args&&... args) { e(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void Error(const std::string& fmt, Args&&... args) { e(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    void ERROR(const std::string& fmt, Args&&... args) { e(fmt, std::forward<Args>(args)...); }

    template <typename... Args>
    static void V(const std::string& tag, const std::string& fmt, Args&&... args) {
        getOrCreate(tag)->v(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void D(const std::string& tag, const std::string& fmt, Args&&... args) {
        getOrCreate(tag)->d(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void I(const std::string& tag, const std::string& fmt, Args&&... args) {
        getOrCreate(tag)->i(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void W(const std::string& tag, const std::string& fmt, Args&&... args) {
        getOrCreate(tag)->w(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void E(const std::string& tag, const std::string& fmt, Args&&... args) {
        getOrCreate(tag)->e(fmt, std::forward<Args>(args)...);
    }

private:
    std::string mTag;

    static std::unordered_map<std::string, Ptr>& registry();
    static std::mutex& registryMutex();

    template <typename T>
    static void appendOne(std::ostringstream& oss, T&& value) {
        oss << std::forward<T>(value);
    }

    static void formatImpl(std::ostringstream& oss, const std::string& fmt, size_t& pos) {
        while (pos < fmt.size()) {
            oss << fmt[pos++];
        }
    }

    template <typename T, typename... Args>
    static void formatImpl(std::ostringstream& oss, const std::string& fmt, size_t& pos, T&& value, Args&&... args) {
        while (pos < fmt.size()) {
            if (fmt[pos] == '{' && pos + 1 < fmt.size() && fmt[pos + 1] == '}') {
                appendOne(oss, std::forward<T>(value));
                pos += 2;
                formatImpl(oss, fmt, pos, std::forward<Args>(args)...);
                return;
            }
            oss << fmt[pos++];
        }
    }

    template <typename... Args>
    static std::string format(const std::string& fmt, Args&&... args) {
        std::ostringstream oss;
        size_t pos = 0;
        formatImpl(oss, fmt, pos, std::forward<Args>(args)...);
        return oss.str();
    }

    template <typename... Args>
    void log(int prio, const std::string& fmt, Args&&... args) {
        std::string msg = format(fmt, std::forward<Args>(args)...);
        __android_log_print(prio, mTag.c_str(), "%s", msg.c_str());
    }
};

}