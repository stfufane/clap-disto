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

    void mouseDown(const visage::MouseEvent& e) override;
    void mouseEnter(const visage::MouseEvent& e) override;
    void mouseExit(const visage::MouseEvent& e) override;

protected:
    void draw(visage::Canvas& canvas) override;
};

}
