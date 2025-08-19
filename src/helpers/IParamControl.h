#pragma once

#include "../params/Parameters.h"
#include <clap/id.h>

namespace stfefane::helpers {

class IParamControl {
public:
    explicit IParamControl(const clap_param_info& param_info): mParamId(param_info.id), mParamInfo(param_info) {
        mCurrentValue = param_info.default_value;
    };
    IParamControl() = delete;

    virtual ~IParamControl() = default;

protected:
    [[nodiscard]] double getMinValue() const noexcept { return mParamInfo.min_value; }
    [[nodiscard]] double getMaxValue() const noexcept { return mParamInfo.max_value; }

    clap_id mParamId = UINT32_MAX;
    const clap_param_info& mParamInfo;

    double mCurrentValue;
};

}
