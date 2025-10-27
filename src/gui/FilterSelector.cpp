#include "FilterSelector.h"

#include "disstortion.h"
#include "embedded/disto_images.h"

namespace stfefane::gui {

FilterSelector::FilterSelector(Disstortion& d, clap_id param_id)
: IParamControl(d, param_id)
, mLowPassButton("lowpass_selector", resources::images::lowpass_svg.data, resources::images::lowpass_svg.size)
, mHiPassButton("hipass_selector", resources::images::hipass_svg.data, resources::images::hipass_svg.size)
, mBandPassButton("bandpass_selector", resources::images::bandpass_svg.data, resources::images::bandpass_svg.size) {

    // To ensure there's always one toggled button, we don't "untoggle" them when they're clicked, it will be updated by the param update
    mLowPassButton.onToggle() = [&](visage::Button*, bool) {
        setNewValue(0.);
    };
    mHiPassButton.onToggle() = [&](visage::Button*, bool)  {
        setNewValue(1.);
    };
    mBandPassButton.onToggle() = [&](visage::Button*, bool) {
        setNewValue(2.);
    };

    addChild(mLowPassButton);
    addChild(mHiPassButton);
    addChild(mBandPassButton);
}

void FilterSelector::draw(visage::Canvas& canvas) {
    const auto current_val = mCurrentValue.load(std::memory_order_relaxed);
    mLowPassButton.setToggled(current_val == 0.);
    mHiPassButton.setToggled(current_val == 1.);
    mBandPassButton.setToggled(current_val == 2.);
}

void FilterSelector::resized() {
    const auto w_3 = width() / 3.f;
    const auto h = height();
    mLowPassButton.setBounds(0.f, 0.f, w_3, h);
    mHiPassButton.setBounds(w_3, 0., w_3, h);
    mBandPassButton.setBounds(w_3 * 2.f, 0., w_3, h);
}

void FilterSelector::setNewValue(double new_val) {
    beginChangeGesture();
    performChange(new_val);
    endChangeGesture();
}

} // namespace stfefane::gui
