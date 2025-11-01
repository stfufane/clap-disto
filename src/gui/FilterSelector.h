#pragma once

#include <visage/widgets.h>
#include "IParamControl.h"

namespace stfefane::gui {

class FilterSelector : public IParamControl {
public:
    FilterSelector(Disstortion& d, clap_id param_id);

    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    void setNewValue(double new_val);

    visage::ToggleIconButton mLowPassButton;
    visage::ToggleIconButton mHiPassButton;
    visage::ToggleIconButton mBandPassButton;
};

}