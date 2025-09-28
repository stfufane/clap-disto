#pragma once

#include "../helpers/IParamControl.h"
#include "visage_widgets/button.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {


class ToggleButton final : public helpers::IParamControl {
public:
    explicit ToggleButton(Disstortion& disstortion, clap_id param_id);

protected:
    void onParameterUpdated(double new_value) override;
    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    visage::Font mFont;
    visage::ToggleTextButton mButton;
};

}
