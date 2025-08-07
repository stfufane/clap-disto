#pragma once

#include <visage/app.h>
#include <vector>
#include "RotaryKnob.h"

namespace stfefane::gui {

class DisstortionEditor : public visage::ApplicationWindow {
public:
    DisstortionEditor();

    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    RotaryKnob mGainKnob;
};

} // namespace gui::stfefane
