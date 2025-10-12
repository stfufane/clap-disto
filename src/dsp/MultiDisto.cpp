#include "MultiDisto.h"

#include <algorithm>
#include <cmath>
#include "../helpers/Utils.h"

namespace stfefane::dsp {

void MultiDisto::activate() {
    mPreFilter.setup(BiquadFilter::Type::LowPass, 10000.);
    mPostFilter.setup(BiquadFilter::Type::HighPass, 80.);
}

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
    if (mPreFilterOn && mPreFilter.getType() != BiquadFilter::Type::None) {
        signal = mPreFilter.process(signal);
    }

    // Determine if non-linear stage can be bypassed (drive ~ 0dB and no bias/asymmetry)
    const bool bypassNonLinear = (std::abs(mDrive - 1.0) < 1e-12) && (mBias == 0.0) && (mAsymmetry == 0.0);

    if (!bypassNonLinear) {
        // Oversampling for anti-aliasing on non-linear types (avoid for bitcrusher)
        if (mType != DistortionType::BITCRUSHER) {
            auto& upsampled = mOversampler.upsample(signal);
            for (auto& sample : upsampled) {
                sample = applyDistortion(sample);
            }
            signal = mOversampler.downsample();
        } else {
            signal = applyDistortion(signal);
        }

        // DC blocking (only needed when using non-linearities / bias)
        signal = mDCBlocker.process(signal);
    }

    // Post-filter
    if (mPostFilterOn && mPostFilter.getType() != BiquadFilter::Type::None) {
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
    // Normalize tanh drive to avoid level jumps: y = tanh(g*x) / tanh(g)
    double g = std::max(1e-6, mDrive * 0.7 * (1.0 + mAsymmetry));
    double x = input;
    double y = std::tanh(g * x);
    double norm = std::tanh(g);
    return (norm > 0.0 ? y / norm : y);
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
    constexpr double threshold = 1.0;

    // Modulo-based foldback into [-threshold, threshold]
    const double ax = std::abs(x);
    double y = std::fmod(ax, 2.0 * threshold);
    if (y > threshold) y = 2.0 * threshold - y;
    y = std::copysign(y, x);

    return y * 0.7; // Scale down to prevent excessive levels
}

double MultiDisto::bitcrushDistortion(double input) const {
    // Map drive (in dB) to a 0..1 control for bit depth and rate reduction
    double driveDbNorm = std::clamp(utils::linearToDB(mDrive) / kMaxDriveDb, 0.0, 1.0);

    // Bit depth: from 16 bits (low drive) down to 4 bits (high drive)
    int bits = 4 + static_cast<int>(std::round((1.0 - driveDbNorm) * 12.0));
    bits = std::clamp(bits, 1, 24);
    double levels = std::pow(2.0, bits) - 1.0;

    // Sample-rate reduction: hold every N samples, from 1 (no SRR) up to ~40 at max drive
    const int holdN = 1 + static_cast<int>(std::round(driveDbNorm * 39.0));

    // Quantize a clipped version of the signal to avoid explosive outputs
    double x = std::clamp(input, -1.0, 1.0);

    if (mBitcrushPhase == 0) {
        mBitcrushHold = std::round(x * levels) / levels;
    }
    mBitcrushPhase = (mBitcrushPhase + 1) % holdN;

    return mBitcrushHold;
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
