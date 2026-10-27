#pragma once
#include <juce_dsp/juce_dsp.h>

class MasterFX {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;

        // Reverb Setup
        reverbParams.roomSize = 0.82f;
        reverbParams.damping  = 0.35f;
        reverbParams.wetLevel = 0.33f;
        reverbParams.dryLevel = 1.0f;
        reverbParams.width    = 1.0f;
        reverb.setParameters(reverbParams);
        reverb.reset();

        // Delay Line Setup (1.5 seconds stereo delay buffer)
        delayL.prepare(spec);
        delayR.prepare(spec);
        delayL.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
        delayR.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));

        // Limiter Setup
        limiter.prepare(spec);
        limiter.setThreshold(0.0f); // 0 dBFS ceiling
        limiter.setRelease(25.0f);   // 25 ms release

        // 5-Band EQ Setup
        for (int i = 0; i < 5; ++i)
            eqBands[i].prepare(spec);

        updateEQ(100.0f, 300.0f, 1000.0f, 3500.0f, 8000.0f);
    }

    void updateEQ(float f1, float f2, float f3, float f4, float f5) {
        *eqBands[0].state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, f1, 0.707f, 1.0f);
        *eqBands[1].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f2, 1.0f, 1.0f);
        *eqBands[2].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f3, 1.0f, 1.0f);
        *eqBands[3].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f4, 1.0f, 1.0f);
        *eqBands[4].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, f5, 0.707f, 1.0f);
    }

    void setReverbMix(float wet) {
        reverbParams.wetLevel = std::clamp(wet, 0.0f, 1.0f);
        reverbParams.dryLevel = 1.0f - (wet * 0.4f);
        reverb.setParameters(reverbParams);
    }

    void setDelayParams(float timeMs, float feedback, float mix) {
        delayTimeSamples = (timeMs / 1000.0f) * sampleRate;
        delayFeedback = std::clamp(feedback, 0.0f, 0.95f);
        delayMix = std::clamp(mix, 0.0f, 1.0f);
    }

    void process(juce::AudioBuffer<float>& buffer) {
        const int numSamples = buffer.getNumSamples();
        float* channelL = buffer.getWritePointer(0);
        float* channelR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : channelL;

        // 1. 5-Band EQ
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (int i = 0; i < 5; ++i)
            eqBands[i].process(context);

        // 2. Delay processing
        for (int i = 0; i < numSamples; ++i) {
            float inL = channelL[i];
            float inR = channelR[i];

            float dOutL = delayL.popSample(0, delayTimeSamples);
            float dOutR = delayR.popSample(0, delayTimeSamples * 1.333f); // Stereo offset

            delayL.pushSample(0, inL + (dOutR * delayFeedback));
            delayR.pushSample(0, inR + (dOutL * delayFeedback));

            channelL[i] = inL * (1.0f - delayMix) + dOutL * delayMix;
            channelR[i] = inR * (1.0f - delayMix) + dOutR * delayMix;
        }

        // 3. Reverb processing
        if (reverbParams.wetLevel > 0.01f) {
            if (buffer.getNumChannels() > 1)
                reverb.processStereo(channelL, channelR, numSamples);
            else
                reverb.processMono(channelL, numSamples);
        }

        // 4. Limiter processing
        limiter.process(context);
    }

private:
    double sampleRate = 44100.0;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL{ 88200 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR{ 88200 };
    float delayTimeSamples = 4410.0f;
    float delayFeedback = 0.3f;
    float delayMix = 0.2f;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> eqBands[5];
    juce::dsp::Limiter<float> limiter;
};