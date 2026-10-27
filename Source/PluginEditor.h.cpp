#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class Arc9KinetixEditor : public juce::AudioProcessorEditor {
public:
    Arc9KinetixEditor(Arc9KinetixProcessor&);
    ~Arc9KinetixEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Arc9KinetixProcessor& audioProcessor;

    // Component bounds constrainer keeps aspect ratio locked
    juce::ComponentBoundsConstrainer constrainer;

    // UI Color Palette from Reference
    const juce::Colour bgSlate    = juce::Colour(0xff161a1d);
    const juce::Colour cardDark   = juce::Colour(0xff22282e);
    const juce::Colour accentTeal = juce::Colour(0xff3dd5b0);
    const juce::Colour noiseYel   = juce::Colour(0xfff0ad1b);
    const juce::Colour tex1Pink   = juce::Colour(0xffe03b8b);
    const juce::Colour tex2Cyan   = juce::Colour(0xff399be2);

    juce::Slider durationSlider;
    juce::Slider warpSlider;
    juce::TabbedComponent tabComponent { juce::TabbedButtonBar::TabsAtLeft };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Arc9KinetixEditor)
};