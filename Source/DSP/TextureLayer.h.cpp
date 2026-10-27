#pragma once
#include <juce_dsp/juce_dsp.h>
#include <random>

class TextureLayer {
public:
    enum FilterMode { LowPass = 0, HighPass, Comb };

    void prepare(double sr) {
        sampleRate = sr;
        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        filter.prepare(spec);
        combDelay.prepare(spec);
        combDelay.setMaximumDelayInSamples(static_cast<int>(sr * 0.05)); // 50ms comb max
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
            filter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
            filter.state->setCutOffFrequency(sampleRate, targetCutoff, 0.707f);
        } else if (mode == HighPass) {
            filter.state->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::highPass;
            filter.state->setCutOffFrequency(sampleRate, targetCutoff, 0.707f);
        }
    }

    void process(float& outL, float& outR) {
        // Procedural multi-category textural generator
        float raw = generateCategorySample();

        // Filter processing
        float filtered = raw;
        if (currentMode == Comb) {
            float delaySamples = std::clamp(sampleRate / std::max(targetCutoff, 50.0f), 1.0, sampleRate * 0.04);
            combDelay.pushSample(0, raw);
            filtered = raw + 0.85f * combDelay.popSample(0, delaySamples);
        } else {
            filtered = filter.processSample(0, raw);
        }

        // Spectral Tilt approximation
        tiltState = 0.9f * tiltState + 0.1f * filtered;
        float tilted = (tilt > 0.0f) ? (filtered + (filtered - tiltState) * tilt)
                                    : (filtered + tiltState * (-tilt));

        // Stereo pan mapping (equal power)
        float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        outL = tilted * std::cos(panAngle);
        outR = tilted * std::sin(panAngle);
    }

private:
    float generateCategorySample() {
        // Phase accumulator modified by pitch ratio
        phase += (0.005f * pitchRatio);
        if (phase >= 1.0f) phase -= 1.0f;

        float white = dist(gen);

        switch (currentCategory) {
            case 0: // Air / Wind: Low-frequency turbulent noise
                noiseSmooth = 0.98f * noiseSmooth + 0.02f * white;
                return noiseSmooth * 4.0f;

            case 1: // Fire / Crackle: Sparse stochastic impulses
                if (std::abs(white) > 0.985f)
                    return white * 3.0f;
                return white * 0.05f;

            case 2: // Electricity: 50/60Hz harmonic buzz with FM
                return std::sin(phase * juce::MathConstants<float>::twoPi * 60.0f + (white * 0.4f));

            case 3: // Metal / Resonance: Ringing metallic modes
                metalPhase1 += 0.073f * pitchRatio;
                metalPhase2 += 0.119f * pitchRatio;
                if (metalPhase1 > 1.0f) metalPhase1 -= 1.0f;
                if (metalPhase2 > 1.0f) metalPhase2 -= 1.0f;
                return (std::sin(metalPhase1 * juce::MathConstants<float>::twoPi) * 0.5f +
                        std::sin(metalPhase2 * juce::MathConstants<float>::twoPi) * 0.5f) * (0.4f + 0.6f * white);

            case 4: // Water / Rain: Pink-filtered droplet impulses
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

    juce::dsp::ProcessorDuplicator<juce::dsp::StateVariableFilter::Filter<float>,
                                  juce::dsp::StateVariableFilter::Parameters<float>> filter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> combDelay{ 44100 };
};