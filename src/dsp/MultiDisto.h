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

class MultiDisto {
public:
    MultiDisto() = default;

    void setSampleRate(double samplerate);
    void reset();

    double process(double input);

    void setDrive(double drive) { mDrive = drive; }
    void setOutputGain(double gain) { mOutputGain = gain; }

private:
    // DC blocking filter
    struct DCBlocker {
        double x1 = 0.0, y1 = 0.0;
        double R = 0.995; // pole location (close to 1 for DC blocking)

        double process(double input) {
            double output = input - x1 + R * y1;
            x1 = input;
            y1 = output;
            return output;
        }
    };

    void updateFilters();

    double applyDistortion(double input) const;

    // Distortion algorithms
    double cubicSaturation(double input) const;
    double tubeSaturation(double input) const;
    double asymmetricClip(double input) const;
    double foldbackDistortion(double input) const;
    double bitcrushDistortion(double input) const;
    double waveShaperDistortion(double input) const;
    double tubeScreamerDistortion(double input) const;
    double fuzzFaceDistortion(double input) const;

    double mSampleRate = 44100.0;
    DistortionType mType = DistortionType::TUBE_SCREAMER;

    BiquadFilter mPreFilter = { 10000., .707, FilterType::LOW_PASS };
    BiquadFilter mPostFilter = { 80., .707, FilterType::HIGH_PASS };
    DCBlocker mDCBlocker;
    Oversampler mOversampler;

    double mInputGain = 1.;
    double mOutputGain = 1.;
    double mDrive = .1;
    double mMix = 1.; // Wet/dry mix
    double mAsymmetry = 0.; // For asymmetric distortion
    double mBias = 0.;
};

}