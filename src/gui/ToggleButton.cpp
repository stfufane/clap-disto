#include "ToggleButton.h"

#include "embedded/disto_images.h"
#include "params/Parameter.h"

namespace stfefane::gui {

ToggleButton::ToggleButton(Disstortion& disstortion, clap_id param_id)
    : IParamControl(disstortion, param_id) {
}

void ToggleButton::mouseDown(const visage::MouseEvent& e) {
    const auto toggled = mCurrentValue.load(std::memory_order_relaxed) > .5;
    beginChangeGesture();
    performChange(!toggled);
    endChangeGesture();
}

void ToggleButton::mouseEnter(const visage::MouseEvent& e) {
    setCursorStyle(visage::MouseCursor::Pointing);
}

void ToggleButton::mouseExit(const visage::MouseEvent& e) {
    setCursorStyle(visage::MouseCursor::Arrow);
}

void ToggleButton::draw(visage::Canvas& canvas) {
    canvas.setColor(mCurrentValue.load(std::memory_order_relaxed) > .5 ? 0xffffffff : 0xffa55555);
    canvas.svg(resources::images::on_off_svg.data, resources::images::on_off_svg.size, 0.f, 0.f, width(), height());
}

} // namespace stfefane::gui