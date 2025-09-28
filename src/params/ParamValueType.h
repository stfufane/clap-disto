#pragma once

#include <charconv>
#include <iomanip>
#include <numeric>
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
    uint32_t mFlags = CLAP_PARAM_IS_AUTOMATABLE;

    [[nodiscard]] virtual std::string toText(double value) const {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << value << mUnit;
        return os.str();
    }

    [[nodiscard]] virtual double toValue(const std::string& text) const {
        return utils::stringToDouble(text);
    }

    [[nodiscard]] virtual double denormalizedValue(double value) const {
        return value;
    }
};

struct DecibelValueType final : public ParamValueType {
    explicit DecibelValueType(double defaultVal, double min_db, double max_db)
        : ParamValueType(0.0, 1.0, defaultVal, " dB"), mMinDb(min_db), mMaxDb(max_db) {}

    [[nodiscard]] std::string toText(double value) const override {
        return ParamValueType::toText(denormalizedValue(value));
    }

    [[nodiscard]] double toValue(const std::string& text) const override {
        // FIXME: value not good
        return utils::dbToLinear(utils::stringToDouble(text)) / (mMaxDb - mMinDb);
    }

    [[nodiscard]] double denormalizedValue(double value) const override {
        return mMinDb + value * (mMaxDb - mMinDb);
    }

    // The parameter is normalized, but we want to be able to map it to different dB values.
    double mMinDb = 0.;
    double mMaxDb = 24.;
};

struct ParamPercentValueType final : public ParamValueType {
    explicit ParamPercentValueType(double defaultVal)
        : ParamValueType(0., 1., defaultVal, " %") {}

    [[nodiscard]] std::string toText(double value) const override {
        return ParamValueType::toText(value * 100.);
    };

    [[nodiscard]] double toValue(const std::string& text) const override {
        return ParamValueType::toValue(text) / 100.;
    }
};

struct SteppedValueType : public ParamValueType {
    explicit SteppedValueType(std::vector<std::string>&& values, double defaultVal)
        : ParamValueType(0., static_cast<double>(values.size() - 1), defaultVal, std::string()), mValues(std::move(values)) {
        mFlags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED;
    }

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