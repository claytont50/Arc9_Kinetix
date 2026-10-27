#pragma once
#include <juce_dsp/juce_dsp.h>
#include <random>

class NoiseEngine {
public:
    enum NoiseType {
        White = 0, Pink, Brown, Violet, Blue,
        Band, SampleAndHold, Lorenz, Rossler, Logistic, Sinus,
        NumTypes
    };

    void prepare(double sr) {
        sampleRate = sr;
        b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0f;
        lastOutput = 0.0f;
    }

    float getNextSample(NoiseType type, float filterCutoff = 1000.0f) {
        float white = dist(gen);

        switch (type) {
            case White: return white;

            case Pink: { // Paul Kellet 3-pole filter
                b0 = 0.99886f * b0 + white * 0.0555179f;
                b1 = 0.99332f * b1 + white * 0.0750759f;
                b2 = 0.96900f * b2 + white * 0.1538520f;
                b3 = 0.86650f * b3 + white * 0.3104856f;
                b4 = 0.55000f * b4 + white * 0.5329522f;
                b5 = -0.7616f * b5 - white * 0.0168980f;
                float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
                b6 = white * 0.115926f;
                return pink * 0.11f;
            }

            case Brown: { // Leaky integrator
                lastOutput = (lastOutput + (0.02f * white)) / 1.02f;
                return lastOutput * 3.5f;
            }

            case Violet: { // Differentiated noise
                float v = white - lastOutput;
                lastOutput = white;
                return v * 0.5f;
            }

            case Blue: { // Spectral tilt
                float b = white - (0.5f * lastOutput);
                lastOutput = white;
                return b * 0.6f;
            }

            case Logistic: { // Dynamic chaos map
                static float x = 0.54321f;
                float r = 3.99f; // Edge of chaos
                x = r * x * (1.0f - x);
                return (x * 2.0f) - 1.0f;
            }

            case Lorenz: { // Runge-Kutta numerical integration
                static float lx = 0.1f, ly = 0.0f, lz = 0.0f;
                const float sigma = 10.0f, rho = 28.0f, beta = 8.0f / 3.0f;
                const float dt = 0.0005f;
                lx += sigma * (ly - lx) * dt;
                ly += (lx * (rho - lz) - ly) * dt;
                lz += (lx * ly - beta * lz) * dt;
                return lx * 0.05f;
            }

            default:
                return white;
        }
    }

private:
    double sampleRate = 44100.0;
    std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };
    float b0, b1, b2, b3, b4, b5, b6, lastOutput;
};