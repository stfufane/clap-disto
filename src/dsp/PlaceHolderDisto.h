#pragma once
#include <cmath>
#include <algorithm>
#include <array>
#include "../helpers/Utils.h"

class AdvancedDistortion {
public:
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

    enum class FilterType {
        NONE,
        LOW_PASS,
        HIGH_PASS,
        BAND_PASS,
        NOTCH,
        PEAK
    };

private:
    // Filter coefficients for biquad filters
    struct BiquadFilter {
        double a0, a1, a2, b1, b2;
        double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
        
        double process(double input) {
            double output = a0 * input + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
            
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            
            return output;
        }
        
        void reset() {
            x1 = x2 = y1 = y2 = 0.0;
        }
    };

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

    // Oversampling for anti-aliasing
    class Oversampler {
    public:
        static constexpr int FACTOR = 4;
        
    private:
        // Simple linear interpolation upsampler
        std::array<double, FACTOR> buffer = {};
        BiquadFilter antiAliasingFilter;
        
    public:
        void setupAntiAliasing(double sampleRate) {
            // Low-pass filter at Nyquist/2 to prevent aliasing
            double freq = sampleRate * 0.25; // Quarter of original sample rate
            double w = stfefane::utils::kTWO_PI_64 * freq / (sampleRate * FACTOR);
            double cosw = cos(w);
            double sinw = sin(w);
            double alpha = sinw / (2.0 * 0.707); // Q = 0.707 for Butterworth
            
            antiAliasingFilter.b1 = -2.0 * cosw;
            antiAliasingFilter.b2 = 1.0 - alpha;
            antiAliasingFilter.a0 = (1.0 - cosw) * 0.5;
            antiAliasingFilter.a1 = 1.0 - cosw;
            antiAliasingFilter.a2 = (1.0 - cosw) * 0.5;
            
            // Normalize
            double norm = 1.0 / (1.0 + alpha);
            antiAliasingFilter.a0 *= norm;
            antiAliasingFilter.a1 *= norm;
            antiAliasingFilter.a2 *= norm;
            antiAliasingFilter.b1 *= norm;
            antiAliasingFilter.b2 *= norm;
        }
        
        std::array<double, FACTOR> upsample(double input) {
            buffer[0] = input;
            for (int i = 1; i < FACTOR; i++) {
                buffer[i] = 0.0; // Zero-stuffing
            }
            
            // Apply anti-aliasing filter to each sample
            for (int i = 0; i < FACTOR; i++) {
                buffer[i] = antiAliasingFilter.process(buffer[i]);
            }
            
            return buffer;
        }
        
        double downsample(const std::array<double, FACTOR>& samples) {
            // Simple decimation - take every FACTOR-th sample
            // In practice, you'd want better anti-aliasing here too
            return samples[0];
        }
    };

    double sampleRate = 44100.0;
    BiquadFilter preFilter, postFilter;
    DCBlocker dcBlocker;
    Oversampler oversampler;
    
    // Parameters
    DistortionType distortionType = DistortionType::CUBIC_SATURATION;
    FilterType preFilterType = FilterType::NONE;
    FilterType postFilterType = FilterType::NONE;
    
    double inputGain = 1.0;
    double outputGain = 1.0;
    double driveAmount = 1.0;
    double mix = 1.0; // Wet/dry mix
    double preFilterFreq = 1000.0;
    double preFilterQ = 0.707;
    double postFilterFreq = 5000.0;
    double postFilterQ = 0.707;
    double asymmetry = 0.0; // For asymmetric distortion
    double bias = 0.0;

public:
    void setSampleRate(double rate) {
        sampleRate = rate;
        oversampler.setupAntiAliasing(rate);
        updateFilters();
    }

    // Parameter setters
    void setDistortionType(DistortionType type) { distortionType = type; }
    void setInputGain(double gain) { inputGain = std::max(0.0, gain); }
    void setOutputGain(double gain) { outputGain = std::max(0.0, gain); }
    void setDrive(double drive) { driveAmount = std::max(0.1, drive); }
    void setMix(double mixAmount) { mix = std::clamp(mixAmount, 0.0, 1.0); }
    void setAsymmetry(double asym) { asymmetry = std::clamp(asym, -1.0, 1.0); }
    void setBias(double biasAmount) { bias = std::clamp(biasAmount, -0.5, 0.5); }
    
