#pragma once

#include <clap/id.h>
#include "../params.h"

namespace stfefane::helpers {

class IParamControl {
public:
    explicit IParamControl(clap_id param_id) {
        clap_param_info param_info;
        params::Parameters::getParamInfo(mParamId, &param_info);
        mCurrentValue = param_info.default_value;
        mMinValue = param_info.min_value;
        mMaxValue = param_info.max_value;
    };
    IParamControl() = delete;

    virtual ~IParamControl() = default;

protected:
    clap_id mParamId = UINT32_MAX;

    double mMinValue = 0.;
    double mMaxValue = 0.;
    double mCurrentValue = 0.;
};

}
