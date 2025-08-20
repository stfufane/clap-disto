#include "Parameter.h"

#include <algorithm>
#include <vector>

namespace stfefane::params {

void Parameter::setValue(const double value) {
    mValue = value;
    for (auto* listener : mUIListeners) {
        listener->onParameterUpdated(value);
    }
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