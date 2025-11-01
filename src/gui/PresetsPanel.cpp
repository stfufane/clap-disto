#include "PresetsPanel.h"

#include "disstortion.h"
#include "embedded/disto_images.h"
#include "embedded/disto_fonts.h"
#include "presets/PresetManager.h"

namespace stfefane::gui {

const std::string PresetsPanel::kPanelName = "Presets Panel";

PresetsPanel::PresetsPanel(const Disstortion& d)
: visage::Frame(kPanelName)
, presets::PresetManager::Listener(d.getPresetManager())
, mFont(14.f, resources::fonts::PressStart2P_ttf)
, mPrevButton(resources::images::prev_preset_svg.data, resources::images::prev_preset_svg.size)
, mNextButton(resources::images::next_preset_svg.data, resources::images::next_preset_svg.size)
, mSaveButton(resources::images::save_svg.data, resources::images::save_svg.size)
, mPresetName(mPresetManager.getCurrentPreset(), mFont) {

    addChild(mPrevButton);
    addChild(mNextButton);
    addChild(mSaveButton);
    addChild(mPresetName);

    setPaletteOverride(visage::theme::OverrideId(1));

    mPrevButton.onMouseDown() = [&](const visage::MouseEvent&) { mPresetManager.loadPreset(presets::PresetLoad::ePrev); };
    mNextButton.onMouseDown() = [&](const visage::MouseEvent&) { mPresetManager.loadPreset(presets::PresetLoad::eNext); };
    mSaveButton.onMouseDown() = [&](const visage::MouseEvent&) { mPresetManager.savePreset("Un test"); }; // TODO: text input
}

void PresetsPanel::currentPresetChanged(const std::string& new_preset) {
    mPresetName.setText(new_preset);
}

void PresetsPanel::resized() {
    mPrevButton.setBounds(10.f, 8.f, 32.f, 32.f);
    mNextButton.setBounds(48.f, 8.f, 32.f, 32.f);
    mPresetName.setBounds(92.f, 8.f, 300.f, 32.f);
    mSaveButton.setBounds(400.f, 8.f, 32.f, 32.f);
}

void PresetsPanel::draw(visage::Canvas& canvas) {
    // TODO: frame (re-export from Designer)
    canvas.setColor(0xffedae49);
    canvas.roundedRectangle(0.f, 0.f, width(), height(), 16.f);
}

void PresetsPanel::loadPreset(const std::string& preset_name) const {
    mPresetManager.loadPreset(preset_name);
}

} // namespace stfefane::gui
