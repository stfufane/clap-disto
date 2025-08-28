#include "OverSampler.h"

namespace stfefane::dsp {

void Oversampler::setupAntiAliasing(double sampleRate) {
    // Low-pass filter at Nyquist/2 to prevent aliasing
    mAntiAliasingFilter.setSampleRate(sampleRate);
    mAntiAliasingFilter.setup(FilterType::LOW_PASS, sampleRate * .25, .707);
}

std::array<double, Oversampler::FACTOR>& Oversampler::upsample(double input) {
    buffer.fill(0.);
    buffer[0] = input;

    // Apply anti-aliasing filter to each sample
    for (int i = 0; i < FACTOR; i++) {
        buffer[i] = mAntiAliasingFilter.process(buffer[i]);
    }

    return buffer;
}

double Oversampler::downsample() const {
    // Simple decimation - take every FACTOR-th sample
    // In practice, you'd want better anti-aliasing here too
    return buffer[0];
}

} // namespace stfefane::dsp
