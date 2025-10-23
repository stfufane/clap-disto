#pragma once

#if WIN32
#define NOMINMAX
#include <shlobj.h>
#endif

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace stfefane::utils::folders {

#if WIN32
inline const auto SETTINGS_DIR = []() -> std::filesystem::path {
    PWSTR path_tmp;

    /* Attempt to get user's AppData folder
     *
     * Microsoft Docs:
     * https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
     * https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
     */
    auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_tmp);

    /* Error check */
    if (get_folder_path_ret != S_OK) {
        CoTaskMemFree(path_tmp);
        std::cout << "Could not find app data folder\n";
        return {};
    }

    /* Convert the Windows path type to a C++ path */
    std::filesystem::path settings_path = std::filesystem::path(path_tmp) / USERDATA_DIR;

    CoTaskMemFree(path_tmp);

    return settings_path;
}();
#else
inline const std::filesystem::path SETTINGS_DIR = std::filesystem::path(USERDATA_DIR);
#endif

inline const auto LOG_SETTINGS_FILE = SETTINGS_DIR / "log.settings";

inline void setupDataFolder() {
    if (!std::filesystem::exists(SETTINGS_DIR)) {
        try {
            std::filesystem::create_directories(SETTINGS_DIR);
        } catch (const std::exception& e) {
            std::cout << "Impossible to create the settings folder\n";
            throw;
        }
    }
}

inline bool writeFileContent(const std::filesystem::path& path, std::string_view content) {
    try {
        std::ofstream file(path.generic_string());
        file << content;
        file.close();
    } catch (const std::exception& e) {
        std::cout << "Impossible to write to file " << path.generic_string() << " -> " << e.what() << "\n";
        return false;
    }
    return true;
}

inline std::string readFileContent(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return {};
    }

    try {
        std::ifstream file(path.string(), std::ifstream::binary);
        if (file.is_open()) {
            std::string content;
            file >> content;
            return content;
        }
    } catch (const std::exception& e) {
        std::cout << "Impossible to read file " << path.generic_string() << " -> " << e.what() << "\n";
        return {};
    }

    return {};
}

}
