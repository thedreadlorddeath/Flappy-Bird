#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#if defined(__ANDROID__)
#include <sys/stat.h>
#endif

namespace Config {
    // Window dimensions
    constexpr float WINDOW_WIDTH  = 480.0f;
    constexpr float WINDOW_HEIGHT = 720.0f;
    constexpr float GROUND_HEIGHT = 110.0f;
    constexpr float PLAY_HEIGHT   = WINDOW_HEIGHT - GROUND_HEIGHT; // 610.0f

    // Bird Physics
    constexpr float BIRD_START_X     = 130.0f;
    constexpr float BIRD_START_Y     = 300.0f;
    constexpr float BIRD_RADIUS      = 17.0f;  // Visual radius
    constexpr float BIRD_HIT_RADIUS  = 13.5f;  // Fair collision hitbox (~80%)
    constexpr float GRAVITY          = 1050.0f; // px/s^2
    constexpr float FLAP_IMPULSE     = -370.0f; // px/s
    constexpr float MAX_FALL_SPEED   = 580.0f;  // Terminal velocity
    constexpr float TILT_UP_ANGLE    = -25.0f; // Degrees when flapping
    constexpr float TILT_DOWN_ANGLE  = 80.0f;  // Degrees nosedive
    constexpr float TILT_SPEED       = 10.0f;  // Smoothing factor

    // Pipe Parameters
    constexpr float PIPE_SPEED       = 175.0f; // px/s
    constexpr float PIPE_WIDTH       = 74.0f;
    constexpr float PIPE_CAP_WIDTH   = 84.0f;
    constexpr float PIPE_CAP_HEIGHT  = 26.0f;
    constexpr float PIPE_GAP         = 150.0f; // Vertical space for bird
    constexpr float PIPE_SPACING     = 230.0f; // Distance between pipe columns
    constexpr float MIN_PIPE_GAP_Y   = 160.0f; // Center of gap
    constexpr float MAX_PIPE_GAP_Y   = 450.0f;
    constexpr float MAX_GAP_DELTA    = 140.0f; // Reachability guarantee

    // Parallax Speeds (px/s)
    constexpr float PARALLAX_MOUNTAINS = 25.0f;
    constexpr float PARALLAX_HILLS     = 60.0f;
    constexpr float PARALLAX_CLOUDS    = 35.0f;
    constexpr float PARALLAX_GROUND    = PIPE_SPEED; // Synchronized with pipes

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
