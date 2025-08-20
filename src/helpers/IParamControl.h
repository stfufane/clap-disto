#pragma once

#include "../params/Parameters.h"
#include <clap/id.h>

namespace stfefane::helpers {

class IParamControl : public visage::Frame, public params::IParameterUIListener {
public:
    explicit IParamControl(params::Parameter* param): IParameterUIListener(param), mParamId(param->getInfo().id) {
        mCurrentValue = param->getInfo().default_value;
    };
    IParamControl() = delete;

protected:
    [[nodiscard]] double getMinValue() const noexcept { return mParam->getInfo().min_value; }
    [[nodiscard]] double getMaxValue() const noexcept { return mParam->getInfo().max_value; }

    void onParameterUpdated(double new_value) override {
        mCurrentValue = new_value;
        redraw();
    }

    clap_id mParamId = UINT32_MAX;

    double mCurrentValue = 0.;
};

}
