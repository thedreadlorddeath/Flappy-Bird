#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float size;
    float lifetime;
    float maxLifetime;
    float rotation;
    float rotationSpeed;
    bool isFeather;
};

class ParticleSystem {
public:
    ParticleSystem();

    void update(float dt);
    void draw(sf::RenderWindow& window);

    // Emitters
    void emitFlapParticles(sf::Vector2f position);
    void emitImpactParticles(sf::Vector2f position);

    // Juice Effects
    void triggerShake(float intensity = 1.0f);
    void triggerFlash(float intensity = 1.0f);

    sf::Vector2f getShakeOffset() const;
    void drawFlash(sf::RenderWindow& window);

    void reset();

private:
    std::vector<Particle> m_particles;

    // Screen Shake
    float m_shakeTrauma;
    float m_shakeTimer;

    // Screen Flash
    float m_flashAlpha;
};
