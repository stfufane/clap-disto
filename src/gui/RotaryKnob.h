#pragma once

#include "gui/IParamControl.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {

class RotaryKnob final : public IParamControl
{
public:
    explicit RotaryKnob(Disstortion& disstortion, clap_id param_id);

    void draw(visage::Canvas& canvas) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;
    bool mouseWheel(const visage::MouseEvent& e) override;

private:
    void handleMouseDelta();

    bool mIsDragging = false;
    float mDragStartY = 0.f;
    double mDragStartValue = 0.;
    float mAccumulatedDrag = 0.f; // Accumulated drag in normalized units

    double mSensitivity;

    visage::Font mFont;
};

}