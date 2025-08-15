#pragma once

#include "clap/ext/params.h"

namespace stfefane::params {

class Parameter {
public:
    explicit Parameter(const clap_param_info& info, size_t index) :
         mIndex(index), mInfo(info) {}
    Parameter() = delete;
    Parameter(const Parameter &) = delete;
    Parameter(Parameter &&) = delete;

    double getValue() const noexcept { return mValue; }
    void setValue(double value) { mValue = value; }
    const clap_param_info& getInfo() const noexcept { return mInfo; }

private:
    size_t mIndex = -1;
    clap_param_info mInfo;

    double mValue = 0.;
};
}