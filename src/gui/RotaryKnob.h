#pragma once

#include "helpers/IParamControl.h"
#include "helpers/Utils.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {

class RotaryKnob final : public helpers::IParamControl
{
public:
    explicit RotaryKnob(Disstortion& disstortion, clap_id param_id, utils::Mapping mapping = utils::Mapping::Linear);

    void draw(visage::Canvas& canvas) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;

private:


    bool mIsDragging = false;
    float mDragStartY = 0.f;
    double mDragStartValue = 0.;
    float mAccumulatedDrag = 0.f; // Accumulated drag in normalized units

    double mRange;
    double mSensitivity;

    utils::Mapping mMapping = utils::Mapping::Linear;

    visage::Font mFont;
};

}