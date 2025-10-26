#pragma once

#include "FilterSelector.h"
#include "RotaryKnob.h"
#include "ToggleButton.h"

#include <clap/id.h>
#include <visage/ui.h>

namespace stfefane::gui {

namespace stfefane {
class Disstortion;
}

class FilterPanel : public visage::Frame {
public:
    FilterPanel(Disstortion& d, const std::string& name, clap_id on_off, clap_id freq, clap_id res, clap_id gain, clap_id type);

    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    ToggleButton mOnOff;
    RotaryKnob mFreq;
    RotaryKnob mRes;
    RotaryKnob mGain;
    FilterSelector mType;

    bool mIsPre = true;
    bool mIsOn = true;
    std::unique_ptr<params::ParameterAttachment> mOnOffAttachment;
};

}
