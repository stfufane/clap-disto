#include "DriveSelector.h"

#include "embedded/disto_fonts.h"
#include "embedded/disto_images.h"

namespace stfefane::gui {

DriveSelector::DriveSelector(Disstortion& d, clap_id param_id)
: IParamControl(d, param_id)
, mFont(20.f, resources::fonts::PressStart2P_ttf)
, mPrevButton(resources::images::prev_type_svg.data, resources::images::prev_type_svg.size)
, mNextButton(resources::images::next_type_svg.data, resources::images::next_type_svg.size) {
    addChild(mPrevButton);
    addChild(mNextButton);

    mPrevButton.onMouseDown() = [&](const visage::MouseEvent&) {
        const auto current_type = mCurrentValue.load(std::memory_order_relaxed);
        auto new_val = static_cast<size_t>(current_type - 1.0) % nbSteps();
        sendNewValue(static_cast<double>(new_val));
    };

    mNextButton.onMouseDown() = [&](const visage::MouseEvent&) {
        const auto current_type = mCurrentValue.load(std::memory_order_relaxed);
        auto new_val = static_cast<size_t>(current_type + 1.0) % nbSteps();
        sendNewValue(static_cast<double>(new_val));
    };
}

void DriveSelector::draw(visage::Canvas& canvas) {
    const auto w = width();
    const auto h = height();

    canvas.setColor(0xffedae49);
    canvas.text(getValueString(), mFont, visage::Font::kCenter, 0, 0, w, h / 2.f);
}

void DriveSelector::resized() {
    const auto half_h = height() / 2.f;
    const auto half_w = width() / 2.f;
    constexpr auto button_margin = 12.f;
    constexpr auto button_width = 44.f;
    constexpr auto button_height = 41.f;

    mPrevButton.setBounds(half_w - button_margin - button_width, half_h, button_width, button_height);
    mNextButton.setBounds(half_w + button_margin, half_h, button_width, button_height);
}
void DriveSelector::sendNewValue(double new_val) {
    beginChangeGesture();
    performChange(new_val);
    endChangeGesture();
}

} // namespace stfefane::gui