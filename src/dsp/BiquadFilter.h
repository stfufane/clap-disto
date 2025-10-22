#pragma once

#include "utils/Utils.h"
#include <array>

namespace stfefane::dsp {

// A fresh, stable biquad filter implementation based on
// the "Audio EQ Cookbook" by Robert Bristow-Johnson (RBJ).
//
// Implementation notes:
// - Coefficients are normalized so that a0 == 1.
// - Processing uses Transposed Direct Form II for better numerical stability.
// - Denormal protection is applied to the state to avoid CPU spikes.
// - Supports common types including shelves and allpass.
class BiquadFilter {
public:
    enum class Type {
        None,
        LowPass,
        HighPass,
        BandPass,
        Notch,
        Peak,
        AllPass,
        LowShelf,
        HighShelf
    };

    BiquadFilter() = default;

    explicit BiquadFilter(Type type, double freq, double q = 0.707, double gainDb = 0.0)
        : mType(type), mFreq(freq), mQ(q), mGainDb(gainDb) {
        updateCoefficients();
    }

    void setup(Type type, double freq, double q = 0.707, double gainDb = 0.0) {
        mType = type;
        mFreq = freq;
        mQ = q;
        mGainDb = gainDb;
        reset();
        updateCoefficients();
    }

    void setSampleRate(double sampleRate) {
        mSampleRate = sampleRate;
        updateCoefficients();
    }

    void setType(Type type) {
        mType = type;
        updateCoefficients();
    }

    void setFreq(double freq) {
        mFreq = freq;
        updateCoefficients();
    }

    void setQ(double q) {
        mQ = q;
        updateCoefficients();
    }

    void setGainDb(double gainDb) {
        mGainDb = gainDb;
        updateCoefficients();
    }

    [[nodiscard]] double getSampleRate() const { return mSampleRate; }
    [[nodiscard]] Type getType() const { return mType; }
    [[nodiscard]] double getFreq() const { return mFreq; }
    [[nodiscard]] double getQ() const { return mQ; }
    [[nodiscard]] double getGainDb() const { return mGainDb; }

    void reset() {
        mZ1 = 0.0;
        mZ2 = 0.0;
    }

    // Process a single sample
    inline double process(double x) {
        // TDF2: y = b0*x + z1; z1 = b1*x - a1*y + z2; z2 = b2*x - a2*y
        const double y = mB0 * x + mZ1;
        const double newZ1 = mB1 * x - mA1 * y + mZ2;
        const double newZ2 = mB2 * x - mA2 * y;

        // Denormal protection
        mZ1 = denormProtect(newZ1);
        mZ2 = denormProtect(newZ2);
        return y;
    }

    // Process a buffer in-place (double)
    void processBuffer(double* samples, std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            samples[i] = process(samples[i]);
        }
    }

