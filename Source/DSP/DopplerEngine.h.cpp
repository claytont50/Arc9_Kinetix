#pragma once
#include <juce_dsp/juce_dsp.h>

class DopplerEngine {
public:
    void prepare(double sr, int maxDelay = 44100) {
        sampleRate = sr;
        delayLineL.prepare({ (double)sr, (juce::uint32)maxDelay, 1 });
        delayLineR.prepare({ (double)sr, (juce::uint32)maxDelay, 1 });
    }

    // progress: 0.0 -> 1.0 across the whoosh duration
    // speedCurve: non-linear velocity distribution
    void process(float inL, float inR, float progress, float pitchDepth,
                 int mode, float& outL, float& outR) {
        // Map 0..1 progress to spatial coordinates: X from -10m to +10m, Y at constant 1.2m distance
        float x = (progress - 0.5f) * 20.0f; 
        float y = 1.2f; 
        
        // Listener head ear offsets (binaural approximation)
        float dL = std::sqrt((x + 0.15f) * (x + 0.15f) + y * y);
        float dR = std::sqrt((x - 0.15f) * (x - 0.15f) + y * y);

        // Acoustic delay modulation (Doppler pitch change)
        const float speedOfSound = 343.0f; // m/s
        float delaySecL = (dL / speedOfSound) * (pitchDepth * 2.0f);
        float delaySecR = (dR / speedOfSound) * (pitchDepth * 2.0f);

        delayLineL.pushSample(0, inL);
        delayLineR.pushSample(0, inR);

        float wetL = delayLineL.popSample(0, delaySecL * (float)sampleRate);
        float wetR = delayLineR.popSample(0, delaySecR * (float)sampleRate);

        // Amplitude attenuation based on inverse-square law
        float attenL = 1.0f / (dL * 0.8f + 0.2f);
        float attenR = 1.0f / (dR * 0.8f + 0.2f);

        outL = wetL * attenL;
        outR = wetR * attenR;
    }

private:
    double sampleRate = 44100.0;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineL{ 44100 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLineR{ 44100 };
};