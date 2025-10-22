#pragma once

#include "gui/IParamControl.h"
#include "visage_widgets/button.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::gui {


class ToggleButton final : public IParamControl {
public:
    explicit ToggleButton(Disstortion& disstortion, clap_id param_id);

protected:
    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    visage::Font mFont;
    visage::ToggleTextButton mButton;
};

}
