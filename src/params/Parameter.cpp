#include "Parameter.h"

#include <algorithm>
#include <vector>

namespace stfefane::params {

void Parameter::setValue(const double value) {
    mValue = value;
    notifyAllParamListeners();
}

void Parameter::notifyAllParamListeners() const noexcept {
    for (auto* listener : mUIListeners) {
        listener->onParameterUpdated(mValue);
    }
}

size_t Parameter::nbSteps() const noexcept {
    if (!isStepped()) {
        return 1;
    }
    if (const auto* stepped_param = dynamic_cast<SteppedValueType*>(mValueType.get()); stepped_param) {
        return stepped_param->mValues.size();
    }

    return 1;
}

void Parameter::addUIListener(IParameterUIListener* listener) {
    if (std::ranges::find(mUIListeners, listener) == mUIListeners.end()) {
        mUIListeners.push_back(listener);
    }
}

void Parameter::removeUIListener(IParameterUIListener* listener) {
    std::erase(mUIListeners, listener);
}

} // namespace stfefane::params