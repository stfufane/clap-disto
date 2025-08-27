#pragma once

#include "../params/Parameters.h"

#include <visage/ui.h>
#include <clap/id.h>
#include <thread>

namespace stfefane {
class Disstortion;
}

namespace stfefane::helpers {

class IParamControl : public visage::Frame, public params::IParameterUIListener {
public:
    explicit IParamControl(Disstortion& disstortion, clap_id param_id);
    IParamControl() = delete;

protected:
    [[nodiscard]] double getMinValue() const noexcept { return mParam->getInfo().min_value; }
    [[nodiscard]] double getMaxValue() const noexcept { return mParam->getInfo().max_value; }
    [[nodiscard]] bool isStepped() const noexcept { return mParam->isStepped(); }
    [[nodiscard]] size_t nbSteps() const noexcept { return mParam->nbSteps(); }

    // To be called by the UI element on mouse gestures
    void beginChangeGesture();
    void performChange(double new_value);
    void endChangeGesture();

    void onParameterUpdated(double new_value) override;

    double mCurrentValue = 0.;

private:
    Disstortion& mDisstortion;
    clap_id mParamId = UINT32_MAX;
};

}
