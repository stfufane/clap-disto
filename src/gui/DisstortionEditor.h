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
    // Reference to the plugin
    Disstortion& mDisstortion;

    RotaryKnob mDriveKnob;
    RotaryKnob mGainKnob;
    RotaryKnob mCutoffKnob;
};

} // namespace gui::stfefane
