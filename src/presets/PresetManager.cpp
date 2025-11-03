#include "PresetManager.h"

#include "disstortion.h"
#include "utils/Logger.h"
#include "utils/Folders.h"

namespace stfefane::presets {

PresetManager::PresetManager(Disstortion& d) : mDisstortion(d) {
    initPresetList();
}

nlohmann::json PresetManager::getCurrentState() const {
    nlohmann::json j;
    // Store all parameters in the JSON object
    j["state_version"] = PROJECT_VERSION;
    for (const auto& param: mDisstortion.getParameters().getParams()) {
        j[param->getInfo().name] = param->getValue();
    }
    return j;
}

bool PresetManager::loadStateFromBuffer(const char* buffer) const {
    try {
        nlohmann::json j = nlohmann::json::parse(buffer);
        LOG_DEBUG("param", "[stateLoad] -> {}", j.dump(4));

        const auto state_version = j["state_version"].get<std::string>();
        if (state_version == PROJECT_VERSION) {
            for (const auto& param: mDisstortion.getParameters().getParams()) {
                param->setValue(j[param->getInfo().name].get<double>());
            }
        } else {
            // TODO: handle changes between versions.
        }
        return true;
    } catch (std::exception& e) {
        LOG_ERROR("param", "Disstortion: Failed to load state -> {}", e.what());
        return false;
    }
}

void PresetManager::initPresetList() {
    auto preset_files = utils::folders::listDirectory(utils::folders::PRESETS_DIR);
    preset_files.erase(std::ranges::remove_if(preset_files, [](const auto& filename) {
            return std::filesystem::path(filename).extension() != kExtension;
        }).begin(), preset_files.end());
    std::ranges::for_each(preset_files, [](auto& filename) {
        filename = std::filesystem::path(filename).stem().string();
    });
    std::ranges::sort(preset_files);
    mPresetList = std::move(preset_files);
    LOG_INFO("fs", "Loaded preset list -> {}", utils::rangeValues(mPresetList));
}

std::optional<size_t> PresetManager::getCurrentPresetIndex() const {
    if (const auto found = std::ranges::find(mPresetList, mCurrentPreset); found != mPresetList.end()) {
        return found - mPresetList.begin();
    }
    LOG_WARN("fs", "Could not find index of current preset -> {}", mCurrentPreset);
    return std::nullopt;
}

void PresetManager::loadPresetIndex(size_t preset_index) {
    if (preset_index >= mPresetList.size()) {
        LOG_WARN("fs", "No preset at index {}", preset_index);
        return;
    }
    const auto& preset_name = mPresetList[preset_index];
    loadPreset(preset_name);
}

void PresetManager::setCurrentPreset(std::string_view preset_name) {
    mCurrentPreset = preset_name;
    notifyListeners();
}

void PresetManager::resetPresetState() {
    for (const auto& param: mDisstortion.getParameters().getParams()) {
        param->reset();
    }
    setCurrentPreset(kInitPreset);
}

void PresetManager::savePreset(std::string_view preset_name) {
    LOG_INFO("fs", "Loading preset {}", preset_name);
    const auto state = getCurrentState();
    // TODO: sanitize preset_name + ensure file does not already exist.
    if (utils::folders::writeFileContent(utils::folders::PRESETS_DIR / std::string(preset_name).append(kExtension), state.dump())) {
        initPresetList();
        setCurrentPreset(preset_name);
    }
}

void PresetManager::loadPreset(std::string_view preset_name) {
    const auto preset_path = utils::folders::PRESETS_DIR / std::string(preset_name).append(kExtension);
    const auto json_state = utils::folders::readFileContent(preset_path);
    if (!loadStateFromBuffer(json_state.data())) {
        LOG_ERROR("fs", "Could not load preset [{}]", preset_name);
        return;
    }
    setCurrentPreset(preset_name);
}

void PresetManager::loadPreset(PresetLoad load) {
    const auto current_preset_index = getCurrentPresetIndex();
    if (!current_preset_index) {
        return;
    }
    const auto new_index = (static_cast<int>(*current_preset_index) + (load == PresetLoad::eNext ? 1 : -1)) % mPresetList.size();
    loadPresetIndex(new_index);
}

void PresetManager::addListener(Listener* listener) {
    if (std::ranges::find(mListeners, listener) == mListeners.end()) {
        mListeners.push_back(listener);
    }
}

void PresetManager::removeListener(Listener* listener) {
    std::erase(mListeners, listener);
}

void PresetManager::notifyListeners() {
    for (auto* listener: mListeners) {
        listener->currentPresetChanged(mCurrentPreset);
    }
}

} // namespace stfefane::presets