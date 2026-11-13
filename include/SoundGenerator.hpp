#pragma once

#include <SFML/Audio.hpp>
#include <vector>
#include <cstdint>

class SoundGenerator {
public:
    SoundGenerator();
    ~SoundGenerator() = default;

    void playFlap();
    void playScore();
    void playHit();
    void playMedal();

private:
    void generateFlapSound();
    void generateScoreSound();
    void generateHitSound();
    void generateMedalSound();

    sf::SoundBuffer m_flapBuffer;
    sf::SoundBuffer m_scoreBuffer;
    sf::SoundBuffer m_hitBuffer;
    sf::SoundBuffer m_medalBuffer;

    sf::Sound m_flapSound;
    sf::Sound m_scoreSound;
    sf::Sound m_hitSound;
    sf::Sound m_medalSound;
};
