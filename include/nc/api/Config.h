#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <nc/api/Macro.h>

class API Config {
public:
    using json = nlohmann::json;

    enum class Format {
        Json,
        Ini,
        Yaml
    };

    explicit Config(const std::string& moduleName = "", bool type = false);
    Config& open(const std::string& filePath);
    Config& load();
    Config& save();

    template <typename T>
    T get(const std::string& key, const T& defaultValue = T{}) const {
        const json* node = findNode(key);
        if (!node || node->is_null()) return defaultValue;
        try {
            return node->get<T>();
        } catch (...) {
            return defaultValue;
        }
    }

    Config& set(const std::string& key, const json& value);
    Config& set(const std::string& key, const char* value);
    Config& set(const std::string& key, const std::string& value);
    Config& set(const std::string& key, bool value);
    Config& set(const std::string& key, int value);
    Config& set(const std::string& key, long value);
    Config& set(const std::string& key, long long value);
    Config& set(const std::string& key, unsigned int value);
    Config& set(const std::string& key, unsigned long value);
    Config& set(const std::string& key, unsigned long long value);
    Config& set(const std::string& key, float value);
    Config& set(const std::string& key, double value);

    bool exists(const std::string& key) const;
    std::string dump(int indent = 2) const;
    const std::string& path() const;
    Format format() const;
    const json& data() const;

private:
    std::string moduleName_;
    std::string filePath_;
    Format format_;
    json data_;
    bool type_;

    std::filesystem::path baseDir() const;
    std::filesystem::path resolvePath(const std::string& filePath) const;
    void ensureParentDirectory() const;

    static Format detectFormat(const std::string& filePath);
    static std::vector<std::string> splitPath(const std::string& key);
    static std::string trim(const std::string& s);
    static std::string unquote(const std::string& s);
    static std::string escapeIniValue(const json& v);
    static std::string escapeYamlValue(const json& v);
    static bool isScalar(const json& v);
    static json parseScalar(const std::string& value);
    static std::string indent(int n);

    json* findNodeMutable(const std::string& key);
    const json* findNode(const std::string& key) const;
    json& ensurePath(const std::string& key);
    void writeJsonFile() const;
    void writeIniFile() const;
    void writeYamlFile() const;
    void readJsonFile();
    void readIniFile();
    void readYamlFile();
    void writeIniSection(std::ostream& out, const json& node, const std::string& sectionPath) const;
    void writeYamlNode(std::ostream& out, const json& node, int level) const;
    void mergeIniKey(const std::string& section, const std::string& key, const json& value);
    void mergeYamlKey(const std::vector<std::string>& path, const json& value);
    static std::vector<std::string> splitSection(const std::string& section);
};