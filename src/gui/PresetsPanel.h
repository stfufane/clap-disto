#pragma once

#include "presets/PresetManager.h"

#include <visage/ui.h>
#include <visage/widgets.h>

namespace stfefane {
class Disstortion;
} // namespace stfefane

namespace stfefane::gui {

class PresetsPanel : public visage::Frame, public presets::PresetManager::Listener {
public:
    explicit PresetsPanel(const Disstortion& d);

    void currentPresetChanged(const std::string& new_preset) override;
    void resized() override;
    void draw(visage::Canvas& canvas) override;

    static const std::string kPanelName;

private:
    void loadPreset(const std::string& preset_name) const;

    visage::Font mFont;

    visage::IconButton mPrevButton;
    visage::IconButton mNextButton;
    visage::IconButton mSaveButton;

    visage::UiButton mPresetName;
};

} // namespace stfefane::gui