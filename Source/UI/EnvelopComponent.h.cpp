#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/EnvelopeWarp.h"

class EnvelopeComponent : public juce::Component {
public:
    EnvelopeComponent() {
        setOpaque(false);
    }

    void setWarp(float w) {
        warpValue = std::clamp(w, 0.05f, 0.95f);
        repaint();
    }

    void setPlayhead(float p) {
        playheadPos = std::clamp(p, 0.0f, 1.0f);
        repaint();
    }

    void setActiveLayer(int layer) {
        activeLayer = layer; // 0: All, 1: Noise, 2: Tex 1, 3: Tex 2
        repaint();
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        float centerY = bounds.getCentreY();
        float maxHeight = bounds.getHeight() * 0.44f;

        // Draw Dark Background Box
        g.setColour(juce::Colour(0xff1b2024));
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0xff2d343c));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // Grid lines
        g.setColour(juce::Colour(0xff272d34));
        g.drawDashedLine(juce::Line<float>(bounds.getX(), centerY, bounds.getRight(), centerY),
                         dashLengths, 2, 1.0f);
        g.drawDashedLine(juce::Line<float>(bounds.getX() + bounds.getWidth() * 0.25f, bounds.getY(),
                                           bounds.getX() + bounds.getWidth() * 0.25f, bounds.getBottom()),
                         dashLengths, 2, 1.0f);
        g.drawDashedLine(juce::Line<float>(bounds.getX() + bounds.getWidth() * 0.75f, bounds.getY(),
                                           bounds.getX() + bounds.getWidth() * 0.75f, bounds.getBottom()),
                         dashLengths, 2, 1.0f);

        // Draw Layer 3: Texture 2 (Cyan)
        if (activeLayer == 0 || activeLayer == 3)
            drawSymmetricEnvelope(g, bounds, warpValue * 1.08f, 2.2f, maxHeight * 0.85f,
                                 juce::Colour(0xff399be2), 0.35f);

        // Draw Layer 2: Texture 1 (Magenta)
        if (activeLayer == 0 || activeLayer == 2)
            drawSymmetricEnvelope(g, bounds, warpValue * 0.92f, 2.8f, maxHeight * 0.92f,
                                 juce::Colour(0xffe03b8b), 0.40f);

        // Draw Layer 1: Noise (Yellow)
        if (activeLayer == 0 || activeLayer == 1)
            drawSymmetricEnvelope(g, bounds, warpValue, 2.5f, maxHeight * 1.0f,
                                 juce::Colour(0xfff0ad1b), 0.65f);

        // Draw Moving Playhead Cursor
        if (playheadPos > 0.0f && playheadPos < 1.0f) {
            float playX = bounds.getX() + playheadPos * bounds.getWidth();
            g.setColour(juce::Colour(0xff3dd5b0));
            g.drawLine(playX, bounds.getY() + 4.0f, playX, bounds.getBottom() - 4.0f, 2.0f);
            g.fillEllipse(playX - 4.0f, bounds.getY() + 4.0f, 8.0f, 8.0f);
        }
    }

private:
    void drawSymmetricEnvelope(juce::Graphics& g, juce::Rectangle<float> bounds,
                               float warp, float curvePower, float height,
                               juce::Colour color, float alpha) {
        juce::Path p;
        float centerY = bounds.getCentreY();
        p.startNewSubPath(bounds.getX(), centerY);

        for (float x = 0.0f; x <= bounds.getWidth(); x += 3.0f) {
            float phase = x / bounds.getWidth();
            float val = EnvelopeWarp::evaluate(phase, warp, curvePower);
            p.lineTo(bounds.getX() + x, centerY - (val * height));
        }
        for (float x = bounds.getWidth(); x >= 0.0f; x -= 3.0f) {
            float phase = x / bounds.getWidth();
            float val = EnvelopeWarp::evaluate(phase, warp, curvePower);
            p.lineTo(bounds.getX() + x, centerY + (val * height));
        }
        p.closeSubPath();

        g.setColour(color.withAlpha(alpha));
        g.fillPath(p);
        g.setColour(color);
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }

    float warpValue = 0.5f;
    float playheadPos = 0.0f;
    int activeLayer = 0;
    const float dashLengths[2] = { 4.0f, 4.0f };
};