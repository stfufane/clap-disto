#include "Folders.h"

#if WIN32
#define NOMINMAX
#include <shlobj.h>
#elif __APPLE__
#include <glob.h>
#include <sysdir.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <fstream>
#include "Logger.h"

namespace stfefane::utils::folders {

const std::filesystem::path PLUGIN_DIR = []() -> std::filesystem::path {
#if WIN32
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
        LOG_ERROR("fs", "Could not find app data folder");
        return {};
    }

    /* Convert the Windows path type to a C++ path */
    std::filesystem::path settings_path = std::filesystem::path(path_tmp) / USERDATA_DIR;

    CoTaskMemFree(path_tmp);

    return settings_path;
#elif __APPLE__
    // On macOS, use the sysdir API to retrieve the user Application Support folder.
    char path[PATH_MAX];
    auto state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_APPLICATION_SUPPORT,
                                                      SYSDIR_DOMAIN_MASK_USER);
    if ((state = sysdir_get_next_search_path_enumeration(state, path))) {
        return std::filesystem::path(expandTilde(path)) / USERDATA_DIR;
    }
    return {};
#else
    const char* home_dir = getpwuid(getuid())->pw_dir;
    return std::filesystem::path(home_dir) / ".config" / USERDATA_DIR;
#endif
}();

const std::filesystem::path LOG_SETTINGS_FILE = PLUGIN_DIR / "log.settings";
const std::filesystem::path PRESETS_DIR = PLUGIN_DIR / "Presets";

#if __APPLE__
std::string folders::expandTilde(const char* str) {
    if (!str) {
        return {};
    }

    glob_t globbuf;
    if (glob(str, GLOB_TILDE, nullptr, &globbuf) == 0) {
        std::string result(globbuf.gl_pathv[0]);
        globfree(&globbuf);
        return result;
    }
    return {};
}
#endif

void setupPluginFolder() {
    LOG_INFO("fs", "Create plugin folder at {}", std::filesystem::absolute(PLUGIN_DIR).generic_string());
    if (!createDirectory(PLUGIN_DIR)) {
        throw;
    }

#if DEBUG
    // Overwrite the log levels with the ones from the settings file.
    loadLogLevelsFromFile();
#endif
}

bool createDirectory(const std::filesystem::path& dir) {
    if (std::error_code ec; !std::filesystem::exists(dir, ec)) {
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            LOG_ERROR("fs", "Could not create directory {} -> {}", dir.generic_string(), ec.message());
            return false;
        }
    }
    return true;
}

bool writeFileContent(const std::filesystem::path& path, std::string_view content) {
    try {
        std::ofstream file(path.generic_string());
        file << content;
        file.close();
    } catch (const std::exception& e) {
        LOG_ERROR("fs", "Impossible to write to file {} -> {}", path.generic_string(), e.what());
        return false;
    }
    return true;
}

std::string readFileContent(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return {};
    }

    try {
        if (std::ifstream file(path.generic_string(), std::ifstream::binary); file.is_open()) {
            std::string content;
            file >> content;
            return content;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("fs", "Impossible to read file {} -> {}", path.generic_string(), e.what());
        return {};
    }

    return {};
}
} // namespace stfefane::utils
