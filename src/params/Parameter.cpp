#include "Parameter.h"

#include <algorithm>
#include <vector>

#include "IParameterListener.h"

namespace stfefane::params {

void Parameter::setValue(const double value) {
    mValue.store(value, std::memory_order_relaxed);
    notifyAllParamListeners();
}

void Parameter::notifyAllParamListeners() const noexcept {
    for (auto* listener : mListeners) {
        listener->onParameterUpdated(getValue());
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

void Parameter::addListener(IParameterListener* listener) {
    if (std::ranges::find(mListeners, listener) == mListeners.end()) {
        mListeners.push_back(listener);
    }
}

void Parameter::removeListener(IParameterListener* listener) {
    std::erase(mListeners, listener);
}

} // namespace stfefane::params