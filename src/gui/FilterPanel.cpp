#include "FilterPanel.h"

#include "disstortion.h"
#include "embedded/disto_images.h"
#include "params/Parameters.h"

namespace stfefane::gui {

FilterPanel::FilterPanel(Disstortion& d, const std::string& name, clap_id on_off, clap_id freq, clap_id res, clap_id gain, clap_id type)
: visage::Frame(name)
, mOnOff(d, on_off)
, mFreq(d, freq)
, mRes(d, res)
, mGain(d, gain)
, mType(d, type) {
    addChild(mOnOff);
    addChild(mFreq);
    addChild(mRes);
    addChild(mGain);
    addChild(mType);

    mIsPre = on_off == params::ePreFilterOn;

    mFreq.setFontSize(10.f);
    mRes.setFontSize(10.f);
    mGain.setFontSize(10.f);
    mGain.setDisplayUnit(false);

    mOnOffAttachment = std::make_unique<params::ParameterAttachment>(d.getParameter(on_off), [&](params::Parameter*, double new_val) {
        mIsOn = new_val > 0.5;
        redraw();
    });
}

void FilterPanel::draw(visage::Canvas& canvas) {
    // TODO: draw text manually to handle color fade
    canvas.setColor(mIsOn ? 0xffffffff : 0xffa55555);
    if (mIsPre) {
        canvas.svg(resources::images::pre_filter_frame_svg.data, resources::images::pre_filter_frame_svg.size, 0.f, 0.f, width(), height());
    } else {
        canvas.svg(resources::images::post_filter_frame_svg.data, resources::images::post_filter_frame_svg.size, 0.f, 0.f, width(), height());
    }
}

void FilterPanel::resized() {
    const auto switch_x = mIsPre ? 11.5f : 186.1f;
    mOnOff.setBounds(switch_x, 12.8f, 17.3f, 17.3f);
    mFreq.setBounds(19.1f, 120.4f, 75.f, 75.f);
    mRes.setBounds(107.1f, 140.4f, 34.f, 34.f);
    mGain.setBounds(157.1f, 140.4f, 34.f, 34.f);
    mType.setBounds(21.7f, 74.2f, 169.1f, 42.f);
}

} // namespace stfefane::gui
