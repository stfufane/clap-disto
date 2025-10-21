#pragma once

#include <algorithm>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include "helpers/Utils.h"
#include "params/ValueMapping.h"

namespace stfefane::params {

struct ParamValueType {
    ParamValueType(double min, double max, double defaultVal, std::string unit = std::string(), MappingType mapping = MappingType::Linear)
        : mMapping(mapping, min, max), mDefault(mMapping.normalize(defaultVal)), mUnit(std::move(unit)) {}
    virtual ~ParamValueType() = default;

    ValueMapping mMapping;
    double mDefault = 0.;
    std::string mUnit;
    uint32_t mFlags = CLAP_PARAM_IS_AUTOMATABLE;

    [[nodiscard]] virtual std::string toText(double value) const {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << denormalizedValue(value) << mUnit;
        return os.str();
    }

    [[nodiscard]] virtual double toValue(const std::string& text) const {
        return normalizedValue(utils::stringToDouble(text));
    }

    [[nodiscard]] double denormalizedValue(double value) const {
        return mMapping.denormalize(value);
    }

    [[nodiscard]] double normalizedValue(double value) const {
        return mMapping.normalize(value);
    }
};

struct SteppedValueType : public ParamValueType {
    explicit SteppedValueType(std::vector<std::string>&& values, double defaultVal)
        : ParamValueType(0., static_cast<double>(values.size() - 1), defaultVal)
        , mValues(std::move(values)) {
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

struct BooleanValueType final : SteppedValueType {
    explicit BooleanValueType(bool defaultVal)
        : SteppedValueType({ "Off", "On" }, defaultVal) {}
};

}