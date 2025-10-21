#include "ValueMapping.h"

#include <algorithm>
#include <cmath>

namespace stfefane::params {

ValueMapping::ValueMapping(MappingType type, double min, double max) : mType(type), mMin(min), mMax(max), mRange(mMax - mMin) {
}
double ValueMapping::normalize(double value) const {
    if (mRange <= 0.0) {
        return 0.0;
    }
    switch (mType) {
    case MappingType::Linear:
        return std::clamp((value - mMin) / mRange, 0.0, 1.0);
    case MappingType::Logarithmic: {
        if (mMin <= 0.0 || mMax <= 0.0 || value <= 0.0) {
            return std::clamp((value - mMin) / mRange, 0.0, 1.0);
        }
        const double log_min = std::log(mMin);
        const double log_max = std::log(mMax);
        const double log_v = std::log(value);
        const double t = (log_v - log_min) / (log_max - log_min);
        return std::clamp(t, 0.0, 1.0);
    }
    case MappingType::BipolarSCurve: {
        // Inverse of the S-curve denormalize above.
        const double mid = 0.5 * (mMin + mMax);
        const double half = 0.5 * (mMax - mMin);
        if (half <= 0.0) {
            return 0.0;
        }
        // Map value to [-1,1]
        double s_shaped = std::clamp((value - mid) / half, -1.0, 1.0);
        // Undo odd-power shaping
        constexpr double p = 3.0;
        const double s = std::copysign(std::pow(std::abs(s_shaped), 1.0 / p), s_shaped);
        // Map back to [0,1]
        return std::clamp(0.5 * (s + 1.0), 0.0, 1.0);
    }
    }
    return std::clamp((value - mMin) / mRange, 0.0, 1.0);
}

double ValueMapping::denormalize(double value) const {
    value = std::clamp(value, 0.0, 1.0);
    switch (mType) {
    case MappingType::Linear:
        return mMin + value * (mMax - mMin);
    case MappingType::Logarithmic: {
        // Guard against non-positive bounds; fallback to linear in that case
        if (mMin <= 0.0 || mMax <= 0.0) {
            return mMin + value * (mMax - mMin);
        }
        const double log_min = std::log(mMin);
        const double log_max = std::log(mMax);
        const double log_val = log_min + value * (log_max - log_min);
        return std::exp(log_val);
    }
    case MappingType::BipolarSCurve: {
        // Higher resolution around 0 for bipolar ranges using an odd-power S-curve.
        // Map t in [0,1] to s in [-1,1]
        const double s = 2.0 * value - 1.0;
        // Apply odd-power for symmetry; p > 1 compresses around 0 (finer control)
        constexpr double p = 3.0;
        const double s_shaped = std::copysign(std::pow(std::abs(s), p), s);
        // Map back to actual range via midpoint/half-range so that 0 sits at the center
        const double mid = 0.5 * (mMin + mMax);
        const double half = 0.5 * (mMax - mMin);
        return mid + s_shaped * half;
    }
    }
    // Fallback
    return mMin + value * (mMax - mMin);
}

} // namespace stfefane::utils
