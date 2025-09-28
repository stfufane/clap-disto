#pragma once

#include <array>
#include "../helpers/Utils.h"
#include "BiquadFilter.h"

namespace stfefane::dsp {

class Oversampler {
public:
    static constexpr int FACTOR = 4;

    // Initialize anti-imaging (upsampling) and anti-aliasing (downsampling) filters
    void setupAntiAliasing(double sampleRate);

    // Generate 4x-oversampled samples for a single input sample using cubic Hermite interpolation
    std::array<double, FACTOR>& upsample(double input);
    // Feed the 4 processed oversampled samples back to base rate with anti-aliasing
    [[nodiscard]] double downsample();

private:
    std::array<double, FACTOR> buffer = {};

    // State for interpolation between previous and current input samples
    double mPrevInput = 0.0;
    double mPrevSlope = 0.0;

    // Anti-imaging filter applied in the upsampled domain (fs * FACTOR)
    BiquadFilter mAntiImagingFilter;
    // Anti-aliasing filter applied before decimation (also in fs * FACTOR)
    BiquadFilter mAntiAliasFilter;
};

}