#include "CubicDrive.h"
#include <cmath>

namespace stfefane::dsp {

void CubicDrive::setDrive(float newDrive) {
    drive = newDrive;
}

void CubicDrive::process(choc::buffer::ChannelArrayView<float>& buffer) const {
    auto numChannels = buffer.getNumChannels();
    auto numFrames = buffer.getNumFrames();

    for (uint32_t channel = 0; channel < numChannels; ++channel) {
        for (uint32_t frame = 0; frame < numFrames; ++frame) {
            auto& inputSample = buffer.getSample(channel, frame);
            // Apply drive
            float drivenSample = inputSample * (1.f + drive * 10.f);
            // Cubic distortion with hard clipping
            float distortedSample = drivenSample - 0.4f * drivenSample * drivenSample * drivenSample;
            inputSample = std::max(-1.f, std::min(1.f, distortedSample));
        }
    }
}

}