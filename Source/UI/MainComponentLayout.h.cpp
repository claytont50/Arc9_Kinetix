#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "EnvelopeComponent.h"

class MainComponentLayout : public juce::Component {
public:
    MainComponentLayout() {
        // Envelope & Warp
        addAndMakeVisible(envelopeView);
        
        warpSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        warpSlider.setRange(0.05, 0.95, 0.001);
        warpSlider.setValue(0.5);
        warpSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        warpSlider.onValueChange = [this]() {
            envelopeView.setWarp(static_cast<float>(warpSlider.getValue()));
        };
        addAndMakeVisible(warpSlider);

        // Duration Knob
        durationKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        durationKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        durationKnob.setRange(0.1, 16.0, 0.01);
        durationKnob.setValue(1.0);
        addAndMakeVisible(durationKnob);

        // Movement Knobs (Doppler Pitch & Filter Depth)
        pitchKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        pitchKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        pitchKnob.setRange(0.0, 100.0, 1.0);
        pitchKnob.setValue(80.0);
        addAndMakeVisible(pitchKnob);

        filterDepthKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        filterDepthKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        filterDepthKnob.setRange(20.0, 20000.0, 1.0);
        filterDepthKnob.setSkewForCentre(1000.0);
        filterDepthKnob.setValue(2500.0);
        addAndMakeVisible(filterDepthKnob);

        // Navigation Tabs for Lower Drawer
        tabButtonSources.setButtonText("SOURCES");
        tabButtonVowel.setButtonText("VOWEL");
        tabButtonFX.setButtonText("FX");
        addAndMakeVisible(tabButtonSources);
        addAndMakeVisible(tabButtonVowel);
        addAndMakeVisible(tabButtonFX);

        tabButtonSources.setRadioGroupId(1001);
        tabButtonVowel.setRadioGroupId(1001);
        tabButtonFX.setRadioGroupId(1001);
        tabButtonSources.setToggleState(true, juce::dontSendNotification);
    }

    void resized() override {
        auto area = getLocalBounds();
        float scale = area.getWidth() / 1280.0f;

        // Top Envelope + Controls
        int headerHeight = static_cast<int>(50 * scale);
        auto topArea = area.removeFromTop(static_cast<int>(460 * scale)).reduced(static_cast<int>(12 * scale));
        topArea.removeFromTop(headerHeight);

        // Left Panel (Duration)
        auto leftPanel = topArea.removeFromLeft(static_cast<int>(250 * scale));
        durationKnob.setBounds(leftPanel.removeFromTop(static_cast<int>(200 * scale)).reduced(static_cast<int>(20 * scale)));

        // Right Panel (Movement & Filter)
        auto rightPanel = topArea.removeFromRight(static_cast<int>(280 * scale));
        pitchKnob.setBounds(rightPanel.removeFromTop(static_cast<int>(180 * scale)).reduced(static_cast<int>(15 * scale)));
        filterDepthKnob.setBounds(rightPanel.removeFromTop(static_cast<int>(180 * scale)).reduced(static_cast<int>(15 * scale)));

        // Center Stage (Warp slider + Envelope View)
        warpSlider.setBounds(topArea.removeFromTop(static_cast<int>(26 * scale)).reduced(static_cast<int>(40 * scale), 0));
        envelopeView.setBounds(topArea.reduced(static_cast<int>(10 * scale)));

        // Bottom Tab Selector Drawer
        auto bottomNav = area.removeFromLeft(static_cast<int>(120 * scale)).reduced(static_cast<int>(8 * scale));
        int btnHeight = bottomNav.getHeight() / 3;
        tabButtonSources.setBounds(bottomNav.removeFromTop(btnHeight).reduced(2));
        tabButtonVowel.setBounds(bottomNav.removeFromTop(btnHeight).reduced(2));
        tabButtonFX.setBounds(bottomNav.removeFromTop(btnHeight).reduced(2));
    }

    EnvelopeComponent envelopeView;
    juce::Slider warpSlider;
    juce::Slider durationKnob;
    juce::Slider pitchKnob;
    juce::Slider filterDepthKnob;

    juce::TextButton tabButtonSources;
    juce::TextButton tabButtonVowel;
    juce::TextButton tabButtonFX;
};