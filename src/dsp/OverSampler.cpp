#include "OverSampler.h"

namespace stfefane::dsp {

void Oversampler::setupAntiAliasing(double sampleRate) {
    // Both filters operate in the oversampled domain (fs * FACTOR)
    const double fsOS = sampleRate * static_cast<double>(FACTOR);

    mAntiImagingFilter.setSampleRate(fsOS);
    mAntiAliasFilter.setSampleRate(fsOS);

    // Keep passband up to ~0.45 * original Nyquist to leave some transition band
    const double cutoff = sampleRate * 0.45; // equals 0.45 * (fs/2)

    mAntiImagingFilter.setup(BiquadFilter::Type::LowPass, cutoff, 0.707);
    mAntiAliasFilter.setup(BiquadFilter::Type::LowPass, cutoff, 0.707);

    // Reset interpolation state
    mPrevInput = 0.0;
    mPrevSlope = 0.0;
}

std::array<double, Oversampler::FACTOR>& Oversampler::upsample(double input) {
    // Cubic Hermite interpolation between previous and current input samples
    const double x0 = mPrevInput;
    const double x1 = input;

    // Estimate slopes using finite differences
    const double m0 = mPrevSlope;         // slope at previous sample
    const double m1 = (x1 - x0);          // slope at current sample (simple estimate)

    // Generate 4 samples at t = 0.25, 0.5, 0.75, 1.0 of the segment [x0 -> x1]
    auto hermite = [&](double t) {
        const double t2 = t * t;
        const double t3 = t2 * t;
        const double h00 = 2.0 * t3 - 3.0 * t2 + 1.0;
        const double h10 = t3 - 2.0 * t2 + t;
        const double h01 = -2.0 * t3 + 3.0 * t2;
        const double h11 = t3 - t2;
        return h00 * x0 + h10 * m0 + h01 * x1 + h11 * m1;
    };

    // Fill buffer with interpolated values then run anti-imaging filter through them sequentially
    buffer[0] = hermite(0.25);
    buffer[1] = hermite(0.5);
    buffer[2] = hermite(0.75);
    buffer[3] = hermite(1.0);

    for (int i = 0; i < FACTOR; ++i) {
        buffer[i] = mAntiImagingFilter.process(buffer[i]);
    }

    // Update state for next call
    mPrevSlope = m1;
    mPrevInput = x1;

    return buffer;
}

double Oversampler::downsample() {
    // Run anti-aliasing filter over the 4 oversampled samples and return the last (aligned) one
    double y = 0.0;
    for (int i = 0; i < FACTOR; ++i) {
        y = mAntiAliasFilter.process(buffer[i]);
    }
    return y;
}

} // namespace stfefane::dsp
