#pragma once

#include <nlohmann/json.hpp>

namespace stfefane {
class Disstortion;
}

namespace stfefane::presets {

class PresetManager {
public:
    explicit PresetManager(Disstortion& d) : mDisstortion(d) {}

    nlohmann::json getCurrentState() const;
    bool loadStateFromBuffer(const char* buffer) const;

private:
    Disstortion& mDisstortion;

};

}