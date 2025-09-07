#pragma once

#include <visage/app.h>
#include "RotaryKnob.h"

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
    void setupKnob(RotaryKnob& knob);

    // Reference to the plugin
    Disstortion& mDisstortion;

    visage::ScrollableFrame mScrollable;

    RotaryKnob mDriveType;
    RotaryKnob mDrive;
    RotaryKnob mInputGain;
    RotaryKnob mOutputGain;
    RotaryKnob mPreFilter;
    RotaryKnob mPostFilter;

    static constexpr int kMinHeight = 120;
    static constexpr int kMaxHeight = 800;
    static constexpr int kMinWidth = 120;
    static constexpr int kMaxWidth = 800;
};

} // namespace gui::stfefane
