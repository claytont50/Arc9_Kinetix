#include "PluginEditor.h"

Arc9KinetixEditor::Arc9KinetixEditor(Arc9KinetixProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    
    // Set 16:9 native aspect ratio (1280x720) with strict constraints
    setConstrainer(&constrainer);
    constrainer.setFixedAspectRatio(1280.0 / 720.0);
    constrainer.setSizeLimits(960, 540, 2560, 1440);

    setSize(1280, 720);
    setResizable(true, true);

    // Setup Warp Slider (mirrors top center reference slider)
    warpSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    warpSlider.setRange(0.05, 0.95, 0.01);
    warpSlider.setValue(0.5);
    warpSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(warpSlider);
}

Arc9KinetixEditor::~Arc9KinetixEditor() {}

void Arc9KinetixEditor::paint(juce::Graphics& g) {
    g.fillAll(bgSlate);

    // Dynamic resolution scaling factor
    float scale = (float)getWidth() / 1280.0f;

    // Center Stage Wave Envelope Visualizer Box
    auto centerArea = juce::Rectangle<float>(320 * scale, 60 * scale, 640 * scale, 340 * scale);
    g.setColour(cardDark);
    g.fillRoundedRectangle(centerArea, 8.0f * scale);

    // Render Layer Splines (Noise, Texture 1, Texture 2)
    juce::Path envelopePath;
    float centerY = centerArea.getCentreY();
    float height = centerArea.getHeight() * 0.45f;
    float warp = (float)warpSlider.getValue();

    envelopePath.startNewSubPath(centerArea.getX(), centerY);

    for (float x = 0; x <= centerArea.getWidth(); x += 2.0f) {
        float phase = x / centerArea.getWidth();
        // Envelope curve calculation
        float amp = EnvelopeWarp::evaluate(phase, warp, 2.5f);
        envelopePath.lineTo(centerArea.getX() + x, centerY - (amp * height));
    }
    // Complete bottom reflection mirroring UVI layout
    for (float x = centerArea.getWidth(); x >= 0; x -= 2.0f) {
        float phase = x / centerArea.getWidth();
        float amp = EnvelopeWarp::evaluate(phase, warp, 2.5f);
        envelopePath.lineTo(centerArea.getX() + x, centerY + (amp * height));
    }
    envelopePath.closeSubPath();

    g.setColour(noiseYel.withAlpha(0.65f));
    g.fillPath(envelopePath);
    g.setColour(noiseYel);
    g.strokePath(envelopePath, juce::PathStrokeType(1.5f * scale));

    // Brand Label
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Segoe UI", 24.0f * scale, juce::Font::bold));
    g.drawText("WHOOSH", 20 * scale, 15 * scale, 200 * scale, 30 * scale, juce::Justification::left);
    g.setColour(accentTeal);
    g.setFont(juce::Font("Segoe UI", 12.0f * scale, juce::Font::bold));
    g.drawText("ARC-9 FX", 145 * scale, 13 * scale, 80 * scale, 20 * scale, juce::Justification::left);
}

void Arc9KinetixEditor::resized() {
    float scale = (float)getWidth() / 1280.0f;
    warpSlider.setBounds(330 * scale, 38 * scale, 620 * scale, 16 * scale);
}