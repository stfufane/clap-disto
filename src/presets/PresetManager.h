#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>

namespace stfefane {
class Disstortion;
}

namespace stfefane::presets {

enum class PresetLoad {
    ePrev,
    eNext
};

using std::literals::operator ""sv;

class PresetManager {
public:
    explicit PresetManager(Disstortion& d);

    class Listener {
    public:
        Listener() = delete;
        explicit Listener(PresetManager& manager) : mPresetManager(manager) {
            mPresetManager.addListener(this);
        }
        virtual ~Listener() {
            mPresetManager.removeListener(this);
        }
        virtual void currentPresetChanged(const std::string& new_preset) = 0;
    protected:
        PresetManager& mPresetManager;
    };

    [[nodiscard]] nlohmann::json getCurrentState() const;
    bool loadStateFromBuffer(const char* buffer) const;

    [[nodiscard]] const std::vector<std::string>& getPresetList() const { return mPresetList; }
    [[nodiscard]] const std::string& getCurrentPreset() const { return mCurrentPreset; };

    void resetPresetState();

    void savePreset(std::string_view preset_name);
    void loadPreset(std::string_view preset_name);

    void loadPreset(PresetLoad load);

    void addListener(Listener* listener);
    void removeListener(Listener* listener);
    void notifyListeners();

private:
    static constexpr auto kInitPreset = "init"sv;
    static constexpr auto kExtension = ".diss"sv;

    void initPresetList();

    [[nodiscard]] std::optional<size_t> getCurrentPresetIndex() const;
    void loadPresetIndex(size_t preset_index);

    void setCurrentPreset(std::string_view preset_name);

    Disstortion& mDisstortion;

    std::vector<Listener*> mListeners;

    std::vector<std::string> mPresetList;
    std::string mCurrentPreset { kInitPreset };

};

}