#include <nc/api/Config.h>
#include <nc/utility/module.h>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <system_error>

using json = nlohmann::json;

Config::Config(const std::string& moduleName, bool type)
    : moduleName_(moduleName.empty() ? "default" : moduleName), type_(type), format_(Format::Json), data_(json::object()) {
}

std::filesystem::path Config::baseDir() const {
	char appId[512];
	std::string appPackage = "";

    if (getAppId(appId, sizeof(appId)) == 0) {
        appPackage = std::string(appId);
    } else {
    	appPackage = "io.kitsuri.mayape";
    }
    
    std::string configPath = type_ ? "/sdcard/games" : "/sdcard/Android/media/" + appPackage + "/modules";
    
    return std::filesystem::path(configPath);
}

std::filesystem::path Config::resolvePath(const std::string& filePath) const {
    std::filesystem::path p(filePath);
    if (p.is_absolute()) return p;
    return baseDir() / moduleName_ / p;
}

void Config::ensureParentDirectory() const {
    if (filePath_.empty()) return;
    std::filesystem::path p(filePath_);
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
}

Config& Config::open(const std::string& filePath) {
    filePath_ = resolvePath(filePath).string();
    format_ = detectFormat(filePath_);
    data_ = json::object();
    load();
    return *this;
}

Config& Config::load() {
    if (filePath_.empty()) return *this;

    std::ifstream in(filePath_, std::ios::binary);
    if (!in.good()) {
        data_ = json::object();
        save();
        return *this;
    }

    switch (format_) {
        case Format::Json:
            readJsonFile();
            break;
        case Format::Ini:
            readIniFile();
            break;
        case Format::Yaml:
            readYamlFile();
            break;
    }

    return *this;
}

Config& Config::save() {
    if (filePath_.empty()) return *this;

    ensureParentDirectory();

    switch (format_) {
        case Format::Json:
            writeJsonFile();
            break;
        case Format::Ini:
            writeIniFile();
            break;
        case Format::Yaml:
            writeYamlFile();
            break;
    }

    return *this;
}

Config& Config::set(const std::string& key, const json& value) {
    ensurePath(key) = value;
    save();
    return *this;
}

Config& Config::set(const std::string& key, const char* value) {
    return set(key, json(value ? value : ""));
}

Config& Config::set(const std::string& key, const std::string& value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, bool value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, int value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, long value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, long long value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, unsigned int value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, unsigned long value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, unsigned long long value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, float value) {
    return set(key, json(value));
}

Config& Config::set(const std::string& key, double value) {
    return set(key, json(value));
}

bool Config::exists(const std::string& key) const {
    return findNode(key) != nullptr;
}

std::string Config::dump(int indent) const {
    return data_.dump(indent);
}

const std::string& Config::path() const {
    return filePath_;
}

Config::Format Config::format() const {
    return format_;
}

const Config::json& Config::data() const {
    return data_;
}

Config::Format Config::detectFormat(const std::string& filePath) {
    std::filesystem::path p(filePath);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (ext == ".json") return Format::Json;
    if (ext == ".ini") return Format::Ini;
    if (ext == ".yaml" || ext == ".yml") return Format::Yaml;
    return Format::Json;
}

