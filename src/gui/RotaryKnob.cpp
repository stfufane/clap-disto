#include "RotaryKnob.h"

#include <numbers>

namespace stfefane::gui {

RotaryKnob::RotaryKnob(const clap_param_info& param_info) : IParamControl(param_info) {}

void RotaryKnob::draw(visage::Canvas& canvas) {
    const auto smaller_size = std::min(width(), height());
    const auto w = smaller_size * 0.8f;
    // Center the knob
    const auto margin_x = (width() - w) * .5f;
    const auto margin_y = (height() - w) * .5f;

    // Draw the knob circle
    canvas.setColor(0xffaaff88);
    canvas.circle(margin_x, margin_y, w);

    constexpr auto kPI = std::numbers::pi_v<float>;
    constexpr auto angle_start = .7f * kPI; // Start at the bottom center (PI/2) with a slight offset (0.2 PI)
    // Clamp the angle to 0.8 PI, it will be doubled from the center, covering 1.6 PI at the maximum value.
    const auto angle = static_cast<float>((mCurrentValue - getMinValue()) / (getMaxValue() - getMinValue())) * kPI * .8f;
    // Shift the center of the radian
    const auto center_radians = angle + angle_start;

    canvas.setColor(0xaa215529);
    canvas.roundedArc(margin_x, margin_y, w, 3.f, center_radians, angle);
    canvas.setColor(0xff222222);
    canvas.flatArc(margin_x, margin_y, w, 3.f, center_radians + angle, .05f);
}

void RotaryKnob::mouseDown(const visage::MouseEvent& e) {
    if (e.isLeftButton()) {
        mIsDragging = true;
        mDragStartY = e.position.y;
    }
}

void RotaryKnob::mouseUp(const visage::MouseEvent& e) {
    if (e.isLeftButton()) {
        mIsDragging = false;
    }
}

void RotaryKnob::mouseDrag(const visage::MouseEvent& e) {
    if (mIsDragging) {
        float dy = mDragStartY - e.position.y;
        const auto range = getMaxValue() - getMinValue();
        mCurrentValue += dy * range / 200.;
        mCurrentValue = std::max(getMinValue(), std::min(getMaxValue(), mCurrentValue));
        mDragStartY = e.position.y;
        redraw();
    }
}

} // namespace stfefane::gui
