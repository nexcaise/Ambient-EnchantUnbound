#include <nc/utility/AndroidFileUtils.h>

#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#include <fstream>
#include <sstream>
#include <functional>

namespace androidfs {

std::string GetCwd() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) return std::string(buf);
    return "";
}

std::string TrimTrailingSlashes(std::string path) {
    while (path.size() > 1 && path.back() == '/') path.pop_back();
    return path;
}

std::string NormalizePath(const std::string& input) {
    if (input.empty()) return "";

    bool absolute = input[0] == '/';
    std::vector<std::string> parts;
    std::string token;

    auto push_token = [&](const std::string& t) {
        if (t.empty() || t == ".") return;
        if (t == "..") {
            if (!parts.empty() && parts.back() != "..") {
                parts.pop_back();
            } else if (!absolute) {
                parts.push_back("..");
            }
            return;
        }
        parts.push_back(t);
    };

    for (size_t i = 0; i <= input.size(); ++i) {
        char c = (i < input.size()) ? input[i] : '/';
        if (c == '/') {
            push_token(token);
            token.clear();
        } else {
            token.push_back(c);
        }
    }

    std::string out = absolute ? "/" : "";
    for (size_t i = 0; i < parts.size(); ++i) {
        out += parts[i];
        if (i + 1 < parts.size()) out += "/";
    }

    if (out.empty()) return absolute ? "/" : ".";
    return out;
}

std::string JoinPath(const std::string& a, const std::string& b) {
    if (b.empty()) return NormalizePath(a);
    if (!b.empty() && b[0] == '/') return NormalizePath(b);
    if (a.empty()) return NormalizePath(b);
    if (a.back() == '/') return NormalizePath(a + b);
    return NormalizePath(a + "/" + b);
}

std::string BaseName(std::string path) {
    if (path.empty()) return "";
    path = TrimTrailingSlashes(path);
    if (path == "/") return "/";
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

std::string DirName(std::string path) {
    if (path.empty()) return ".";
    path = TrimTrailingSlashes(path);
    if (path == "/") return "/";
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return ".";
    if (pos == 0) return "/";
    return path.substr(0, pos);
}

std::string Extension(const std::string& path) {
    std::string name = BaseName(path);
    size_t pos = name.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return "";
    return name.substr(pos + 1);
}

std::string Stem(const std::string& path) {
    std::string name = BaseName(path);
    size_t pos = name.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return name;
    return name.substr(0, pos);
}

bool Exists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool IsFile(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool IsDir(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool IsSymlink(const std::string& path) {
    struct stat st;
    return lstat(path.c_str(), &st) == 0 && S_ISLNK(st.st_mode);
}

long long FileSize(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return -1;
    return static_cast<long long>(st.st_size);
}

std::string AbsolutePath(const std::string& path) {
    if (path.empty()) return "";

    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved)) {
        return std::string(resolved);
    }

    if (!path.empty() && path[0] == '/') {
        return NormalizePath(path);
    }

    return NormalizePath(JoinPath(GetCwd(), path));
}

bool CreateDirectory(const std::string& path, mode_t mode) {
    if (path.empty()) return false;
    if (IsDir(path)) return true;
    if (mkdir(path.c_str(), mode) == 0) return true;
    return errno == EEXIST && IsDir(path);
}

bool CreateDirectories(const std::string& path, mode_t mode) {
    if (path.empty()) return false;

    std::string norm = NormalizePath(path);
    if (norm == "/" || norm == ".") return true;
    if (IsDir(norm)) return true;

    std::string current = norm[0] == '/' ? "/" : "";
    size_t i = (norm[0] == '/') ? 1 : 0;

    while (i <= norm.size()) {
        size_t j = norm.find('/', i);
        std::string part = norm.substr(i, j == std::string::npos ? std::string::npos : j - i);
        if (!part.empty()) {
            if (!current.empty() && current.back() != '/') current += "/";
            current += part;
            if (!IsDir(current)) {
                if (mkdir(current.c_str(), mode) != 0 && errno != EEXIST) {
                    return false;
                }
            }
        }
        if (j == std::string::npos) break;
        i = j + 1;
    }

    return IsDir(norm);
}

bool EnsureParentDirectory(const std::string& path, mode_t mode) {
    return CreateDirectories(DirName(path), mode);
}

bool ReadText(const std::string& path, std::string& out) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

bool WriteText(const std::string& path, const std::string& data, bool append) {
    if (!EnsureParentDirectory(path)) return false;
    std::ofstream ofs(path, std::ios::out | std::ios::binary | (append ? std::ios::app : std::ios::trunc));
    if (!ofs) return false;
    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
    return ofs.good();
}

bool ReadBinary(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    if (size < 0) return false;
    ifs.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(size));
    if (size == 0) return true;
    return static_cast<bool>(ifs.read(reinterpret_cast<char*>(out.data()), size));
}

bool WriteBinary(const std::string& path, const void* data, size_t size, bool append) {
    if (!EnsureParentDirectory(path)) return false;
    std::ofstream ofs(path, std::ios::out | std::ios::binary | (append ? std::ios::app : std::ios::trunc));
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return ofs.good();
}

bool WriteBinary(const std::string& path, const std::vector<uint8_t>& data, bool append) {
    return WriteBinary(path, data.data(), data.size(), append);
}

bool Touch(const std::string& path) {
    if (!EnsureParentDirectory(path)) return false;
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd < 0) return false;
    close(fd);
    return true;
}

