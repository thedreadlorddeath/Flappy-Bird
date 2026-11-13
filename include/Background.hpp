#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct Cloud {
    float x;
    float y;
    float speed;
    float scale;
    float alpha;
};

class Background {
public:
    Background();

    void update(float dt, int score);
    void drawSky(sf::RenderWindow& window, int score);
    void drawParallax(sf::RenderWindow& window);
    void drawGround(sf::RenderWindow& window);

    void reset();

private:
    void initMountains();
    void initClouds();
    void initHills();

    // Parallax scroll offsets
    float m_mountainOffset;
    float m_hillOffset;
    float m_groundOffset;

    // Time counter for shimmering sun & subtle animations
    float m_time;

    // Clouds
    std::vector<Cloud> m_clouds;

    // Procedural geometry
    sf::VertexArray m_skyVertices;
    sf::VertexArray m_mountainVertices;
    sf::VertexArray m_hillVertices;
};
