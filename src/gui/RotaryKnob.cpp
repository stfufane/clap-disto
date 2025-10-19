#include "RotaryKnob.h"

#include <algorithm>
#include <cmath>
#include "embedded/disto_fonts.h"
#include "helpers/Utils.h"

namespace stfefane::gui {

RotaryKnob::RotaryKnob(Disstortion& disstortion, clap_id param_id, Mapping mapping) : IParamControl(disstortion, param_id)
    , mRange(getMaxValue() - getMinValue())
    , mSensitivity(200.)
    , mMapping(mapping)
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
    // Compute normalized position depending on mapping
    const double t = normalize(mCurrentValue);
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
    // Accumulate drag in normalized space so mapping can be applied
    mAccumulatedDrag += dy / static_cast<float>(mSensitivity);

    // Starting normalized position from the start value
    double start_t = normalize(mDragStartValue);
    double target_t = start_t + static_cast<double>(mAccumulatedDrag);
    target_t = std::clamp(target_t, 0.0, 1.0);

    double target_value = denormalize(target_t);
    target_value = std::clamp(target_value, getMinValue(), getMaxValue());
    double new_value = target_value;

    if (isStepped()) {
        // For stepped parameters, calculate which step we're closest to (linear spacing across actual range)
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

// Mapping helpers
double RotaryKnob::denormalize(double t) const noexcept {
    t = std::clamp(t, 0.0, 1.0);
    const double min = getMinValue();
    const double max = getMaxValue();
    switch (mMapping) {
        case Mapping::Linear:
            return min + t * (max - min);
        case Mapping::Logarithmic: {
            // Guard against non-positive bounds; fallback to linear in that case
            if (min <= 0.0 || max <= 0.0) {
                return min + t * (max - min);
            }
            const double log_min = std::log(min);
            const double log_max = std::log(max);
            const double log_val = log_min + t * (log_max - log_min);
            return std::exp(log_val);
        }
    }
    // Fallback
    return min + t * (max - min);
}

double RotaryKnob::normalize(double value) const noexcept {
    const double min = getMinValue();
    const double max = getMaxValue();
    const double range = max - min;
    if (range <= 0.0) return 0.0;
    switch (mMapping) {
        case Mapping::Linear:
            return std::clamp((value - min) / range, 0.0, 1.0);
        case Mapping::Logarithmic: {
            if (min <= 0.0 || max <= 0.0 || value <= 0.0) {
                return std::clamp((value - min) / range, 0.0, 1.0);
            }
            const double log_min = std::log(min);
            const double log_max = std::log(max);
            const double log_v = std::log(value);
            const double t = (log_v - log_min) / (log_max - log_min);
            return std::clamp(t, 0.0, 1.0);
        }
    }
    return std::clamp((value - min) / range, 0.0, 1.0);
}

} // namespace stfefane::gui
