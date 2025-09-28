#pragma once

#include "BiquadFilter.h"
#include "OverSampler.h"

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

    void setSampleRate(double samplerate);
    void reset();

    double process(double input);

    void setDrive(double drive);
    void setType(DistortionType type) { mType = type; }
    void setInputGain(double gainDb) { mInputGain = utils::dbToLinear(gainDb); }
    void setOutputGain(double gainDb) { mOutputGain = utils::dbToLinear(gainDb); }
    void setPreFilterOn(bool on) { mPreFilterOn = on; }
    void setPreFilterFreq(double freq) { mPreFilter.setFreq(freq); }
    void setPostFilterOn(bool on) { mPostFilterOn = on; }
    void setPostFilterFreq(double freq) { mPostFilter.setFreq(freq); }
    void setBias(double bias) { mBias = bias; }
    void setAsymmetry(double asymmetry) { mAsymmetry = asymmetry; }
    void setMix(double mix) { mMix = mix; }

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

    BiquadFilter mPreFilter = { 10000., .707, FilterType::LOW_PASS };
    BiquadFilter mPostFilter = { 80., .707, FilterType::HIGH_PASS };
    DCBlocker mDCBlocker;
    Oversampler mOversampler;

    // State for bitcrusher sample-rate reduction
    mutable int mBitcrushPhase = 0;
    mutable double mBitcrushHold = 0.0;

    double mInputGain = 1.;
    double mOutputGain = 1.;
    double mDrive = 6.;
    double mAsymmetry = 0.; // For asymmetric distortion
    double mBias = 0.;
    double mMix = 1.; // Wet/dry mix
    bool mPreFilterOn = true;
    bool mPostFilterOn = true;
};

}