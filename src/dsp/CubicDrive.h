#pragma once

#include <choc/audio/choc_SampleBuffers.h>

namespace stfefane::dsp {

class CubicDrive {
public:
    CubicDrive() = default;

    void process(choc::buffer::ChannelArrayView<float>& buffer) const;
    void setDrive(float drive);

private:
    float drive = .5f;
};

}