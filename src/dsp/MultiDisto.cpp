#include "MultiDisto.h"

#include <algorithm>
#include <cmath>

namespace stfefane::dsp {


void MultiDisto::setSampleRate(double samplerate) {
    mSampleRate = samplerate;
    mOversampler.setupAntiAliasing(samplerate);
    mPreFilter.setSampleRate(samplerate);
    mPostFilter.setSampleRate(samplerate);
}

void MultiDisto::reset() {
    mPreFilter.reset();
    mPostFilter.reset();
    mDCBlocker = DCBlocker{};
}

double MultiDisto::process(double input) {
    // Store dry signal for mix
    double drySignal = input;

    // Apply input gain
    double signal = input * mInputGain;

    // Pre-filter
    if (mPreFilterOn && mPreFilter.getType() != FilterType::NONE) {
        signal = mPreFilter.process(signal);
    }

    // Oversampling for anti-aliasing (optional, can be toggled)
    if (mDrive > 9.0) {
        auto& upsampled = mOversampler.upsample(signal);
        for (auto& sample : upsampled) {
            sample = applyDistortion(sample);
        }
        signal = mOversampler.downsample();
    } else {
        signal = applyDistortion(signal);
    }

    // DC blocking
    signal = mDCBlocker.process(signal);

    // Post-filter
    if (mPostFilterOn && mPostFilter.getType() != FilterType::NONE) {
        signal = mPostFilter.process(signal);
    }

    // Apply output gain
    signal *= mOutputGain;

    // Wet/dry mix
    signal = mMix * signal + (1.0 - mMix) * drySignal;

    // Final safety limiting
    return std::clamp(signal, -1., 1.);
}

void MultiDisto::setDrive(double drive) {
    // Denormalize the value to a 0-36dB range
    mDrive = utils::dbToLinear(drive * kMaxDriveDb);
}

double MultiDisto::applyDistortion(double input) const {
    // Add bias
    input += mBias;

    switch (mType) {
    case DistortionType::CUBIC_SATURATION:
        return cubicSaturation(input);

    case DistortionType::TUBE_SATURATION:
        return tubeSaturation(input);

    case DistortionType::ASYMMETRIC_CLIP:
        return asymmetricClip(input);

    case DistortionType::FOLDBACK:
        return foldbackDistortion(input);

    case DistortionType::BITCRUSHER:
        return bitcrushDistortion(input);

    case DistortionType::WAVE_SHAPER:
        return waveShaperDistortion(input);

    case DistortionType::TUBE_SCREAMER:
        return tubeScreamerDistortion(input);

    case DistortionType::FUZZ_FACE:
        return fuzzFaceDistortion(input);

    default:
        return input;
    }
}

double MultiDisto::cubicSaturation(double input) const {
    double x = input * mDrive;
    if (std::abs(x) < 2.0 / 3.0) {
        return x * (1.0 + mAsymmetry * x);
    } else {
        double sign = (x > 0.0) ? 1.0 : -1.0;
        return sign * (1.0 - std::pow(2.0 - 3.0 * std::abs(x), 2.0) / 3.0);
    }
}

double MultiDisto::tubeSaturation(double input) const {
    double x = input * mDrive * 0.7;
    // Tube-like saturation curve
    return std::tanh(x * (1.0 + mAsymmetry)) * (1.0 + 0.1 * x * x);
}

double MultiDisto::asymmetricClip(double input) const {
    double x = input * mDrive;
    double posThresh = 0.7 + mAsymmetry * 0.3;
    double negThresh = -0.7 - mAsymmetry * 0.3;

    if (x > posThresh) {
        return posThresh + (x - posThresh) * 0.1;
    } else if (x < negThresh) {
        return negThresh + (x - negThresh) * 0.1;
    }
    return x;
}

double MultiDisto::foldbackDistortion(double input) const {
    double x = input * mDrive;
    double threshold = 1.0;

    while (std::abs(x) > threshold) {
        if (x > threshold) {
            x = 2.0 * threshold - x;
        } else if (x < -threshold) {
            x = -2.0 * threshold - x;
        }
    }
    return x * 0.7; // Scale down to prevent excessive levels
}

double MultiDisto::bitcrushDistortion(double input) const {
    double x = input * mDrive;
    int bits = std::max(1, (int)(16 - mDrive * 12)); // Variable bit depth
    double levels = std::pow(2.0, bits) - 1.0;
    return std::round(x * levels) / levels;
}

double MultiDisto::waveShaperDistortion(double input) const {
    double x = input * mDrive;
    // Sigmoid-based waveshaping
    double k = 2.0 * mDrive;
    return x * (1.0 + k) / (1.0 + k * std::abs(x));
}

double MultiDisto::tubeScreamerDistortion(double input) const {
    // Tube Screamer-inspired soft clipping
    double x = input * mDrive * 2.0;
    double sign = (x >= 0.0) ? 1.0 : -1.0;
    x = std::abs(x);

    if (x < 1.0 / 3.0) {
        return sign * 2.0 * x;
    } else if (x < 2.0 / 3.0) {
        return sign * (3.0 - std::pow(2.0 - 3.0 * x, 2.0)) / 3.0;
    } else {
        return sign;
    }
}

double MultiDisto::fuzzFaceDistortion(double input) const {
    // Fuzz Face-inspired germanium transistor distortion
    double x = input * mDrive * 1.5;
    double sign = (x >= 0.0) ? 1.0 : -1.0;
    x = std::abs(x);

    // Asymmetric germanium-like curve
    double pos_curve = 1.0 - std::exp(-x * (2.0 + mAsymmetry));
    double neg_curve = 1.0 - std::exp(-x * (2.0 - mAsymmetry));

    return sign * (sign > 0 ? pos_curve : neg_curve) * 0.8;
}

} // namespace stfefane::dsp
