#pragma once

#include "IParamControl.h"
#include "visage_widgets/button.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {

class DriveSelector final : public IParamControl {
public:
    explicit DriveSelector(Disstortion& d, clap_id param_id);

    void draw(visage::Canvas& canvas) override;
    void resized() override;
private:
    void sendNewValue(double new_val);

    visage::Font mFont;

    visage::IconButton mPrevButton;
    visage::IconButton mNextButton;
};

}
