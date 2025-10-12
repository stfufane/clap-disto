#include "ToggleButton.h"

#include "embedded/disto_fonts.h"

namespace stfefane::gui {

ToggleButton::ToggleButton(Disstortion& disstortion, clap_id param_id)
    : IParamControl(disstortion, param_id),
      mFont(12.f, resources::fonts::DroidSansMono_ttf),
      mButton(name() + " ToggleButton", mFont) {
    mButton.setToggled(mParam->getValue() > .5);
    mButton.onToggle() = [this]([[maybe_unused]] visage::Button* button, bool toggled) {
        beginChangeGesture();
        performChange(toggled);
        endChangeGesture();
        mButton.setText(toggled ? "On" : "Off");
    };
    addChild(mButton);
}

void ToggleButton::draw(visage::Canvas& canvas) {
    const auto button_height = height() * .8f;
    const auto w = width() * 0.9f;
    const auto margin_x = (width() - w) * .5f;

    canvas.setColor(0xff325232);
    canvas.roundedRectangleBorder(margin_x, button_height, w, height() - button_height, 8.f, 1.f);

    canvas.setColor(0xff111111);
    canvas.text(mTitle, mFont.withSize(10.f), visage::Font::kCenter, margin_x, button_height, w, height() - button_height);

    mButton.setToggled(mCurrentValue > .5);
    mButton.setText(mCurrentValue > .5 ? "On" : "Off");
}

void ToggleButton::resized() {
    IParamControl::resized();
    const auto button_height = height() * .8f;
    const auto smaller_size = std::min(width(), button_height);
    const auto w = smaller_size * .6f;

    // Center the button
    const auto margin_x = (width() - w) * .5f;
    const auto margin_y = (button_height - w) * .5f;

    mButton.setBounds(margin_x, margin_y, w, button_height * .5f);
}

} // namespace stfefane::gui