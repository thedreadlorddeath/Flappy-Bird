#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <cmath>
#include <algorithm>
#if defined(__ANDROID__)
#include <sys/stat.h>
#endif

namespace Config {
    // Window dimensions
    constexpr float WINDOW_WIDTH         = 480.0f;
    constexpr float WINDOW_HEIGHT        = 800.0f;
    constexpr float DEFAULT_PLAY_HEIGHT  = 610.0f;
    constexpr float GROUND_HEIGHT        = 110.0f;
    constexpr int   MAX_SCORE            = 999;

    // Bird Physics
    constexpr float BIRD_START_X     = 130.0f;
    constexpr float BIRD_START_Y     = 350.0f;
    constexpr float BIRD_RADIUS      = 17.0f;  // Visual radius
    constexpr float BIRD_HIT_RADIUS  = 13.5f;  // Fair collision hitbox (~80%)
    constexpr float GRAVITY          = 1050.0f; // px/s^2
    constexpr float FLAP_IMPULSE     = -370.0f; // px/s
    constexpr float MAX_FALL_SPEED   = 580.0f;  // Terminal velocity
    constexpr float TILT_UP_ANGLE    = -25.0f; // Degrees when flapping
    constexpr float TILT_DOWN_ANGLE  = 80.0f;  // Degrees nosedive
    constexpr float TILT_SPEED       = 10.0f;  // Smoothing factor

    // Base Obstacle Parameters
    constexpr float BASE_PIPE_SPEED  = 175.0f; // px/s
    constexpr float PIPE_WIDTH       = 74.0f;
    constexpr float PIPE_CAP_WIDTH   = 86.0f;
    constexpr float PIPE_CAP_HEIGHT  = 26.0f;
    constexpr float MAX_GAP_DELTA    = 135.0f; // Reachability guarantee

    // Difficulty and Stage Helpers
    inline int getStageIndex(int score) {
        return std::clamp(score / 100, 0, 9);
    }

    inline std::string getStageName(int stage) {
        switch (stage) {
            case 0: return "CLASSIC SUNSET";
            case 1: return "EMERALD FOREST";
            case 2: return "PREHISTORIC DINO";
            case 3: return "CYBER METROPOLIS";
            case 4: return "ORBITAL SPACE";
            case 5: return "COSMIC UNIVERSE";
            case 6: return "XIANXIA REALM";
            case 7: return "LORD OF THE MYSTERIES";
            case 8: return "ASTRAL RIFT";
            case 9: return "FOOL'S APOTHEOSIS";
            default: return "SUNSET ARCADE";
        }
    }

    inline float getSpeedMultiplier(int score) {
        float t = std::clamp(static_cast<float>(score) / 999.0f, 0.0f, 1.0f);
        int stage = getStageIndex(score);
        return 1.0f + (1.35f * std::pow(t, 0.78f)) + (stage * 0.04f);
    }

    inline float getCurrentPipeSpeed(int score) {
        return BASE_PIPE_SPEED * getSpeedMultiplier(score);
    }

    inline float getCurrentGapHeight(int score) {
        float t = std::clamp(static_cast<float>(score) / 999.0f, 0.0f, 1.0f);
        return std::max(118.0f, 182.0f - (64.0f * std::pow(t, 0.72f)));
    }

    inline float getHorizontalSpacing(int score) {
        int stage = getStageIndex(score);
        return std::max(210.0f, 275.0f - (stage * 6.0f));
    }

    inline float getOscillationAmplitude(int score) {
        if (score < 15) return 0.0f;
        float t = std::clamp(static_cast<float>(score) / 999.0f, 0.0f, 1.0f);
        int stage = getStageIndex(score);
        return std::min(46.0f, 6.0f + 32.0f * std::pow(t, 0.8f) + stage * 0.8f);
    }

    inline float getOscillationFrequency(int score) {
        float t = std::clamp(static_cast<float>(score) / 999.0f, 0.0f, 1.0f);
        int stage = getStageIndex(score);
        return 1.35f + 1.15f * t + stage * 0.08f;
    }

    // Save File Path Resolvers
    inline std::string getSaveFilePath(const std::string& filename) {
#if defined(__ANDROID__)
        const char* internalDir = "/data/data/com.thedreadlorddeath.flappybird/files";
        mkdir(internalDir, 0755);
        return std::string(internalDir) + "/" + filename;
#else
        return filename;
#endif
    }

    inline std::string getHighScoreFilePath() {
        return getSaveFilePath("highscore.txt");
    }

    inline std::string getCosmeticsFilePath() {
        return getSaveFilePath("cosmetics.txt");
    }

    inline const char* HIGHSCORE_FILE = "highscore.txt";
}
