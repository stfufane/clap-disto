#pragma once

#include <visage/ui.h>
#include "../helpers/IParamControl.h"

namespace stfefane::gui {

class RotaryKnob final : public helpers::IParamControl
{
public:
    explicit RotaryKnob(params::Parameter* param);

    void draw(visage::Canvas& canvas) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;

private:

    bool mIsDragging = false;
    float mDragStartY = 0.f;
};

}