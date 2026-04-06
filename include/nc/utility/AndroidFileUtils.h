#pragma once

#include <sys/types.h>
#include <cstdint>
#include <string>
#include <vector>

namespace androidfs {

std::string GetCwd();
std::string TrimTrailingSlashes(std::string path);
std::string NormalizePath(const std::string& input);
std::string JoinPath(const std::string& a, const std::string& b);
std::string BaseName(std::string path);
std::string DirName(std::string path);
std::string Extension(const std::string& path);
std::string Stem(const std::string& path);

bool Exists(const std::string& path);
bool IsFile(const std::string& path);
bool IsDir(const std::string& path);
bool IsSymlink(const std::string& path);
long long FileSize(const std::string& path);
std::string AbsolutePath(const std::string& path);

bool CreateDirectory(const std::string& path, mode_t mode = 0755);
bool CreateDirectories(const std::string& path, mode_t mode = 0755);
bool EnsureParentDirectory(const std::string& path, mode_t mode = 0755);

bool ReadText(const std::string& path, std::string& out);
bool WriteText(const std::string& path, const std::string& data, bool append = false);

bool ReadBinary(const std::string& path, std::vector<uint8_t>& out);
bool WriteBinary(const std::string& path, const void* data, size_t size, bool append = false);
bool WriteBinary(const std::string& path, const std::vector<uint8_t>& data, bool append = false);

bool Touch(const std::string& path);
bool CopyFile(const std::string& src, const std::string& dst, bool overwrite = true);
bool MoveFile(const std::string& src, const std::string& dst, bool overwrite = true);

bool RemoveFile(const std::string& path);
bool RemoveDirectory(const std::string& path);
bool RemoveRecursive(const std::string& path);
bool RenamePath(const std::string& oldPath, const std::string& newPath);

bool ListDirectory(const std::string& path, std::vector<std::string>& out, bool recursive = false, bool fullPath = true);

std::string ReadLinkTarget(const std::string& path);
bool SetPermissions(const std::string& path, mode_t mode);
bool SetTimesNow(const std::string& path);

bool IsReadable(const std::string& path);
bool IsWritable(const std::string& path);
bool IsExecutable(const std::string& path);

std::string FileNameWithoutExtension(const std::string& path);
std::string ParentPath(const std::string& path);
std::string RealPathIfExists(const std::string& path);
bool CopyDirectory(const std::string& src, const std::string& dst, bool overwrite);
}