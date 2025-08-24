#pragma once

namespace stfefane::dsp {

enum class FilterType {
    NONE,
    LOW_PASS,
    HIGH_PASS,
    BAND_PASS,
    NOTCH,
    PEAK
};

class BiquadFilter {

public:
    BiquadFilter() = default;
    BiquadFilter(double freq, double q, FilterType type);

    void setup(FilterType type, double freq, double q);
    void setFreq(double freq) { mFreq = freq; }
    void setSampleRate(double samplerate) { mSampleRate = samplerate; }

    FilterType getType() const { return mType; }

    double process(double input);

    void update();
    void normalize(double factor);
    void reset();

private:
    double a0 = 0., a1 = 0., a2 = 0., b1 = 0., b2 = 0.;
    double x1 = 0., x2 = 0., y1 = 0., y2 = 0.;

    double mFreq = 1000.;
    double mQ = 0.707;
    FilterType mType = FilterType::NONE;

    double mSampleRate = 44100.;
};

}