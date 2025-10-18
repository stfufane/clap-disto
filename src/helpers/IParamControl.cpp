#include "IParamControl.h"

#include "../disstortion.h"

#include <spdlog/spdlog.h>

namespace stfefane::helpers {

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

bool IParamControl::isStepped() const noexcept {
    return mParam->isStepped();
}

size_t IParamControl::nbSteps() const noexcept {
    return mParam->nbSteps();
}

void IParamControl::onParameterUpdated(double new_value) {
    spdlog::debug("[IParamControl::onParameterUpdated] -> {} = {}", mParam->getInfo().name, new_value);
    mCurrentValue = new_value;
    redraw();
}

void IParamControl::resetParam() {
    beginChangeGesture();
    performChange(mParam->getInfo().default_value);
    endChangeGesture();
}

std::string IParamControl::getValueString() const noexcept {
    return mParam->getValueType().toText(mCurrentValue.load(std::memory_order_relaxed));
}

void IParamControl::beginChangeGesture() { mDisstortion.beginParameterChange(mParamId); }
void IParamControl::performChange(double new_value) { mDisstortion.updateParameterChange(mParamId, new_value); }
void IParamControl::endChangeGesture() { mDisstortion.endParameterChange(mParamId); }

}