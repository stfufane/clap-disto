#pragma once
#include "IParamControl.h"

namespace stfefane::gui {

class FilterSelector : public IParamControl {
public:
    FilterSelector(Disstortion& d, clap_id param_id);

    enum class Type {
        eHiPass,
        eLowPass,
        eBandPass
    };

    void draw(visage::Canvas& canvas) override;

private:
    Type mActive = Type::eHiPass;
};

}