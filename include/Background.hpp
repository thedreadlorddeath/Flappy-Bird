#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct DecItem {
    int stage;       // 0 to 9
    int itemType;    // 0 to 9 (10 items per stage)
    float x;
    float y;
    float speed;
    float scale;
    float alpha;
    float param;     // Animation phase / rotation
};

class Background {
public:
    Background();

    void setScreenHeight(float screenHeight) { m_screenHeight = screenHeight; }
    float getScreenHeight() const { return m_screenHeight; }

    void update(float dt, int score);
    void drawSky(sf::RenderWindow& window, int score);
    void drawParallax(sf::RenderWindow& window, int score);
    void drawGround(sf::RenderWindow& window, int score);

    void reset();

private:
    void initDecorations();
    void drawDecorations(sf::RenderWindow& window, int stage);
    void drawSingleDecItem(sf::RenderWindow& window, const DecItem& item, float time);

    float m_screenHeight;
    float m_time;

    // Scrolling offsets
    float m_bgOffset;
    float m_midOffset;
    float m_fgOffset;
    float m_groundOffset;

    // All active themed decorative items (10 per stage)
    std::vector<DecItem> m_decorations;
};
