#include "CubicDrive.h"
#include <cmath>

namespace stfefane::dsp {

void CubicDrive::setDrive(float drive) {
    mDrive = drive;
}

void CubicDrive::process(choc::buffer::ChannelArrayView<float>& buffer) const {
    auto num_channels = buffer.getNumChannels();
    auto num_frames = buffer.getNumFrames();

    for (uint32_t channel = 0; channel < num_channels; ++channel) {
        for (uint32_t frame = 0; frame < num_frames; ++frame) {
            auto& io_sample = buffer.getSample(channel, frame);
            // Apply drive
            float driven_sample = io_sample * (1.f + mDrive * 10.f);
            // Cubic distortion with hard clipping
            float distorted_sample = driven_sample - 0.4f * driven_sample * driven_sample * driven_sample;
            io_sample = std::max(-1.f, std::min(1.f, distorted_sample));
        }
    }
}

}