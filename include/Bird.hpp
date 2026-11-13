#pragma once

#include <SFML/Graphics.hpp>

class Bird {
public:
    Bird();

    void update(float dt);
    void updateBobbing(float dt);
    void flap();
    void reset();

    void draw(sf::RenderWindow& window);

    // Getters
    sf::Vector2f getPosition() const { return m_position; }
    float getHitRadius() const;
    sf::FloatRect getBounds() const;
    bool isAlive() const { return m_isAlive; }
    void setAlive(bool alive) { m_isAlive = alive; }

    void setPosition(sf::Vector2f pos) { m_position = pos; }
    float getVelocity() const { return m_velocity; }

private:
    sf::Vector2f m_position;
    float m_velocity;
    float m_rotation;       // Degrees
    float m_targetRotation; // Degrees

    // Flapping wing animation state
    float m_wingCycle;      // Radians
    float m_bobTimer;       // For Get Ready floating

    bool m_isAlive;
};
