#include "MultiDisto.h"

#include "disstortion.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cmath>

namespace stfefane::dsp {

void MultiDisto::initParameterAttachments(const Disstortion& d) {
    using namespace params;
    mParameterAttachments.reserve(d.getParameters().count());

    auto add_basic_attachment = [&](clap_id id, SmoothedValue& attached_to) {
        mParameterAttachments.emplace_back(d.getParameter(id), [&](Parameter* param, double new_val) {
            attached_to = param->getValueType().denormalizedValue(new_val);
        });
    };
    add_basic_attachment(eAsymmetry, mAsymmetry);

    auto add_dB_attachment = [&]<typename T>(clap_id id, T& attached_to) {
        mParameterAttachments.emplace_back(d.getParameter(id), [&](Parameter* param, double new_val) {
            attached_to = utils::dbToLinear(param->getValueType().denormalizedValue(new_val));
        });
    };
    add_dB_attachment(eDrive, mDrive);
    add_dB_attachment(eInGain, mInputGain);
    add_dB_attachment(eOutGain, mOutputGain);

    mParameterAttachments.emplace_back(d.getParameter(eMix), [&](Parameter*, double new_val) {
        mMix = new_val;
    });
    mParameterAttachments.emplace_back(d.getParameter(eDriveType), [&](Parameter*, double new_type) {
        mType = static_cast<dsp::DistortionType>(new_type);
    });

    mParameterAttachments.emplace_back(d.getParameter(ePreFilterOn), [&](Parameter*, double new_pre) {
        mPreFilterOn = new_pre > .5;
    });
    mParameterAttachments.emplace_back(d.getParameter(ePreFilterType), [&](Parameter*, double new_type) {
        mPreFilter.setType(static_cast<BiquadFilter::Type>(new_type + 1)); // +1 because we skip None.
    });
    mParameterAttachments.emplace_back(d.getParameter(ePreFilterFreq), [&](Parameter* param, double new_freq) {
        mPreFilter.setFreq(param->getValueType().denormalizedValue(new_freq));
    });
    mParameterAttachments.emplace_back(d.getParameter(ePreFilterQ), [&](Parameter* param, double new_q) {
        mPreFilter.setQ(param->getValueType().denormalizedValue(new_q));
    });
    mParameterAttachments.emplace_back(d.getParameter(ePreFilterGain), [&](Parameter* param, double new_gain) {
        mPreFilter.setGainDb(param->getValueType().denormalizedValue(new_gain));
    });

    mParameterAttachments.emplace_back(d.getParameter(ePostFilterOn), [&](Parameter*, double new_post) {
        mPostFilterOn = new_post > .5;
    });
    mParameterAttachments.emplace_back(d.getParameter(ePostFilterType), [&](Parameter*, double new_type) {
        mPostFilter.setType(static_cast<BiquadFilter::Type>(new_type + 1)); // +1 because we skip None.
    });
    mParameterAttachments.emplace_back(d.getParameter(ePostFilterFreq), [&](Parameter* param, double new_freq) {
        mPostFilter.setFreq(param->getValueType().denormalizedValue(new_freq));
    });
    mParameterAttachments.emplace_back(d.getParameter(ePostFilterQ), [&](Parameter* param, double new_q) {
        mPostFilter.setQ(param->getValueType().denormalizedValue(new_q));
    });
    mParameterAttachments.emplace_back(d.getParameter(ePostFilterGain), [&](Parameter* param, double new_gain) {
        mPostFilter.setGainDb(param->getValueType().denormalizedValue(new_gain));
    });
}

void MultiDisto::setSampleRate(double samplerate) {
    LOG_INFO("dsp", "[MultiDisto::setSampleRate] new_samplerate = {}", samplerate);
    mSampleRate = samplerate;
    mOversampler.setupAntiAliasing(samplerate);
    mPreFilter.setSampleRate(samplerate);
    mPostFilter.setSampleRate(samplerate);
    mDrive.setup(samplerate, 10.);
    mAsymmetry.setup(samplerate, 5.);
}

void MultiDisto::reset() {
    mPreFilter.reset();
    mPostFilter.reset();
    mDCBlocker = DCBlocker{};
}

double MultiDisto::process(double input) {
    // Compute smoothing of values
    smoothValues();

    // Store dry signal for mix
    double drySignal = input;

    // Apply input gain
    double signal = input * mInputGain;

    // Pre-filter
    if (mPreFilterOn && mPreFilter.getType() != BiquadFilter::Type::None) {
        signal = mPreFilter.process(signal);
    }

    // Determine if non-linear stage can be bypassed (drive ~ 0dB and no asymmetry)
    const bool bypassNonLinear = utils::almostEqual<double>(mDrive, 1.)
                                && utils::almostEqual<double>(mAsymmetry, 0.);

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

        // DC blocking (only needed when using non-linearities)
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

    // Final soft safety limiting
    return std::tanh(signal);
}

void MultiDisto::smoothValues() {
    mDrive.process();
    mAsymmetry.process();
}

double MultiDisto::applyDistortion(double input) const {
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
    }
    double sign = (x > 0.0) ? 1.0 : -1.0;
    return sign * (1.0 - std::pow(2.0 - 3.0 * std::abs(x), 2.0) / 3.0);
}

double MultiDisto::tubeSaturation(double input) const {
    // Normalize tanh drive to avoid level jumps: y = tanh(g*x) / tanh(g)
    double g = std::max(1e-6, 0.7 * (1.0 + mAsymmetry));
    double x = input * mDrive;
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
    }
    if (x < negThresh) {
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
    if (y > threshold) {
        y = 2.0 * threshold - y;
    }
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
