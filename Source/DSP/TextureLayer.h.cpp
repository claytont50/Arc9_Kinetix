#pragma once
#include <juce_dsp/juce_dsp.h>
#include <random>

class TextureLayer {
public:
    enum FilterMode { LowPass = 0, HighPass, Comb };

    void prepare(double sr) {
        sampleRate = sr;
        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        tptFilter.prepare(spec);
        tptFilter.reset();

        combDelay.prepare({ sr, 512, 1 });
        combDelay.setMaximumDelayInSamples(static_cast<int>(sr * 0.05));
    }

    void setParameters(int category, float pitchSemitones, float cutoffHz, FilterMode mode, 
                       float panVal, float tiltVal) {
        currentCategory = category;
        pitchRatio = std::pow(2.0f, pitchSemitones / 12.0f);
        targetCutoff = std::clamp(cutoffHz, 20.0f, 20000.0f);
        currentMode = mode;
        pan = std::clamp(panVal, -1.0f, 1.0f);
        tilt = std::clamp(tiltVal, -1.0f, 1.0f);

        if (mode == LowPass) {
            tptFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            tptFilter.setCutoffFrequency(targetCutoff);
        } else if (mode == HighPass) {
            tptFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            tptFilter.setCutoffFrequency(targetCutoff);
        }
    }

    void process(float& outL, float& outR) {
        float raw = generateCategorySample();
        float filtered = raw;

        if (currentMode == Comb) {
            float delaySamples = std::clamp(static_cast<float>(sampleRate) / std::max(targetCutoff, 50.0f), 1.0f, static_cast<float>(sampleRate * 0.04));
            combDelay.pushSample(0, raw);
            filtered = raw + 0.85f * combDelay.popSample(0, delaySamples);
        } else {
            filtered = tptFilter.processSample(0, raw);
        }

        // Tilt EQ
        tiltState = 0.9f * tiltState + 0.1f * filtered;
        float tilted = (tilt > 0.0f) ? (filtered + (filtered - tiltState) * tilt)
                                    : (filtered + tiltState * (-tilt));

        // Equal-power stereo pan
        float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        outL = tilted * std::cos(panAngle);
        outR = tilted * std::sin(panAngle);
    }

private:
    float generateCategorySample() {
        phase += (0.005f * pitchRatio);
        if (phase >= 1.0f) phase -= 1.0f;

        float white = dist(gen);

        switch (currentCategory) {
            case 0: // Air / Wind
                noiseSmooth = 0.98f * noiseSmooth + 0.02f * white;
                return noiseSmooth * 4.0f;
            case 1: // Fire / Crackle
                if (std::abs(white) > 0.985f) return white * 3.0f;
                return white * 0.05f;
            case 2: // Electricity
                return std::sin(phase * juce::MathConstants<float>::twoPi * 60.0f + (white * 0.4f));
            case 3: // Metal
                metalPhase1 += 0.073f * pitchRatio;
                metalPhase2 += 0.119f * pitchRatio;
                if (metalPhase1 > 1.0f) metalPhase1 -= 1.0f;
                if (metalPhase2 > 1.0f) metalPhase2 -= 1.0f;
                return (std::sin(metalPhase1 * juce::MathConstants<float>::twoPi) * 0.5f +
                        std::sin(metalPhase2 * juce::MathConstants<float>::twoPi) * 0.5f) * (0.4f + 0.6f * white);
            case 4: // Water / Rain
            default:
                noiseSmooth = 0.92f * noiseSmooth + 0.08f * white;
                return noiseSmooth * 2.2f;
        }
    }

    double sampleRate = 44100.0;
    int currentCategory = 0;
    float pitchRatio = 1.0f;
    float targetCutoff = 1000.0f;
    FilterMode currentMode = LowPass;
    float pan = 0.0f;
    float tilt = 0.0f;
    float phase = 0.0f;
    float noiseSmooth = 0.0f;
    float tiltState = 0.0f;
    float metalPhase1 = 0.0f, metalPhase2 = 0.0f;

    std::mt19937 gen{ 1337 };
    std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };

    juce::dsp::StateVariableTPTFilter<float> tptFilter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> combDelay{ 44100 };
};