std::vector<std::string> Config::splitPath(const std::string& key) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : key) {
        if (c == '.') {
            if (!current.empty()) parts.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

std::vector<std::string> Config::splitSection(const std::string& section) {
    if (section.empty()) return {};
    return splitPath(section);
}

std::string Config::trim(const std::string& s) {
    size_t start = 0;
    size_t end = s.size();

    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;

    return s.substr(start, end - start);
}

std::string Config::unquote(const std::string& s) {
    if (s.size() >= 2) {
        char a = s.front();
        char b = s.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

bool Config::isScalar(const json& v) {
    return v.is_boolean() || v.is_number() || v.is_string() || v.is_null();
}

std::string Config::escapeIniValue(const json& v) {
    if (v.is_null()) return "";
    if (v.is_boolean()) return v.get<bool>() ? "true" : "false";
    if (v.is_number_integer()) return std::to_string(v.get<long long>());
    if (v.is_number_unsigned()) return std::to_string(v.get<unsigned long long>());
    if (v.is_number_float()) {
        std::ostringstream oss;
        oss << v.get<double>();
        return oss.str();
    }
    std::string s = v.get<std::string>();
    bool needsQuotes = s.empty() || s.find_first_of(" \t#;=") != std::string::npos;
    if (!needsQuotes) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

std::string Config::escapeYamlValue(const json& v) {
    if (v.is_null()) return "null";
    if (v.is_boolean()) return v.get<bool>() ? "true" : "false";
    if (v.is_number_integer()) return std::to_string(v.get<long long>());
    if (v.is_number_unsigned()) return std::to_string(v.get<unsigned long long>());
    if (v.is_number_float()) {
        std::ostringstream oss;
        oss << v.get<double>();
        return oss.str();
    }
    std::string s = v.get<std::string>();
    bool needsQuotes = s.empty() || s.find_first_of(":#\n\r\t\"'{}[]&,*?|-<>=!%@\\") != std::string::npos;
    if (!needsQuotes) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

Config::json Config::parseScalar(const std::string& value) {
    std::string s = trim(value);
    if (s.empty()) return json("");
    s = unquote(s);

    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (lower == "null" || lower == "~") return nullptr;
    if (lower == "true") return true;
    if (lower == "false") return false;

    bool integerLike = !s.empty() && (std::isdigit(static_cast<unsigned char>(s[0])) || s[0] == '-' || s[0] == '+');
    bool floatLike = s.find('.') != std::string::npos || s.find('e') != std::string::npos || s.find('E') != std::string::npos;

    try {
        if (integerLike && !floatLike) {
            long long v = std::stoll(s);
            return json(v);
        }
        if (integerLike || floatLike) {
            double v = std::stod(s);
            return json(v);
        }
    } catch (...) {
    }

    return json(s);
}

std::string Config::indent(int n) {
    return std::string(static_cast<size_t>(n), ' ');
}

Config::json* Config::findNodeMutable(const std::string& key) {
    auto parts = splitPath(key);
    if (parts.empty()) return nullptr;

    json* node = &data_;
    for (const auto& part : parts) {
        if (!node->is_object() || !node->contains(part)) return nullptr;
        node = &(*node)[part];
    }
    return node;
}

const Config::json* Config::findNode(const std::string& key) const {
    auto parts = splitPath(key);
    if (parts.empty()) return nullptr;

    const json* node = &data_;
    for (const auto& part : parts) {
        if (!node->is_object() || !node->contains(part)) return nullptr;
        node = &(*node)[part];
    }
    return node;
}

Config::json& Config::ensurePath(const std::string& key) {
    auto parts = splitPath(key);
    if (parts.empty()) return data_;

    json* node = &data_;
    if (!node->is_object()) *node = json::object();

    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        if (i + 1 == parts.size()) {
            return (*node)[part];
        }
        if (!(*node).contains(part) || !(*node)[part].is_object()) {
            (*node)[part] = json::object();
        }
        node = &(*node)[part];
    }
    return data_;
}

void Config::writeJsonFile() const {
    std::ofstream out(filePath_, std::ios::binary | std::ios::trunc);
    out << data_.dump(2);
}

void Config::writeIniSection(std::ostream& out, const json& node, const std::string& sectionPath) const {
    if (!node.is_object()) return;

    std::vector<std::pair<std::string, json>> nested;

    if (!sectionPath.empty()) {
        out << "[" << sectionPath << "]\n";
    }

    for (auto it = node.begin(); it != node.end(); ++it) {
        const std::string key = it.key();
        const json& value = it.value();

        if (value.is_object()) {
            nested.emplace_back(key, value);
        } else {
            out << key << "=" << escapeIniValue(value) << "\n";
        }
    }

    if (!sectionPath.empty()) {
        out << "\n";
    }

    for (const auto& item : nested) {
        std::string nextPath = sectionPath.empty() ? item.first : sectionPath + "." + item.first;
        writeIniSection(out, item.second, nextPath);
    }
}

void Config::writeIniFile() const {
    std::ofstream out(filePath_, std::ios::binary | std::ios::trunc);
    if (!data_.is_object()) return;
    writeIniSection(out, data_, "");
}

void Config::writeYamlNode(std::ostream& out, const json& node, int level) const {
    if (!node.is_object()) return;

    for (auto it = node.begin(); it != node.end(); ++it) {
        const std::string key = it.key();
        const json& value = it.value();

        out << indent(level) << key << ":";
        if (value.is_object()) {
            if (value.empty()) {
                out << " {}\n";
            } else {
                out << "\n";
                writeYamlNode(out, value, level + 2);
            }
        } else {
            out << " " << escapeYamlValue(value) << "\n";
        }
    }
}

void Config::writeYamlFile() const {
    std::ofstream out(filePath_, std::ios::binary | std::ios::trunc);
    if (!data_.is_object()) return;
    writeYamlNode(out, data_, 0);
}

void Config::readJsonFile() {
    std::ifstream in(filePath_, std::ios::binary);
    if (!in.good()) {
        data_ = json::object();
        return;
    }

    try {
        in >> data_;
        if (!data_.is_object() && !data_.is_array()) {
            data_ = json::object();
        }
    } catch (...) {
        data_ = json::object();
    }
}

void Config::mergeIniKey(const std::string& section, const std::string& key, const json& value) {
    json* node = &data_;
    auto parts = splitSection(section);
    for (const auto& part : parts) {
        if (!node->contains(part) || !(*node)[part].is_object()) {
            (*node)[part] = json::object();
        }
        node = &(*node)[part];
    }
    (*node)[key] = value;
}

void Config::readIniFile() {
    data_ = json::object();
    std::ifstream in(filePath_, std::ios::binary);
    if (!in.good()) return;

    std::string line;
    std::string currentSection;

    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            currentSection = trim(line.substr(1, line.size() - 2));
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) pos = line.find(':');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (!value.empty()) {
            size_t cpos = value.find_first_of(";#");
            if (cpos != std::string::npos) {
                value = trim(value.substr(0, cpos));
            }
        }

        mergeIniKey(currentSection, key, parseScalar(value));
    }
}

void Config::mergeYamlKey(const std::vector<std::string>& path, const json& value) {
    if (path.empty()) return;

    json* node = &data_;
    for (size_t i = 0; i < path.size(); ++i) {
        const std::string& part = path[i];
        if (i + 1 == path.size()) {
            (*node)[part] = value;
            return;
        }
        if (!node->contains(part) || !(*node)[part].is_object()) {
            (*node)[part] = json::object();
        }
        node = &(*node)[part];
    }
}

void Config::readYamlFile() {
    data_ = json::object();
    std::ifstream in(filePath_, std::ios::binary);
    if (!in.good()) return;

    std::string line;
    std::vector<std::pair<int, std::vector<std::string>>> stack;
    stack.push_back({-1, {}});

    while (std::getline(in, line)) {
        std::string raw = line;
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();
        std::string t = trim(raw);
        if (t.empty()) continue;
        if (t[0] == '#' || t[0] == ';') continue;

        int spaces = 0;
        while (spaces < static_cast<int>(raw.size()) && raw[spaces] == ' ') spaces++;

        while (stack.size() > 1 && spaces <= stack.back().first) {
            stack.pop_back();
        }

        auto pos = t.find(':');
        if (pos == std::string::npos) continue;

        std::string key = trim(t.substr(0, pos));
        std::string value = trim(t.substr(pos + 1));

        std::vector<std::string> currentPath = stack.back().second;
        currentPath.push_back(key);

        if (value.empty()) {
            stack.push_back({spaces, currentPath});
            mergeYamlKey(currentPath, json::object());
        } else {
            mergeYamlKey(currentPath, parseScalar(value));
        }
    }
}