#pragma once

#include <atomic>
#include <thread>
#include <visage/ui.h>
#include <clap/id.h>

#include "params/IParameterListener.h"

namespace stfefane {
class Disstortion;
}

namespace stfefane::helpers {

class IParamControl : public params::IParameterListener, public visage::Frame {
public:
    explicit IParamControl(Disstortion& disstortion, clap_id param_id);
    IParamControl() = delete;

protected:
    [[nodiscard]] double getMinValue() const noexcept;
    [[nodiscard]] double getMaxValue() const noexcept;
    [[nodiscard]] bool isStepped() const noexcept;
    [[nodiscard]] size_t nbSteps() const noexcept;
    [[nodiscard]] std::string getValueString() const noexcept;

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
