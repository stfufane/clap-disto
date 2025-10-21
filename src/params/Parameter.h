#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <clap/ext/params.h>

#include "ParamValueType.h"

namespace stfefane::params {

class IParameterListener;

class Parameter {
public:
    explicit Parameter(clap_id id, const std::string& name, std::unique_ptr<ParamValueType> value_type, size_t index);
    Parameter() = delete;
    Parameter(const Parameter &) = delete;
    Parameter(Parameter &&) = delete;

    [[nodiscard]] double getValue() const noexcept { return mValue.load(std::memory_order_relaxed); }
    void setValue(double value);

    void notifyAllParamListeners() const noexcept;

    [[nodiscard]] const clap_param_info& getInfo() const noexcept { return mInfo; }
    [[nodiscard]] const ParamValueType& getValueType() const noexcept { return *mValueType; }

    [[nodiscard]] bool isStepped() const noexcept { return mInfo.flags & CLAP_PARAM_IS_STEPPED; }
    [[nodiscard]] size_t nbSteps() const noexcept;

    void addListener(IParameterListener* listener);
    void removeListener(IParameterListener* listener);

private:
    size_t mIndex = -1;
    clap_param_info mInfo;
    std::atomic<double> mValue = 0.;

    std::unique_ptr<ParamValueType> mValueType;

    std::vector<IParameterListener*> mListeners;
};

}