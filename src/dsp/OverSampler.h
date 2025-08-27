#pragma once

#include <array>
#include "../helpers/Utils.h"
#include "BiquadFilter.h"

namespace stfefane::dsp {

class Oversampler {
public:
    static constexpr int FACTOR = 4;

    void setupAntiAliasing(double sampleRate);

    std::array<double, FACTOR>& upsample(double input);
    [[nodiscard]] double downsample() const;

private:
    // Simple linear interpolation upsampler
    std::array<double, FACTOR> buffer = {};
    BiquadFilter mAntiAliasingFilter;
};

}