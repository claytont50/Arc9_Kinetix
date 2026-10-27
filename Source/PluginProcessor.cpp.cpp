#include "PluginProcessor.h"
#include "PluginEditor.h"

Arc9KinetixProcessor::Arc9KinetixProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {}

Arc9KinetixProcessor::~Arc9KinetixProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout Arc9KinetixProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("duration", "Duration", 0.1f, 16.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("warp", "Envelope Warp", 0.05f, 0.95f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("noiseType", "Noise Type",
        juce::StringArray{ "White", "Pink", "Brown", "Violet", "Blue", "Band", "S+H", "Lorenz", "Rossler", "Logistic", "Sinus" }, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>("tex1Cat", "Texture 1 Category", 0, 4, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("tex1Pitch", "Texture 1 Pitch", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("tex2Cat", "Texture 2 Category", 0, 4, 1));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("tex2Pitch", "Texture 2 Pitch", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("dopplerPitch", "Doppler Depth", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("vowelOn", "Vowel Filter Enable", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>("vowel1", "Start Vowel", 0, 4, 1)); // E
    params.push_back(std::make_unique<juce::AudioParameterInt>("vowel2", "End Vowel", 0, 4, 0));   // A
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fxReverb", "Reverb Wet", 0.0f, 1.0f, 0.35f));

    return { params.begin(), params.end() };
}

void Arc9KinetixProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };

    noiseEngine.prepare(sampleRate);
    texture1.prepare(sampleRate);
    texture2.prepare(sampleRate);
    dopplerEngine.prepare(sampleRate);
    vowelFilter.prepare(spec);
    masterFX.prepare(spec);

    totalDurationSamples = *apvts.getRawParameterValue("duration") * sampleRate;
}

void Arc9KinetixProcessor::releaseResources() {}

void Arc9KinetixProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // Check MIDI triggers (Note-On triggers envelope cycle)
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            isTriggered = true;
            triggerSampleCounter = 0.0;
        }
    }

    if (!isTriggered) {
        buffer.clear();
        return;
    }

    // Read parameter snapshots
    float durationSec = *apvts.getRawParameterValue("duration");
    float warp = *apvts.getRawParameterValue("warp");
    int nType = static_cast<int>(*apvts.getRawParameterValue("noiseType"));
    int t1Cat = static_cast<int>(*apvts.getRawParameterValue("tex1Cat"));
    float t1P = *apvts.getRawParameterValue("tex1Pitch");
    int t2Cat = static_cast<int>(*apvts.getRawParameterValue("tex2Cat"));
    float t2P = *apvts.getRawParameterValue("tex2Pitch");
    float dopPitch = *apvts.getRawParameterValue("dopplerPitch");
    bool vOn = *apvts.getRawParameterValue("vowelOn") > 0.5f;
    int v1 = static_cast<int>(*apvts.getRawParameterValue("vowel1"));
    int v2 = static_cast<int>(*apvts.getRawParameterValue("vowel2"));
    float revMix = *apvts.getRawParameterValue("fxReverb");

    totalDurationSamples = std::max(durationSec * getSampleRate(), 1000.0);
    masterFX.setReverbMix(revMix);

    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : outL;

    for (int i = 0; i < numSamples; ++i) {
        if (!isTriggered) {
            outL[i] = 0.0f;
            outR[i] = 0.0f;
            continue;
        }

        float progress = static_cast<float>(triggerSampleCounter / totalDurationSamples);
        currentProgress.store(progress);

        // 1. Calculate layer amplitude from Warp Spline
        float envNoise = EnvelopeWarp::evaluate(progress, warp, 2.5f);
        float envTex1  = EnvelopeWarp::evaluate(progress, warp * 0.92f, 2.8f);
        float envTex2  = EnvelopeWarp::evaluate(progress, warp * 1.08f, 2.2f);

        // 2. Generate Sound Sources
        float nSig = noiseEngine.getNextSample(static_cast<NoiseEngine::NoiseType>(nType)) * envNoise;

        float t1L = 0.0f, t1R = 0.0f;
        texture1.setParameters(t1Cat, t1P, 2500.0f, TextureLayer::LowPass, -0.2f, 0.0f);
        texture1.process(t1L, t1R);
        t1L *= envTex1; t1R *= envTex1;

        float t2L = 0.0f, t2R = 0.0f;
        texture2.setParameters(t2Cat, t2P, 800.0f, TextureLayer::LowPass, 0.2f, 0.0f);
        texture2.process(t2L, t2R);
        t2L *= envTex2; t2R *= envTex2;

        float mixL = (nSig * 0.5f) + t1L + t2L;
        float mixR = (nSig * 0.5f) + t1R + t2R;

        // 3. Vowel Filter Formant Morphing (if enabled)
        if (vOn) {
            vowelFilter.updateFormants(v1, v2, progress, 9.0f);
            mixL = vowelFilter.processSample(mixL);
            mixR = vowelFilter.processSample(mixR);
        }

        // 4. Movement & Doppler Engine
        float dopL = 0.0f, dopR = 0.0f;
        dopplerEngine.process(mixL, mixR, progress, dopPitch, 0, dopL, dopR);

        outL[i] = dopL;
        outR[i] = dopR;

        // Advance sample counter
        triggerSampleCounter += 1.0;
        if (triggerSampleCounter >= totalDurationSamples) {
            isTriggered = false;
            currentProgress.store(0.0f);
        }
    }

    // 5. Final Master Effects
    masterFX.process(buffer);
}

void Arc9KinetixProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Arc9KinetixProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* Arc9KinetixProcessor::createEditor() {
    return new Arc9KinetixEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new Arc9KinetixProcessor();
}