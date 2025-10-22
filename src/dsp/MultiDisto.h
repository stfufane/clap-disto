#pragma once

#include <vector>
#include "BiquadFilter.h"
#include "OverSampler.h"

namespace stfefane {
namespace params {
class ParameterAttachment;
}
class Disstortion;
}

namespace stfefane::dsp {

enum class DistortionType {
    CUBIC_SATURATION,
    TUBE_SATURATION,
    ASYMMETRIC_CLIP,
    FOLDBACK,
    BITCRUSHER,
    WAVE_SHAPER,
    TUBE_SCREAMER,
    FUZZ_FACE
};

static constexpr double kMaxDriveDb = 36.;

class MultiDisto {
public:
    MultiDisto() = default;

    void initParameterAttachments(const Disstortion& d);

    void setSampleRate(double samplerate);
    void reset();

    double process(double input);

private:
    // DC blocking filter
    struct DCBlocker {
        double x1 = 0.0, y1 = 0.0;
        double R = 0.995; // pole location (close to 1 for DC blocking)

        double process(double input) {
            double output = input - x1 + R * y1;
            // Denormal protection
            if (std::abs(output) < 1e-20) output = 0.0;
            x1 = input;
            y1 = output;
            return output;
        }
    };

    [[nodiscard]] double applyDistortion(double input) const;

    // Distortion algorithms
    [[nodiscard]] double cubicSaturation(double input) const;
    [[nodiscard]] double tubeSaturation(double input) const;
    [[nodiscard]] double asymmetricClip(double input) const;
    [[nodiscard]] double foldbackDistortion(double input) const;
    [[nodiscard]] double bitcrushDistortion(double input) const;
    [[nodiscard]] double waveShaperDistortion(double input) const;
    [[nodiscard]] double tubeScreamerDistortion(double input) const;
    [[nodiscard]] double fuzzFaceDistortion(double input) const;

    double mSampleRate = 44100.0;
    DistortionType mType = DistortionType::TUBE_SCREAMER;

    BiquadFilter mPreFilter { BiquadFilter::Type::LowPass, 10000. };
    BiquadFilter mPostFilter { BiquadFilter::Type::HighPass, 80. };
    DCBlocker mDCBlocker;
    Oversampler mOversampler;

    // State for bitcrusher sample-rate reduction
    mutable int mBitcrushPhase = 0;
    mutable double mBitcrushHold = 0.0;

    std::vector<params::ParameterAttachment> mParameterAttachments;

    double mInputGain = 0.;
    double mOutputGain = 0.;
    double mDrive = 0.;
    double mAsymmetry = 0.; // For asymmetric distortion
    double mBias = 0.;
    double mMix = 1.; // Wet/dry mix
    bool mPreFilterOn = true;
    bool mPostFilterOn = true;
};

}