    void setPreFilter(FilterType type, double freq, double q = 0.707) {
        preFilterType = type;
        preFilterFreq = std::clamp(freq, 20.0, 22'000.);
        preFilterQ = std::clamp(q, 0.1, 10.0);
        updateFilters();
    }
    
    void setPostFilter(FilterType type, double freq, double q = 0.707) {
        postFilterType = type;
        postFilterFreq = std::clamp(freq, 20.0, 22'000.);
        postFilterQ = std::clamp(q, 0.1, 10.0);
        updateFilters();
    }

private:
    void updateFilters() {
        updateBiquadFilter(preFilter, preFilterType, preFilterFreq, preFilterQ);
        updateBiquadFilter(postFilter, postFilterType, postFilterFreq, postFilterQ);
    }
    
    void updateBiquadFilter(BiquadFilter& filter, FilterType type, double freq, double q) {
        if (type == FilterType::NONE) return;
        
        double w = 2.0 * stfefane::utils::kPI * freq / sampleRate;
        double cosw = cos(w);
        double sinw = sin(w);
        double alpha = sinw / (2.0 * q);
        
        switch (type) {
            case FilterType::LOW_PASS:
                filter.b1 = -2.0 * cosw;
                filter.b2 = 1.0 - alpha;
                filter.a0 = (1.0 - cosw) * 0.5;
                filter.a1 = 1.0 - cosw;
                filter.a2 = (1.0 - cosw) * 0.5;
                break;
                
            case FilterType::HIGH_PASS:
                filter.b1 = -2.0 * cosw;
                filter.b2 = 1.0 - alpha;
                filter.a0 = (1.0 + cosw) * 0.5;
                filter.a1 = -(1.0 + cosw);
                filter.a2 = (1.0 + cosw) * 0.5;
                break;
                
            case FilterType::BAND_PASS:
                filter.b1 = -2.0 * cosw;
                filter.b2 = 1.0 - alpha;
                filter.a0 = sinw * 0.5;
                filter.a1 = 0.0;
                filter.a2 = -sinw * 0.5;
                break;
                
            case FilterType::NOTCH:
                filter.b1 = -2.0 * cosw;
                filter.b2 = 1.0 - alpha;
                filter.a0 = 1.0;
                filter.a1 = -2.0 * cosw;
                filter.a2 = 1.0;
                break;
                
            case FilterType::PEAK: {
                double A = pow(10.0, 6.0 / 40.0); // 6dB boost/cut
                filter.b1 = -2.0 * cosw;
                filter.b2 = 1.0 - alpha / A;
                filter.a0 = 1.0 + alpha * A;
                filter.a1 = -2.0 * cosw;
                filter.a2 = 1.0 - alpha * A;
                break;
            }
            default:
                return;
        }
        
        // Normalize coefficients
        double norm = 1.0 / (1.0 + alpha);
        filter.a0 *= norm;
        filter.a1 *= norm;
        filter.a2 *= norm;
        filter.b1 *= norm;
        filter.b2 *= norm;
    }
    
    double applyDistortion(double input) {
        // Add bias
        input += bias;
        
        switch (distortionType) {
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
    
    // Distortion algorithms
    double cubicSaturation(double input) {
        double x = input * driveAmount;
        if (std::abs(x) < 2.0/3.0) {
            return x * (1.0 + asymmetry * x);
        } else {
            double sign = (x > 0.0) ? 1.0 : -1.0;
            return sign * (1.0 - pow(2.0 - 3.0 * std::abs(x), 2.0) / 3.0);
        }
    }
    
    double tubeSaturation(double input) {
        double x = input * driveAmount * 0.7;
        // Tube-like saturation curve
        return tanh(x * (1.0 + asymmetry)) * (1.0 + 0.1 * x * x);
    }
    
    double asymmetricClip(double input) {
        double x = input * driveAmount;
        double posThresh = 0.7 + asymmetry * 0.3;
        double negThresh = -0.7 - asymmetry * 0.3;
        
        if (x > posThresh) {
            return posThresh + (x - posThresh) * 0.1;
        } else if (x < negThresh) {
            return negThresh + (x - negThresh) * 0.1;
        }
        return x;
    }
    
    double foldbackDistortion(double input) {
        double x = input * driveAmount;
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
    
    double bitcrushDistortion(double input) {
        double x = input * driveAmount;
        int bits = std::max(1, (int)(16 - driveAmount * 12)); // Variable bit depth
        double levels = pow(2.0, bits) - 1.0;
        return round(x * levels) / levels;
    }
    
    double waveShaperDistortion(double input) {
        double x = input * driveAmount;
        // Sigmoid-based waveshaping
        double k = 2.0 * driveAmount;
        return x * (1.0 + k) / (1.0 + k * std::abs(x));
    }
    
    double tubeScreamerDistortion(double input) {
        // Tube Screamer-inspired soft clipping
        double x = input * driveAmount * 2.0;
        double sign = (x >= 0.0) ? 1.0 : -1.0;
        x = std::abs(x);
        
        if (x < 1.0/3.0) {
            return sign * 2.0 * x;
        } else if (x < 2.0/3.0) {
            return sign * (3.0 - pow(2.0 - 3.0 * x, 2.0)) / 3.0;
        } else {
            return sign;
        }
    }
    
    double fuzzFaceDistortion(double input) {
        // Fuzz Face-inspired germanium transistor distortion
        double x = input * driveAmount * 1.5;
        double sign = (x >= 0.0) ? 1.0 : -1.0;
        x = std::abs(x);
        
        // Asymmetric germanium-like curve
        double pos_curve = 1.0 - exp(-x * (2.0 + asymmetry));
        double neg_curve = 1.0 - exp(-x * (2.0 - asymmetry));
        
        return sign * (sign > 0 ? pos_curve : neg_curve) * 0.8;
    }

public:
    double process(double input) {
        // Store dry signal for mix
        double drySignal = input;
        
        // Apply input gain
        double signal = input * inputGain;
        
        // Pre-filter
        if (preFilterType != FilterType::NONE) {
            signal = preFilter.process(signal);
        }
        
        // Oversampling for anti-aliasing (optional, can be toggled)
        bool useOversampling = (driveAmount > 2.0); // Only use for heavy distortion
        
        if (useOversampling) {
            auto upsampled = oversampler.upsample(signal);
            for (auto& sample : upsampled) {
                sample = applyDistortion(sample);
            }
            signal = oversampler.downsample(upsampled);
        } else {
            signal = applyDistortion(signal);
        }
        
        // DC blocking
        signal = dcBlocker.process(signal);
        
        // Post-filter
        if (postFilterType != FilterType::NONE) {
            signal = postFilter.process(signal);
        }
        
        // Apply output gain
        signal *= outputGain;
        
        // Wet/dry mix
        signal = mix * signal + (1.0 - mix) * drySignal;
        
        // Final safety limiting
        return std::clamp(signal, -0.95, 0.95);
    }
    
    void reset() {
        preFilter.reset();
        postFilter.reset();
        dcBlocker = DCBlocker{};
    }
};

// Usage example for CLAP integration:
/*
class ClapDistoPlugin {
private:
    AdvancedDistortion distortion;
    
public:
    void setSampleRate(double rate) {
        distortion.setSampleRate(rate);
    }
    
    void processAudio(float** inputs, float** outputs, uint32_t frameCount) {
        for (uint32_t i = 0; i < frameCount; ++i) {
            // Process left channel
            outputs[0][i] = distortion.process(inputs[0][i]);
            
            // For stereo, you'd want separate instances or process right channel
            if (numChannels > 1) {
                outputs[1][i] = distortion.process(inputs[1][i]); 
            }
        }
    }
    
    void setParameters(uint32_t paramId, double value) {
        switch (paramId) {
            case 0: distortion.setInputGain(value); break;
            case 1: distortion.setDrive(value); break;
            case 2: distortion.setOutputGain(value); break;
            case 3: distortion.setMix(value); break;
            case 4: distortion.setDistortionType((AdvancedDistortion::DistortionType)(int)value); break;
            case 5: distortion.setPreFilter(AdvancedDistortion::FilterType::LOW_PASS, value); break;
            case 6: distortion.setPostFilter(AdvancedDistortion::FilterType::HIGH_PASS, value); break;
            case 7: distortion.setAsymmetry(value); break;
            case 8: distortion.setBias(value); break;
        }
    }
};
*/