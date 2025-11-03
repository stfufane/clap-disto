#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace stfefane::utils::folders {

extern const std::filesystem::path PLUGIN_DIR;
extern const std::filesystem::path LOG_SETTINGS_FILE;
extern const std::filesystem::path PRESETS_DIR;

#if __APPLE__
std::string expandTilde(const char* str);
#endif

void setupPluginFolder();
bool createDirectory(const std::filesystem::path& dir);

std::vector<std::string> listDirectory(const std::filesystem::path& dir);

bool writeFileContent(const std::filesystem::path& path, std::string_view content);
std::string readFileContent(const std::filesystem::path& path);

} // namespace stfefane::utils::folders
