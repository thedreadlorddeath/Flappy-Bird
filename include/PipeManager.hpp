#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct PipePair {
    float x;
    float gapY;       // Center of the vertical gap
    float gapHeight;  // Size of the gap
    bool scored;

    // Hitboxes
    sf::FloatRect getTopBodyRect() const;
    sf::FloatRect getTopCapRect() const;
    sf::FloatRect getBottomCapRect() const;
    sf::FloatRect getBottomBodyRect() const;
};

class PipeManager {
public:
    PipeManager();

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();

    // Collision detection with bird circle
    bool checkCollision(sf::Vector2f birdPos, float birdRadius) const;

    // Check if bird passed any unscored pipe
    bool checkScored(float birdX);

    const std::vector<PipePair>& getPipes() const { return m_pipes; }

private:
    void spawnPipe();
    void drawSinglePipe(sf::RenderWindow& window, float x, float topY, float btmY, bool isTop);

    std::vector<PipePair> m_pipes;
    float m_lastGapY;
    float m_spawnDistance;
};