bool CopyFile(const std::string& src, const std::string& dst, bool overwrite) {
    if (!IsFile(src)) return false;
    if (!EnsureParentDirectory(dst)) return false;
    if (!overwrite && Exists(dst)) return false;

    int in = open(src.c_str(), O_RDONLY);
    if (in < 0) return false;

    int flags = O_WRONLY | O_CREAT | (overwrite ? O_TRUNC : O_EXCL);
    int out = open(dst.c_str(), flags, 0644);
    if (out < 0) {
        close(in);
        return false;
    }

    char buf[8192];
    while (true) {
        ssize_t r = read(in, buf, sizeof(buf));
        if (r == 0) break;
        if (r < 0) {
            if (errno == EINTR) continue;
            close(in);
            close(out);
            return false;
        }

        ssize_t written = 0;
        while (written < r) {
            ssize_t w = write(out, buf + written, static_cast<size_t>(r - written));
            if (w < 0) {
                if (errno == EINTR) continue;
                close(in);
                close(out);
                return false;
            }
            written += w;
        }
    }

    struct stat st;
    if (fstat(in, &st) == 0) {
        fchmod(out, st.st_mode);
    }

    close(in);
    close(out);
    return true;
}

bool MoveFile(const std::string& src, const std::string& dst, bool overwrite) {
    if (!Exists(src)) return false;
    if (!EnsureParentDirectory(dst)) return false;

    if (overwrite && Exists(dst)) {
        if (IsDir(dst)) return false;
        if (unlink(dst.c_str()) != 0) return false;
    }

    if (rename(src.c_str(), dst.c_str()) == 0) return true;

    if (!CopyFile(src, dst, overwrite)) return false;
    if (IsDir(src)) return false;
    return unlink(src.c_str()) == 0;
}

bool RemoveFile(const std::string& path) {
    return unlink(path.c_str()) == 0;
}

bool RemoveDirectory(const std::string& path) {
    return rmdir(path.c_str()) == 0;
}

bool RemoveRecursive(const std::string& path) {
    if (!Exists(path)) return true;
    if (IsFile(path) || IsSymlink(path)) {
        return unlink(path.c_str()) == 0;
    }
    if (!IsDir(path)) return false;

    DIR* dir = opendir(path.c_str());
    if (!dir) return false;

    struct dirent* entry;
    bool ok = true;

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string child = JoinPath(path, name);
        if (!RemoveRecursive(child)) {
            ok = false;
            break;
        }
    }

    closedir(dir);
    if (!ok) return false;
    return rmdir(path.c_str()) == 0;
}

bool RenamePath(const std::string& oldPath, const std::string& newPath) {
    if (!EnsureParentDirectory(newPath)) return false;
    return rename(oldPath.c_str(), newPath.c_str()) == 0;
}

bool ListDirectory(const std::string& path, std::vector<std::string>& out, bool recursive, bool fullPath) {
    out.clear();
    if (!IsDir(path)) return false;

    std::function<bool(const std::string&)> walk = [&](const std::string& dirPath) -> bool {
        DIR* dir = opendir(dirPath.c_str());
        if (!dir) return false;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;

            std::string child = JoinPath(dirPath, name);
            out.push_back(fullPath ? child : name);

            if (recursive && IsDir(child)) {
                if (!walk(child)) {
                    closedir(dir);
                    return false;
                }
            }
        }

        closedir(dir);
        return true;
    };

    return walk(path);
}

std::string ReadLinkTarget(const std::string& path) {
    char buf[PATH_MAX];
    ssize_t n = readlink(path.c_str(), buf, sizeof(buf) - 1);
    if (n < 0) return "";
    buf[n] = '\0';
    return std::string(buf);
}

bool SetPermissions(const std::string& path, mode_t mode) {
    return chmod(path.c_str(), mode) == 0;
}

bool SetTimesNow(const std::string& path) {
    struct timespec ts[2];
    ts[0].tv_nsec = UTIME_NOW;
    ts[1].tv_nsec = UTIME_NOW;
    return utimensat(AT_FDCWD, path.c_str(), ts, 0) == 0;
}

bool IsReadable(const std::string& path) {
    return access(path.c_str(), R_OK) == 0;
}

bool IsWritable(const std::string& path) {
    return access(path.c_str(), W_OK) == 0;
}

bool IsExecutable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

std::string FileNameWithoutExtension(const std::string& path) {
    return Stem(path);
}

std::string ParentPath(const std::string& path) {
    return DirName(path);
}

std::string RealPathIfExists(const std::string& path) {
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved)) return std::string(resolved);
    return "";
}

bool CopyDirectory(const std::string& src, const std::string& dst, bool overwrite) {
    if (!androidfs::IsDir(src)) return false;

    if (!androidfs::CreateDirectories(dst)) return false;

    std::vector<std::string> entries;
    if (!androidfs::ListDirectory(src, entries, false, false)) return false;
	if (androidfs::AbsolutePath(dst).find(androidfs::AbsolutePath(src)) == 0) {
	    return false;
	}
    for (const auto& name : entries) {
        std::string srcPath = androidfs::JoinPath(src, name);
        std::string dstPath = androidfs::JoinPath(dst, name);

        if (androidfs::IsDir(srcPath)) {
            if (!CopyDirectory(srcPath, dstPath, overwrite)) return false;
        } else if (androidfs::IsFile(srcPath)) {
            if (!androidfs::CopyFile(srcPath, dstPath, overwrite)) return false;
        } else if (androidfs::IsSymlink(srcPath)) {
            // optional: copy symlink
            std::string target = androidfs::ReadLinkTarget(srcPath);
            symlink(target.c_str(), dstPath.c_str());
        }
    }

    return true;
}

}