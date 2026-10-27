#pragma once
#include <cmath>

class EnvelopeWarp {
public:
    // phase: 0.0 to 1.0 across duration
    // warp: 0.05 (skew left) to 0.95 (skew right), 0.5 is symmetric
    // curveShape: curvature power (1.0 = linear, 2.0+ = exponential bell)
    static float evaluate(float phase, float warp, float curveShape = 2.0f) {
        warp = std::clamp(warp, 0.01f, 0.99f);
        float env = 0.0f;

        if (phase < warp) {
            float norm = phase / warp;
            env = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * norm));
        } else {
            float norm = (phase - warp) / (1.0f - warp);
            env = 0.5f * (1.0f + std::cos(juce::MathConstants<float>::pi * norm));
        }

        return std::pow(std::clamp(env, 0.0f, 1.0f), curveShape);
    }
};