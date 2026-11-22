#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct PipePair {
    float x;
    float baseGapY;
    float gapY;       // Center of the vertical gap
    float gapHeight;  // Size of the gap
    float oscPhase;
    int stage;
    bool scored;
    float playHeight;

    // Hitboxes
    sf::FloatRect getTopBodyRect() const;
    sf::FloatRect getTopCapRect() const;
    sf::FloatRect getBottomCapRect() const;
    sf::FloatRect getBottomBodyRect() const;
};

class PipeManager {
public:
    PipeManager();

    void setPlayHeight(float playHeight) { m_playHeight = playHeight; }
    float getPlayHeight() const { return m_playHeight; }

    void update(float dt, int score);
    void draw(sf::RenderWindow& window, int score, float time);
    void reset();

    // Collision detection with bird circle
    bool checkCollision(sf::Vector2f birdPos, float birdRadius) const;

    // Check if bird passed any unscored pipe
    bool checkScored(float birdX);

    const std::vector<PipePair>& getPipes() const { return m_pipes; }

private:
    void spawnPipe(int score);
    void drawThemedObstaclePair(sf::RenderWindow& window, const PipePair& pipe, float time);

    std::vector<PipePair> m_pipes;
    float m_lastGapY;
    float m_spawnDistance;
    float m_playHeight;
    float m_time;
};
