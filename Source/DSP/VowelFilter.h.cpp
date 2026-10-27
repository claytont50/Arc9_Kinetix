#pragma once
#include <juce_dsp/juce_dsp.h>

class VowelFilter {
public:
    // Formant frequency lookup (F1, F2, F3 in Hz)
    const float vowelFrequencies[5][3] = {
        { 800.0f, 1200.0f, 2500.0f }, // 'A'
        { 400.0f, 2000.0f, 2800.0f }, // 'E'
        { 250.0f, 2300.0f, 3000.0f }, // 'I'
        { 500.0f, 850.0f,  2400.0f }, // 'O'
        { 320.0f, 700.0f,  2200.0f }  // 'U'
    };

    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
        for (int i = 0; i < 3; ++i) {
            filterBand[i].prepare(spec);
        }
    }

    // morphPosition: 0.0 (Vowel 1) to 1.0 (Vowel 2)
    void updateFormants(int vowelIdx1, int vowelIdx2, float morphPosition, float Q) {
        vowelIdx1 = std::clamp(vowelIdx1, 0, 4);
        vowelIdx2 = std::clamp(vowelIdx2, 0, 4);

        for (int i = 0; i < 3; ++i) {
            float f1 = vowelFrequencies[vowelIdx1][i];
            float f2 = vowelFrequencies[vowelIdx2][i];
            float currentFreq = f1 + morphPosition * (f2 - f1);

            *filterBand[i].state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(
                sampleRate, currentFreq, Q);
        }
    }

    float processSample(float input) {
        return (filterBand[0].processSample(input) * 1.0f +
                filterBand[1].processSample(input) * 0.7f +
                filterBand[2].processSample(input) * 0.4f);
    }

private:
    double sampleRate = 44100.0;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filterBand[3];
};