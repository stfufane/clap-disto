#pragma once

#include "helpers/IParamControl.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {

class RotaryKnob final : public helpers::IParamControl
{
public:
    enum class Mapping {
        Linear,
        Logarithmic
    };

    explicit RotaryKnob(Disstortion& disstortion, clap_id param_id, Mapping mapping = Mapping::Linear);

    void draw(visage::Canvas& canvas) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;

    // Configure how the knob maps normalized position to value
    void setMapping(Mapping mapping) noexcept { mMapping = mapping; }
    [[nodiscard]] Mapping mapping() const noexcept { return mMapping; }

private:
    // Mapping helpers
    [[nodiscard]] double denormalize(double t) const noexcept; // [0,1] -> value
    [[nodiscard]] double normalize(double value) const noexcept; // value -> [0,1]

    bool mIsDragging = false;
    float mDragStartY = 0.f;
    double mDragStartValue = 0.;
    float mAccumulatedDrag = 0.f; // Accumulated drag in normalized units

    double mRange;
    double mSensitivity;

    Mapping mMapping = Mapping::Linear;

    visage::Font mFont;
};

}