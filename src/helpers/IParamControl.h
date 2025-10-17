#pragma once

#include "../params/Parameters.h"

#include <visage/ui.h>
#include <clap/id.h>
#include <atomic>
#include <thread>

namespace stfefane {
class Disstortion;
}

namespace stfefane::helpers {

class IParamControl : public params::IParameterListener, public visage::Frame {
public:
    explicit IParamControl(Disstortion& disstortion, clap_id param_id);
    IParamControl() = delete;

protected:
    [[nodiscard]] double getMinValue() const noexcept { return mParam->getInfo().min_value; }
    [[nodiscard]] double getMaxValue() const noexcept { return mParam->getInfo().max_value; }
    [[nodiscard]] bool isStepped() const noexcept { return mParam->isStepped(); }
    [[nodiscard]] size_t nbSteps() const noexcept { return mParam->nbSteps(); }
    [[nodiscard]] std::string getValueString() const noexcept { return mParam->getValueType().toText(mCurrentValue); }

    // To be called by the UI element on mouse gestures
    void beginChangeGesture();
    void performChange(double new_value);
    void endChangeGesture();

    // Called on double click to reset a parameter value to its default
    void resetParam();

    void onParameterUpdated(double new_value) override;

    std::atomic<double> mCurrentValue = 0.;
    std::string mTitle;

private:
    Disstortion& mDisstortion;
    clap_id mParamId = UINT32_MAX;
};

}
