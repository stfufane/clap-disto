#pragma once

#include <charconv>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include "../helpers/Utils.h"

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
        return utils::stringToDouble(text);
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

    [[nodiscard]] double toValue(const std::string& text) const override {
        return ParamValueType::toValue(text) / 100.;
    }
};

struct SteppedValueType final : ParamValueType {
    explicit SteppedValueType(std::vector<std::string>&& values, double defaultVal)
        : ParamValueType(0., static_cast<double>(values.size() - 1), defaultVal, std::string()), mValues(std::move(values)) {}

    [[nodiscard]] std::string toText(double value) const override {
        const auto index = static_cast<size_t>(value);
        if (index >= mValues.size()) {
            return "INVALID INDEX";
        }
        return mValues[index];
    }

    [[nodiscard]] double toValue(const std::string& text) const override {
        if (const auto it = std::ranges::find(mValues, text); it != mValues.end()) {
            const auto index = std::distance(mValues.begin(), it);
            return static_cast<double>(index);
        }
        return 0.;
    }

    const std::vector<std::string> mValues;
};

}