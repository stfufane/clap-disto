#include "RotaryKnob.h"

#include <numbers>
#include "../params.h"


RotaryKnob::RotaryKnob(uint32_t param_id)
{
    clap_param_info param_info;
    stfefane::params::Parameters::getParamInfo(param_id, &param_info);
    value = param_info.default_value;
    min_value = param_info.min_value;
    max_value = param_info.max_value;
}

void RotaryKnob::draw(visage::Canvas& canvas)
{
    const auto smaller_size = std::min(width(), height());
    const auto w = smaller_size * 0.8f;
    const auto margin_x = width() * .1f;
    const auto margin_y = height() * .1f;

    // Draw the knob circle
    canvas.setColor(0xffaaff88);
    canvas.circle(margin_x, margin_y, w);

    constexpr auto kPI = std::numbers::pi_v<float>;
    constexpr auto angle_start = .7f * kPI; // Start at the bottom center (PI/2) with a slight offset (0.2 PI)
    // Clamp the angle to 0.8 PI, it will be doubled from the center, covering 1.6 PI at the maximum value.
    const auto angle = static_cast<float>((value - min_value) / (max_value - min_value)) * kPI * .8f;
    // Shift the center of the radian
    const auto center_radians = angle + angle_start;

    canvas.setColor(0xaa215529);
    canvas.roundedArc(margin_x, margin_y, w, 3.f, center_radians, angle);
    canvas.setColor(0xff222222);
    canvas.flatArc(margin_x, margin_y, w, 3.f, center_radians + angle, .05f);
}

void RotaryKnob::mouseDown(const visage::MouseEvent& e) {
    if (e.isLeftButton())
    {
        is_dragging = true;
        drag_start_y = e.position.y;
    }
}

void RotaryKnob::mouseUp(const visage::MouseEvent& e) {
    if (e.isLeftButton())
    {
        is_dragging = false;
    }
}

void RotaryKnob::mouseDrag(const visage::MouseEvent& e) {
    if (is_dragging)
    {
        float dy = drag_start_y - e.position.y;
        auto range = max_value - min_value;
        value += dy * range / 200.;
        value = std::max(min_value, std::min(max_value, value));
        drag_start_y = e.position.y;
        redraw();
    }
}