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
    explicit DisstortionEditor(Disstortion& disstortion);
    DisstortionEditor() = delete;

    void draw(visage::Canvas& canvas) override;
    void resized() override;

    [[nodiscard]] int pluginWidth() const;
    [[nodiscard]] int pluginHeight() const;
    void setPluginDimensions(int width, int height);

private:
    void setupElement(visage::Frame& element);

    // Reference to the plugin
    Disstortion& mDisstortion;

    visage::ScrollableFrame mScrollable;

    RotaryKnob mDrive;
    RotaryKnob mDriveType;
    RotaryKnob mMix;
    RotaryKnob mBias;
    RotaryKnob mAsymmetry;
    RotaryKnob mInputGain;
    RotaryKnob mOutputGain;
    RotaryKnob mPreFilter;
    RotaryKnob mPostFilter;
    ToggleButton mPreFilterOn;
    ToggleButton mPostFilterOn;


    static constexpr int kMinHeight = 120;
    static constexpr int kMaxHeight = 800;
    static constexpr int kMinWidth = 120;
    static constexpr int kMaxWidth = 800;
};

} // namespace gui::stfefane
