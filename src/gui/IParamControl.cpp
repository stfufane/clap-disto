#include "IParamControl.h"

#include "disstortion.h"
#include "utils/Logger.h"

namespace stfefane::gui {

IParamControl::IParamControl(Disstortion& disstortion, clap_id param_id)
    : IParameterListener(disstortion.getParameter(param_id))
    , visage::Frame(mParam->getInfo().name)
    , mTitle(mParam->getInfo().name)
    , mDisstortion(disstortion)
    , mParamId(mParam->getInfo().id) {
    mCurrentValue = mParam->getValue();
}

double IParamControl::getMinValue() const noexcept {
    return mParam->getInfo().min_value;
}

double IParamControl::getMaxValue() const noexcept {
    return mParam->getInfo().max_value;
}

double IParamControl::getNormalizedCurrentValue() const noexcept {
    return !isStepped()
        ? mCurrentValue.load(std::memory_order::relaxed)
        : mParam->getValueType().normalizedValue(mCurrentValue.load(std::memory_order::relaxed));
}

bool IParamControl::isStepped() const noexcept {
    return mParam->isStepped();
}

size_t IParamControl::nbSteps() const noexcept {
    return mParam->nbSteps();
}

void IParamControl::onParameterUpdated(double new_value) {
    LOG_INFO("param", "[IParamControl::onParameterUpdated] -> {} = {}", mParam->getInfo().name, new_value);
    mCurrentValue.store(new_value, std::memory_order_relaxed);
    redraw();
}

void IParamControl::resetParam() {
    beginChangeGesture();
    performChange(mParam->getInfo().default_value);
    endChangeGesture();
}

std::string IParamControl::getValueString(bool display_unit) const noexcept {
    return mParam->getValueType().toText(mCurrentValue.load(std::memory_order_relaxed), display_unit);
}

void IParamControl::beginChangeGesture() { mDisstortion.beginParameterChange(mParamId); }
void IParamControl::performChange(double new_value) { mDisstortion.updateParameterChange(mParamId, new_value); }
void IParamControl::endChangeGesture() { mDisstortion.endParameterChange(mParamId); }

}