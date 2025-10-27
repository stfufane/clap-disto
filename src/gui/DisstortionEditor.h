#pragma once

#include "DriveSelector.h"
#include "FilterPanel.h"
#include "RotaryKnob.h"

#include <visage/app.h>

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {

class DisstortionEditor : public visage::ApplicationWindow {
public:
    explicit DisstortionEditor(Disstortion& d);
    DisstortionEditor() = delete;

    void draw(visage::Canvas& canvas) override;
    void resized() override;

    [[nodiscard]] int pluginWidth() const;
    [[nodiscard]] int pluginHeight() const;
    void setPluginDimensions(int width, int height);

    static constexpr auto kWidth = 600.f;
    static constexpr auto kHeight = 650.f;
private:
    // Reference to the plugin
    Disstortion& mDisstortion;

    visage::Palette mPalette;

    RotaryKnob mInputGain;
    RotaryKnob mOutputGain;
    RotaryKnob mDrive;
    RotaryKnob mAsymmetry;
    RotaryKnob mMix;

    DriveSelector mDriveSelector;

    FilterPanel mPreFilter;
    FilterPanel mPostFilter;

    std::unique_ptr<visage::ShaderPostEffect> mGlitchShader;

    std::unique_ptr<params::ParameterAttachment> mDriveAttachment;
};

} // namespace gui::stfefane
