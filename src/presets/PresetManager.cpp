#include "PresetManager.h"

#include "disstortion.h"
#include "utils/Logger.h"

namespace stfefane::presets {

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

} // namespace stfefane::presets