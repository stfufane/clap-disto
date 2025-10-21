#include "RotaryKnob.h"

#include "embedded/disto_fonts.h"
#include "helpers/Logger.h"
#include "helpers/Utils.h"
#include "params/Parameter.h"

#include <algorithm>
#include <cmath>

namespace stfefane::gui {

RotaryKnob::RotaryKnob(Disstortion& disstortion, clap_id param_id) : IParamControl(disstortion, param_id)
    , mSensitivity(200.)
    , mFont(12.f, resources::fonts::DroidSansMono_ttf) {
    if (isStepped()) {
        mSensitivity = 100.;
    }
}

void RotaryKnob::draw(visage::Canvas& canvas) {
    // Keep the bottom 20% of the height for a label
    const auto knob_height = height() * .8f;

    const auto smaller_size = std::min(width(), knob_height);
    const auto w = smaller_size * 0.9f;

    // Center the knob
    const auto margin_x = (width() - w) * .5f;
    const auto margin_y = (knob_height - w) * .5f;
    const auto center_x = margin_x + w * .5f;
    const auto center_y = margin_y + w * .5f;

    canvas.setColor(0xff325232);
    canvas.roundedRectangleBorder(margin_x, knob_height, w, height() - knob_height, 8.f, 1.f);

    // Draw the knob circle
    canvas.setColor(0xffaaff88);
    canvas.circle(margin_x, margin_y, w);

    constexpr auto angle_start = .7f * utils::kPI; // Start at the bottom center (PI/2) with a slight offset (0.2 PI)
    // Use normalized value to compute the angle
    const double t = getNormalizedCurrentValue();
    // Clamp the angle to 0.8 PI, it will be doubled from the center, covering 1.6 PI at the maximum value.
    const auto angle = static_cast<float>(t) * utils::kPI * .8f;
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

    // Write the text value of the knob at the center.
    canvas.setColor(0xff111111);
    canvas.text(getValueString(), mFont.withSize(12.f), visage::Font::kCenter, 0, 0, width(), knob_height);
    canvas.text(mTitle, mFont.withSize(10.f), visage::Font::kCenter, margin_x, knob_height, w, height() - knob_height);
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
        mDragStartValue = getNormalizedCurrentValue();
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
    // Accumulate drag in normalized space so mapping can be applied
    mAccumulatedDrag += dy / static_cast<float>(mSensitivity * (e.isMainModifier() ? 4.f : 1.f));
    handleMouseDelta();
    mDragStartY = e.position.y;
}

bool RotaryKnob::mouseWheel(const visage::MouseEvent& e) {
    if (mIsDragging) {
        return false;
    }
    mDragStartValue = getNormalizedCurrentValue();
    mAccumulatedDrag = e.wheel_delta_y / (isStepped() ? 5.f : mSensitivity * .5f);
    handleMouseDelta();
    return true;
}

void RotaryKnob::handleMouseDelta() {
    double target_t = mDragStartValue + static_cast<double>(mAccumulatedDrag);
    if (isStepped()) {
        // For stepped parameters, calculate which step we're closest to (linear spacing across actual range)
        target_t = std::round(target_t * static_cast<double>(nbSteps() - 1));
    }
    const auto new_value = std::clamp(target_t, getMinValue(), getMaxValue());

    if (new_value != mCurrentValue) {
        performChange(new_value);
        mCurrentValue.store(new_value, std::memory_order::relaxed);
        redraw();
    }
}

} // namespace stfefane::gui