    // Process a buffer in-place (float)
    void processBuffer(float* samples, std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            samples[i] = static_cast<float>(process(samples[i]));
        }
    }

    // Returns current coefficients [b0, b1, b2, a1, a2] where a0 == 1
    [[nodiscard]] std::array<double, 5> getCoefficients() const { return {mB0, mB1, mB2, mA1, mA2}; }

    // Recompute coefficients using RBJ cookbook (a0 normalized to 1)
    void updateCoefficients() {
        if (mType == Type::None || mSampleRate <= 0.0) {
            // Bypass
            mB0 = 1.0;
            mB1 = 0.0;
            mB2 = 0.0;
            mA1 = 0.0;
            mA2 = 0.0;
            return;
        }

        const double nyquist = 0.5 * mSampleRate;
        double f = mFreq;
        if (f <= 0.0) {
            f = 1.0; // prevent zero/negative
        }
        if (f > nyquist * 0.99) {
            f = nyquist * 0.99; // safety clamp under Nyquist
        }

        const double w0 = 2.0 * utils::kPI * (f / mSampleRate);
        const double cw = std::cos(w0);
        const double sw = std::sin(w0);
        const double A = std::pow(10.0, mGainDb / 40.0); // for shelving/peak

        // For Q <= 0 treat as minimum Q
        const double Q = (mQ > 1e-6) ? mQ : 1e-6;
        const double alpha = sw / (2.0 * Q);

        double b0{}, b1{}, b2{}, a0{}, a1{}, a2{};

        switch (mType) {
        case Type::LowPass:
            b0 = (1 - cw) * 0.5;
            b1 = 1 - cw;
            b2 = (1 - cw) * 0.5;
            a0 = 1 + alpha;
            a1 = -2 * cw;
            a2 = 1 - alpha;
            break;
        case Type::HighPass:
            b0 = (1 + cw) * 0.5;
            b1 = -(1 + cw);
            b2 = (1 + cw) * 0.5;
            a0 = 1 + alpha;
            a1 = -2 * cw;
            a2 = 1 - alpha;
            break;
        case Type::BandPass:
            b0 = sw * 0.5; // constant skirt gain, peak gain = Q
            b1 = 0.0;
            b2 = -sw * 0.5;
            a0 = 1 + alpha;
            a1 = -2 * cw;
            a2 = 1 - alpha;
            break;
        case Type::Notch:
            b0 = 1;
            b1 = -2 * cw;
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2 * cw;
            a2 = 1 - alpha;
            break;
        case Type::AllPass:
            b0 = 1 - alpha;
            b1 = -2 * cw;
            b2 = 1 + alpha;
            a0 = 1 + alpha;
            a1 = -2 * cw;
            a2 = 1 - alpha;
            break;
        case Type::Peak: {
            const double alphaA = alpha * A;
            const double alphaDivA = alpha / A;
            b0 = 1 + alphaA;
            b1 = -2 * cw;
            b2 = 1 - alphaA;
            a0 = 1 + alphaDivA;
            a1 = -2 * cw;
            a2 = 1 - alphaDivA;
            break;
        }
        case Type::LowShelf: {
            const double sqrtA = std::sqrt(A);
            const double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1) - (A - 1) * cw + twoSqrtAAlpha);
            b1 = 2 * A * ((A - 1) - (A + 1) * cw);
            b2 = A * ((A + 1) - (A - 1) * cw - twoSqrtAAlpha);
            a0 = (A + 1) + (A - 1) * cw + twoSqrtAAlpha;
            a1 = -2 * ((A - 1) + (A + 1) * cw);
            a2 = (A + 1) + (A - 1) * cw - twoSqrtAAlpha;
            break;
        }
        case Type::HighShelf: {
            const double sqrtA = std::sqrt(A);
            const double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1) + (A - 1) * cw + twoSqrtAAlpha);
            b1 = -2 * A * ((A - 1) + (A + 1) * cw);
            b2 = A * ((A + 1) + (A - 1) * cw - twoSqrtAAlpha);
            a0 = (A + 1) - (A - 1) * cw + twoSqrtAAlpha;
            a1 = 2 * ((A - 1) - (A + 1) * cw);
            a2 = (A + 1) - (A - 1) * cw - twoSqrtAAlpha;
            break;
        }
        case Type::None:
            // handled earlier
            b0 = 1;
            b1 = 0;
            b2 = 0;
            a0 = 1;
            a1 = 0;
            a2 = 0;
            break;
        }

        // Normalize so a0 == 1
        const double invA0 = (a0 != 0.0) ? (1.0 / a0) : 1.0;
        mB0 = b0 * invA0;
        mB1 = b1 * invA0;
        mB2 = b2 * invA0;
        mA1 = a1 * invA0;
        mA2 = a2 * invA0;
    }

private:
    static inline double denormProtect(double v) {
        // Add very tiny DC offset removal behavior: flush to zero
        return (std::fabs(v) < 1e-30) ? 0.0 : v;
    }

    // Coefficients, a0 is implicitly 1
    double mB0{1.0}, mB1{0.0}, mB2{0.0}, mA1{0.0}, mA2{0.0};

    // States for TDF2
    double mZ1{0.0}, mZ2{0.0};

    // Parameters
    Type mType{Type::None};
    double mSampleRate{44100.0};
    double mFreq{1000.0};
    double mQ{0.707};
    double mGainDb{0.0};
};

} // namespace stfefane::dsp
