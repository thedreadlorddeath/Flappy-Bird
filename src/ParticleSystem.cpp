#include "ParticleSystem.hpp"
#include "Config.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    float randomFloat(float min, float max) {
        return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
}

ParticleSystem::ParticleSystem()
    : m_shakeTrauma(0.0f)
    , m_shakeTimer(0.0f)
    , m_flashAlpha(0.0f) {
    m_particles.reserve(256);
}

void ParticleSystem::update(float dt) {
    // Update active particles
    for (size_t i = 0; i < m_particles.size(); ) {
        Particle& p = m_particles[i];
        p.lifetime += dt;

        if (p.lifetime >= p.maxLifetime) {
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
            continue;
        }

        // Apply physics
        p.position += p.velocity * dt;
        p.velocity.x *= (1.0f - 2.5f * dt); // Air drag
        p.velocity.y += 120.0f * dt;         // Gentle gravity drift
        p.rotation += p.rotationSpeed * dt;

        ++i;
    }

    // Update screen shake
    if (m_shakeTrauma > 0.0f) {
        m_shakeTimer += dt;
        m_shakeTrauma = std::max(0.0f, m_shakeTrauma - dt * 3.2f);
    } else {
        m_shakeTimer = 0.0f;
    }

    // Update screen flash
    if (m_flashAlpha > 0.0f) {
        m_flashAlpha = std::max(0.0f, m_flashAlpha - dt * 500.0f);
    }
}

void ParticleSystem::emitFlapParticles(sf::Vector2f position) {
    // Emit 8-12 glowing feather and sparkle particles
    int count = 9 + (std::rand() % 4);

    for (int i = 0; i < count; ++i) {
        Particle p;
        // Positioned slightly behind the bird's tail
        p.position = position + sf::Vector2f(-12.0f + randomFloat(-4.0f, 4.0f), randomFloat(-6.0f, 8.0f));

        // Velocity bursts backwards and downward
        float speed = randomFloat(60.0f, 180.0f);
        float angleDeg = randomFloat(130.0f, 210.0f);
        float angleRad = angleDeg * 3.14159265f / 180.0f;
        p.velocity = sf::Vector2f(std::cos(angleRad) * speed, std::sin(angleRad) * speed);

        p.size = randomFloat(3.0f, 6.5f);
        p.lifetime = 0.0f;
        p.maxLifetime = randomFloat(0.35f, 0.65f);
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-180.0f, 180.0f);
        p.isFeather = (i % 2 == 0);

        // Vibrant golden, amber, and crimson tones
        int palette = std::rand() % 4;
        if (palette == 0) {
            p.color = sf::Color(255, 235, 120); // Bright gold
        } else if (palette == 1) {
            p.color = sf::Color(255, 180, 50);  // Warm amber
        } else if (palette == 2) {
            p.color = sf::Color(255, 90, 60);   // Crimson flare
        } else {
            p.color = sf::Color(255, 255, 240); // Radiant white sparkle
        }

        m_particles.push_back(p);
    }
}

void ParticleSystem::emitImpactParticles(sf::Vector2f position) {
    // 25+ fiery and golden spark particles bursting in all directions
    int count = 28;
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position + sf::Vector2f(randomFloat(-10.0f, 10.0f), randomFloat(-10.0f, 10.0f));

        float speed = randomFloat(100.0f, 320.0f);
        float angleRad = randomFloat(0.0f, 6.283185f);
        p.velocity = sf::Vector2f(std::cos(angleRad) * speed, std::sin(angleRad) * speed);

        p.size = randomFloat(3.5f, 7.5f);
        p.lifetime = 0.0f;
        p.maxLifetime = randomFloat(0.45f, 0.85f);
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-360.0f, 360.0f);
        p.isFeather = (i % 3 == 0);

        int palette = std::rand() % 4;
        if (palette == 0) p.color = sf::Color(255, 240, 160);
        else if (palette == 1) p.color = sf::Color(255, 140, 40);
        else if (palette == 2) p.color = sf::Color(240, 50, 50);
        else p.color = sf::Color(255, 255, 255);

        m_particles.push_back(p);
    }
}

void ParticleSystem::triggerShake(float intensity) {
    m_shakeTrauma = std::min(1.0f, m_shakeTrauma + intensity);
}

void ParticleSystem::triggerFlash(float intensity) {
    m_flashAlpha = std::min(220.0f, 220.0f * intensity);
}

sf::Vector2f ParticleSystem::getShakeOffset() const {
    if (m_shakeTrauma <= 0.001f) {
        return sf::Vector2f(0.0f, 0.0f);
    }

    // Non-linear trauma response (trauma^2) feels punchier and crisper
    float shakeAmount = m_shakeTrauma * m_shakeTrauma;
    float maxOffsetX = 14.0f;
    float maxOffsetY = 10.0f;

    float offsetX = std::sin(m_shakeTimer * 52.0f) * maxOffsetX * shakeAmount;
    float offsetY = std::cos(m_shakeTimer * 43.0f) * maxOffsetY * shakeAmount;

    return sf::Vector2f(offsetX, offsetY);
}

void ParticleSystem::draw(sf::RenderWindow& window) {
    sf::CircleShape circle;
    sf::RectangleShape feather;

    for (const auto& p : m_particles) {
        float lifeRatio = p.lifetime / p.maxLifetime;
        float fade = 1.0f - lifeRatio;
        sf::Color c = p.color;
        c.a = static_cast<sf::Uint8>(std::clamp(fade * 255.0f, 0.0f, 255.0f));

        float currentSize = p.size * (1.0f - lifeRatio * 0.4f);

        if (p.isFeather) {
            feather.setSize(sf::Vector2f(currentSize * 2.2f, currentSize * 1.1f));
            feather.setOrigin(currentSize * 1.1f, currentSize * 0.55f);
            feather.setPosition(p.position);
            feather.setRotation(p.rotation);
            feather.setFillColor(c);
            window.draw(feather);
        } else {
            // Sparkle with soft outer glow
            circle.setRadius(currentSize);
            circle.setOrigin(currentSize, currentSize);
            circle.setPosition(p.position);
            circle.setFillColor(c);
            window.draw(circle);

            // Specular core dot
            if (fade > 0.4f) {
                sf::CircleShape core(currentSize * 0.4f);
                core.setOrigin(currentSize * 0.4f, currentSize * 0.4f);
                core.setPosition(p.position);
                core.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(fade * 230.0f)));
                window.draw(core);
            }
        }
    }
}

void ParticleSystem::drawFlash(sf::RenderWindow& window) {
    if (m_flashAlpha > 1.0f) {
        sf::RectangleShape flash(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
        flash.setFillColor(sf::Color(255, 245, 220, static_cast<sf::Uint8>(m_flashAlpha)));
        window.draw(flash);
    }
}

void ParticleSystem::reset() {
    m_particles.clear();
    m_shakeTrauma = 0.0f;
    m_shakeTimer = 0.0f;
    m_flashAlpha = 0.0f;
}
