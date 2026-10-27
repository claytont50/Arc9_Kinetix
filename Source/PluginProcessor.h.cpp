#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DSP/NoiseEngine.h"
#include "DSP/TextureLayer.h"
#include "DSP/EnvelopeWarp.h"
#include "DSP/DopplerEngine.h"
#include "DSP/VowelFilter.h"
#include "DSP/MasterFX.h"

class Arc9KinetixProcessor : public juce::AudioProcessor {
public:
    Arc9KinetixProcessor();
    ~Arc9KinetixProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Arc-9 Kinetix"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    float getCurrentProgress() const { return currentProgress.load(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Engines
    NoiseEngine noiseEngine;
    TextureLayer texture1;
    TextureLayer texture2;
    DopplerEngine dopplerEngine;
    VowelFilter vowelFilter;
    MasterFX masterFX;

    // Trigger & Time Progress State Machine
    std::atomic<float> currentProgress { 0.0f };
    bool isTriggered = false;
    double triggerSampleCounter = 0.0;
    double totalDurationSamples = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Arc9KinetixProcessor)
};