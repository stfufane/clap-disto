#include "RotaryKnob.h"

#include "../helpers/Utils.h"

namespace stfefane::gui {

RotaryKnob::RotaryKnob(Disstortion& disstortion, clap_id param_id) : IParamControl(disstortion, param_id)
    , mRange(getMaxValue() - getMinValue()) {
    mSensitivity = 200.;
    if (isStepped()) {
        // Scale sensitivity based on number of steps - more steps = less sensitive
        mSensitivity = 100.; // std::max(80.0f, 200.0f / nbSteps());
    }
}

void RotaryKnob::draw(visage::Canvas& canvas) {
    const auto smaller_size = std::min(width(), height());
    const auto w = smaller_size * 0.8f;

    // Center the knob
    const auto margin_x = (width() - w) * .5f;
    const auto margin_y = (height() - w) * .5f;
    const auto center_x = margin_x + w * 0.5f;
    const auto center_y = margin_y + w * 0.5f;

    // Draw the knob circle
    canvas.setColor(0xffaaff88);
    canvas.circle(margin_x, margin_y, w);

    constexpr auto angle_start = .7f * utils::kPI; // Start at the bottom center (PI/2) with a slight offset (0.2 PI)
    // Clamp the angle to 0.8 PI, it will be doubled from the center, covering 1.6 PI at the maximum value.
    const auto angle = static_cast<float>((mCurrentValue - getMinValue()) / (getMaxValue() - getMinValue())) * utils::kPI * .8f;
    // Shift the center of the radian
    const auto center_radians = angle + angle_start;

    canvas.setColor(0xaa215529);

    // Draw the indicator dot
    const auto radius = w * 0.5f;
    const auto dot_location = radius * 0.9f;   // Place the dot at 80% from center
    const auto dot_x = center_x + std::cos(center_radians + angle) * dot_location;
    const auto dot_y = center_y + std::sin(center_radians + angle) * dot_location;

    const auto dot_radius = w * .05f;
    canvas.circle(dot_x - dot_radius, dot_y - dot_radius, dot_radius * 2);
}

void RotaryKnob::mouseDown(const visage::MouseEvent& e) {
    if (e.isLeftButton()) {
        // Reset param on double click
        if (e.repeat_click_count > 1) {
            return resetParam();
        }
        beginChangeGesture();
        mIsDragging = true;
        mDragStartY = e.position.y;
        mDragStartValue = mCurrentValue;
        mAccumulatedDrag = 0.f;
    }
}

void RotaryKnob::mouseUp(const visage::MouseEvent& e) {
    if (e.isLeftButton()) {
        mIsDragging = false;
        endChangeGesture();
    }
}

void RotaryKnob::mouseDrag(const visage::MouseEvent& e) {
    if (!mIsDragging) {
        return;
    }

    const float dy = mDragStartY - e.position.y;
    mAccumulatedDrag += dy * mRange / mSensitivity;

    auto target_value = mDragStartValue + mAccumulatedDrag;
    target_value = std::max(getMinValue(), std::min(getMaxValue(), target_value));
    double new_value = target_value;

    if (isStepped()) {
        // For stepped parameters, calculate which step we're closest to
        const auto num_steps = nbSteps();
        const double step_size = mRange / static_cast<double>(num_steps - 1);

        // Calculate which step the continuous value corresponds to
        const auto normalized_pos = (target_value - getMinValue()) / mRange;
        const int target_step = std::round(normalized_pos * static_cast<double>(num_steps - 1));

        // Convert back to actual value
        new_value = getMinValue() + target_step * step_size;
    }

    if (new_value != mCurrentValue) {
        performChange(new_value);
        mCurrentValue = new_value;
        redraw();
    }

    mDragStartY = e.position.y;
}

} // namespace stfefane::gui
