#pragma once

namespace stfefane::params {

enum class MappingType {
    Linear,
    Logarithmic,
    BipolarSCurve
};

class ValueMapping {
public:
    ValueMapping() = default;
    explicit ValueMapping(MappingType type, double min, double max);

    [[nodiscard]] double normalize(double value) const;
    [[nodiscard]] double denormalize(double value) const;

    [[nodiscard]] double getMin() const noexcept { return mMin; }
    [[nodiscard]] double getMax() const noexcept { return mMax; }

private:
    double mMin { 0.0 };
    double mMax { 1.0 };
    double mRange { 1.0 };
    MappingType mType { MappingType::Linear };

};

}
