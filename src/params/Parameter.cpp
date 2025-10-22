#include "Parameter.h"

#include <algorithm>
#include <vector>

#include "IParameterListener.h"
#include "utils/Logger.h"

namespace stfefane::params {

Parameter::Parameter(clap_id id, const std::string& name, std::unique_ptr<ParamValueType> value_type, size_t index)
    : mIndex(index)
    , mInfo {
        .id = id,
        .flags = value_type->mFlags,
        .cookie = this,
        .min_value = 0.,
        .max_value = 1.,
        .default_value = value_type->mDefault,
    }
    , mValue(value_type->mDefault)
    , mValueType(std::move(value_type))
{
    snprintf(mInfo.name, sizeof(mInfo.name), "%s", name.c_str());
    LOG_INFO("param", "Parameter {} init with default_value {}", name, mInfo.default_value);
    if (isStepped()) {
        mInfo.max_value = static_cast<double>(nbSteps() - 1);
    }
}

void Parameter::setValue(double value) {
    if (isStepped()) {
        value = std::round(value);
    }
    mValue.store(value, std::memory_order_relaxed);
    notifyAllListeners();
}

void Parameter::notifyAllListeners() const noexcept {
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