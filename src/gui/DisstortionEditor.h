#pragma once

#include <visage/app.h>
#include "RotaryKnob.h"
#include "ToggleButton.h"

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

    RotaryKnob mInputGain;
    RotaryKnob mOutputGain;
    RotaryKnob mDrive;
    RotaryKnob mAsymmetry;
    RotaryKnob mMix;

    RotaryKnob mPreFreq;
    RotaryKnob mPreRes;
    RotaryKnob mPreGain;

    RotaryKnob mPostFreq;
    RotaryKnob mPostRes;
    RotaryKnob mPostGain;

    std::unique_ptr<visage::ShaderPostEffect> mGlitchShader;
};

} // namespace gui::stfefane
