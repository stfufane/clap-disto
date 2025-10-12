#include "IParamControl.h"

#include "../disstortion.h"

namespace stfefane::helpers {

IParamControl::IParamControl(Disstortion& disstortion, clap_id param_id)
    : IParameterUIListener(disstortion.getParameter(param_id))
    , visage::Frame(mParam->getInfo().name)
    , mTitle(mParam->getInfo().name)
    , mDisstortion(disstortion)
    , mParamId(mParam->getInfo().id) {
    mCurrentValue = mParam->getValue();
}

void IParamControl::onParameterUpdated(double new_value) {
    mCurrentValue = new_value;
    redraw();
}

void IParamControl::resetParam() {
    beginChangeGesture();
    performChange(mParam->getInfo().default_value);
    endChangeGesture();
}

void IParamControl::beginChangeGesture() { mDisstortion.beginParameterChange(mParamId); }
void IParamControl::performChange(double new_value) { mDisstortion.updateParameterChange(mParamId, new_value); }
void IParamControl::endChangeGesture() { mDisstortion.endParameterChange(mParamId); }

}