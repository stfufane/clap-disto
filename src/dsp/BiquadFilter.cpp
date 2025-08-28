#include "BiquadFilter.h"

#include <cmath>
#include "../helpers/Utils.h"

namespace stfefane::dsp {

BiquadFilter::BiquadFilter(double freq, double q, FilterType type)
    : mFreq(freq), mQ(q), mType(type)
{
}

void BiquadFilter::setup(FilterType type, double freq, double q) {
    mType = type;
    mFreq = freq;
    mQ = q;
    update();
}

void BiquadFilter::setFreq(double freq) {
    mFreq = freq;
    update();
}

void BiquadFilter::setSampleRate(double samplerate) {
    mSampleRate = samplerate;
    update();
}

double BiquadFilter::process(double input) {
    double output = a0 * input + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;

    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;

    return output;
}

void BiquadFilter::reset() {
    x1 = x2 = y1 = y2 = 0.0;
}

void BiquadFilter::update() {
    if (mType == FilterType::NONE) return;

    double w = utils::kTWO_PI_64 * mFreq / mSampleRate;
    double cosw = cos(w);
    double sinw = sin(w);
    double alpha = sinw / (2.0 * mQ);

    switch (mType) {
        case FilterType::LOW_PASS:
            b1 = -2.0 * cosw;
            b2 = 1.0 - alpha;
            a0 = (1.0 - cosw) * 0.5;
            a1 = 1.0 - cosw;
            a2 = (1.0 - cosw) * 0.5;
            break;

        case FilterType::HIGH_PASS:
            b1 = -2.0 * cosw;
            b2 = 1.0 - alpha;
            a0 = (1.0 + cosw) * 0.5;
            a1 = -(1.0 + cosw);
            a2 = (1.0 + cosw) * 0.5;
            break;

        case FilterType::BAND_PASS:
            b1 = -2.0 * cosw;
            b2 = 1.0 - alpha;
            a0 = sinw * 0.5;
            a1 = 0.0;
            a2 = -sinw * 0.5;
            break;

        case FilterType::NOTCH:
            b1 = -2.0 * cosw;
            b2 = 1.0 - alpha;
            a0 = 1.0;
            a1 = -2.0 * cosw;
            a2 = 1.0;
            break;

        case FilterType::PEAK: {
            const double A = pow(10.0, 6.0 / 40.0); // 6dB boost/cut
            b1 = -2.0 * cosw;
            b2 = 1.0 - alpha / A;
            a0 = 1.0 + alpha * A;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alpha * A;
            break;
        }
        default:
            return;
    }

    // Normalize coefficients
    normalize(1.0 / (1.0 + alpha));
}

void BiquadFilter::normalize(double factor) {
    a0 *= factor;
    a1 *= factor;
    a2 *= factor;
    b1 *= factor;
    b2 *= factor;
}

} // namespace stfefane::dsp
