#pragma once

#include "clap/ext/params.h"

namespace stfefane::params {

struct ParamValueType {
    ParamValueType(double min, double max, double defaultVal, std::string unit)
        : mMin(min), mMax(max), mDefault(defaultVal), mUnit(std::move(unit)) {}
    virtual ~ParamValueType() = default;

    double mMin = std::numeric_limits<double>::lowest();
    double mMax = std::numeric_limits<double>::max();
    double mDefault = 0.;
    std::string mUnit;

    [[nodiscard]] virtual std::string toText(double value) const {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << value << mUnit;
        return os.str();
    }

    [[nodiscard]] virtual double toValue(const std::string& text) const {
        double result = 0.;
        std::from_chars(text.c_str(), text.c_str() + text.size(), result);
        return result;
    }
};

struct ParamPercentValueType final : ParamValueType {
    explicit ParamPercentValueType(double defaultVal)
        : ParamValueType(0., 1., defaultVal, " %") {}

    [[nodiscard]] std::string toText(double value) const override {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << value * 100. << mUnit;
        return os.str();
    };
};

class Parameter {
public:
    explicit Parameter(const clap_param_info& info, std::unique_ptr<ParamValueType> value_type, size_t index) :
         mIndex(index), mInfo(info), mValueType(std::move(value_type)) {}
    Parameter() = delete;
    Parameter(const Parameter &) = delete;
    Parameter(Parameter &&) = delete;

    [[nodiscard]] double getValue() const noexcept { return mValue; }
    void setValue(const double value) { mValue = value; }

    [[nodiscard]] const clap_param_info& getInfo() const noexcept { return mInfo; }
    [[nodiscard]] const std::string& getUnit() const noexcept { return mValueType->mUnit; }
    [[nodiscard]] const ParamValueType& getValueType() const noexcept { return *mValueType; }

private:
    size_t mIndex = -1;
    clap_param_info mInfo;

    std::unique_ptr<ParamValueType> mValueType;
    double mValue = 0.;
};
}