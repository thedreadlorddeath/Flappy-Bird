#include "SoundGenerator.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    constexpr unsigned int SAMPLE_RATE = 44100;
    constexpr double PI = 3.14159265358979323846;

    inline sf::Int16 clampSample(double v) {
        if (v > 32767.0) return 32767;
        if (v < -32768.0) return -32768;
        return static_cast<sf::Int16>(v);
    }
}

SoundGenerator::SoundGenerator() {
    try {
        generateFlapSound();
        generateScoreSound();
        generateHitSound();
        generateMedalSound();

        m_flapSound.setBuffer(m_flapBuffer);
        m_flapSound.setVolume(75.0f);

        m_scoreSound.setBuffer(m_scoreBuffer);
        m_scoreSound.setVolume(85.0f);

        m_hitSound.setBuffer(m_hitBuffer);
        m_hitSound.setVolume(90.0f);

        m_medalSound.setBuffer(m_medalBuffer);
        m_medalSound.setVolume(80.0f);
    } catch (...) {
        // Safe fallback if OpenAL device fails to initialize on mobile
    }
}

void SoundGenerator::generateFlapSound() {
    const double duration = 0.13;
    const size_t totalSamples = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(totalSamples);

    double phase = 0.0;
    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double progress = t / duration;

        // Frequency sweep: 280Hz -> 540Hz -> 360Hz
        double freq = 280.0 + 520.0 * progress * (1.0 - progress * 0.7);
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        // Amplitude envelope: quick attack, smooth exponential decay
        double env = 1.0;
        if (progress < 0.15) {
            env = progress / 0.15;
        } else {
            env = std::pow(1.0 - (progress - 0.15) / 0.85, 2.2);
        }

        // Add soft noise rustle
        double noise = ((std::rand() % 2000) - 1000) / 1000.0 * 0.18;
        double wave = (std::sin(phase) + 0.3 * std::sin(phase * 2.0) + noise) * env;

        samples[i] = clampSample(wave * 24000.0);
    }

    m_flapBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void SoundGenerator::generateScoreSound() {
    const double duration = 0.25;
    const size_t totalSamples = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(totalSamples);

    double phase1 = 0.0;
    double phase2 = 0.0;
    const double f1 = 987.77;  // B5
    const double f2 = 1318.51; // E6

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double sampleVal = 0.0;

        // Note 1 (B5) for first 0.09s
        if (t < 0.11) {
            double note1T = t / 0.11;
            phase1 += 2.0 * PI * f1 / SAMPLE_RATE;
            double env1 = std::exp(-note1T * 4.5);
            double tone1 = std::sin(phase1) + 0.25 * std::sin(phase1 * 2.0);
            sampleVal += tone1 * env1 * 0.75;
        }

        // Note 2 (E6) starting at 0.07s
        if (t >= 0.07) {
            double note2T = (t - 0.07) / (duration - 0.07);
            phase2 += 2.0 * PI * f2 / SAMPLE_RATE;
            double env2 = std::exp(-note2T * 3.8);
            double tone2 = std::sin(phase2) + 0.35 * std::sin(phase2 * 2.0) + 0.15 * std::sin(phase2 * 3.0);
            sampleVal += tone2 * env2 * 0.95;
        }

        samples[i] = clampSample(sampleVal * 22000.0);
    }

    m_scoreBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void SoundGenerator::generateHitSound() {
    const double duration = 0.28;
    const size_t totalSamples = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(totalSamples);

    double phase = 0.0;
    double noiseFilter = 0.0;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double progress = t / duration;

        // Low pitch downward plunge: 190Hz -> 45Hz
        double freq = 190.0 * std::exp(-progress * 8.0) + 45.0;
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        // Punchy attack and exponential decay
        double env = std::exp(-progress * 7.5);

        // Low-pass filtered noise for crunch
        double rawNoise = ((std::rand() % 2000) - 1000) / 1000.0;
        noiseFilter += (rawNoise - noiseFilter) * 0.25;

        double wave = (std::sin(phase) + 0.5 * std::sin(phase * 1.5)) * 0.75 + noiseFilter * 0.45;
        samples[i] = clampSample(wave * env * 28000.0);
    }

    m_hitBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void SoundGenerator::generateMedalSound() {
    const double noteDuration = 0.11;
    const double duration = noteDuration * 3.0 + 0.15;
    const size_t totalSamples = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(totalSamples);

    const double freqs[3] = { 523.25, 659.25, 783.99 }; // C5, E5, G5
    double phase = 0.0;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        int noteIdx = std::min(2, static_cast<int>(t / noteDuration));
        double noteT = std::fmod(t, noteDuration) / noteDuration;
        if (t >= noteDuration * 2.0) {
            noteT = (t - noteDuration * 2.0) / (duration - noteDuration * 2.0);
        }

        double freq = freqs[noteIdx];
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        double env = std::exp(-noteT * 3.0);
        double tone = std::sin(phase) + 0.3 * std::sin(phase * 2.0) + 0.1 * std::sin(phase * 3.0);

        samples[i] = clampSample(tone * env * 20000.0);
    }

    m_medalBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void SoundGenerator::playFlap() {
    try {
        m_flapSound.stop();
        m_flapSound.play();
    } catch (...) {}
}

void SoundGenerator::playScore() {
    try {
        m_scoreSound.stop();
        m_scoreSound.play();
    } catch (...) {}
}

void SoundGenerator::playHit() {
    try {
        m_hitSound.stop();
        m_hitSound.play();
    } catch (...) {}
}

void SoundGenerator::playMedal() {
    try {
        m_medalSound.stop();
        m_medalSound.play();
    } catch (...) {}
}
