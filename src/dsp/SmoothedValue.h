#pragma once

#include <algorithm>

namespace stfefane::dsp {

/**
 * Tiny wrapper around a double value to avoid jumps when updating it
 */
struct SmoothedValue {
    double mSampleRate = 44100.0;
    double mCoeff = 0.0;
    // Set to mutable so process can be used inside const methods.
    // The real important value is the target value so it's not really a problem.
    mutable double mProcessedValue = 0.0;
    double mTargetValue = 0.0;

    void setup(double sr, double ms) {
        mSampleRate = sr;
        const double tau = std::max(1e-6, ms * 1e-3);
        mCoeff = 1.0 - std::exp(-1.0 / (tau * mSampleRate));
    }

    double process() const {
        mProcessedValue += mCoeff * (mTargetValue - mProcessedValue);
        return mProcessedValue;
    }

    // Allows to use arithmetic operations with a regular double.
    // Voluntarily not made explicit to allow simple automatic conversion.
    operator double() const {
        return process();
    }

    // Set the target with a simple value assignment
    SmoothedValue& operator=(double v) {
        mTargetValue = v;
        return *this;
    }

    // Simple operator to compare with other double values
    bool operator==(double v) const {
        return mTargetValue == v;
    }

    bool operator==(const SmoothedValue& v) const {
        return mTargetValue == v.mTargetValue;
    }
};

}