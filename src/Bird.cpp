#include "Bird.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>

Bird::Bird() {
    reset();
}

void Bird::reset() {
    m_position = sf::Vector2f(Config::BIRD_START_X, Config::BIRD_START_Y);
    m_velocity = 0.0f;
    m_rotation = 0.0f;
    m_targetRotation = 0.0f;
    m_wingCycle = 0.0f;
    m_bobTimer = 0.0f;
    m_isAlive = true;
}

void Bird::flap() {
    if (!m_isAlive) return;

    // Snappy upward impulse
    m_velocity = Config::FLAP_IMPULSE;
    m_targetRotation = Config::TILT_UP_ANGLE;
    m_wingCycle = 0.0f; // Reset wing to begin downstroke
}

void Bird::update(float dt) {
    // Gravity acceleration
    m_velocity += Config::GRAVITY * dt;
    if (m_velocity > Config::MAX_FALL_SPEED) {
        m_velocity = Config::MAX_FALL_SPEED;
    }

    // Move bird
    m_position.y += m_velocity * dt;

    // Prevent flying above ceiling
    if (m_position.y < Config::BIRD_RADIUS) {
        m_position.y = Config::BIRD_RADIUS;
        if (m_velocity < 0.0f) m_velocity = 0.0f;
    }

    // Dynamic rotation:
    // If rising (velocity < 0): tilt up towards -25 deg
    // If falling (velocity >= 0): smoothly pitch down towards +80 deg nosedive
    if (m_velocity < 0.0f) {
        m_targetRotation = Config::TILT_UP_ANGLE;
    } else {
        float fallFraction = std::clamp(m_velocity / 480.0f, 0.0f, 1.0f);
        m_targetRotation = Config::TILT_UP_ANGLE + (Config::TILT_DOWN_ANGLE - Config::TILT_UP_ANGLE) * fallFraction;
    }

    // Smooth lerp rotation
    float rotSpeed = (m_velocity < 0.0f) ? 14.0f : 8.0f;
    m_rotation += (m_targetRotation - m_rotation) * rotSpeed * dt;

    // Wing flapping cycle: flap faster when rising, glide when nosediving
    if (m_velocity < 150.0f) {
        m_wingCycle += dt * 26.0f;
    } else {
        m_wingCycle += dt * 8.0f;
    }
}

void Bird::updateBobbing(float dt) {
    m_bobTimer += dt;
    // Gentle sine wave bobbing in Get Ready state
    m_position.y = Config::BIRD_START_Y + std::sin(m_bobTimer * 4.5f) * 9.0f;
    m_rotation = std::sin(m_bobTimer * 4.5f) * 4.0f; // Subtle breathing angle

    // Gentle steady flutter
    m_wingCycle += dt * 14.0f;
}

float Bird::getHitRadius() const {
    return Config::BIRD_HIT_RADIUS;
}

sf::FloatRect Bird::getBounds() const {
    float r = Config::BIRD_HIT_RADIUS;
    return sf::FloatRect(m_position.x - r, m_position.y - r, r * 2.0f, r * 2.0f);
}

void Bird::draw(sf::RenderWindow& window, CosmeticsManager* cosmetics, float time) {
    sf::Transform transform;
    transform.translate(m_position);
    transform.rotate(m_rotation);

    sf::RenderStates states;
    states.transform = transform;

    // Draw Auras and Outfit Back Layers (Capes, Jetpacks, Wings)
    if (cosmetics) {
        cosmetics->drawAuras(window, m_position, cosmetics->getEquippedOutfit(), cosmetics->getEquippedHat(), time);
        cosmetics->drawOutfit(window, m_position, m_rotation, 1.0f, cosmetics->getEquippedOutfit(), time);
    }

    // Outline color
    const sf::Color outlineColor(40, 22, 12);

    // 1. Crimson Head Feather Crest (behind body)
    sf::ConvexShape crest;
    crest.setPointCount(4);
    crest.setPoint(0, sf::Vector2f(-10.0f, -12.0f));
    crest.setPoint(1, sf::Vector2f(-22.0f, -22.0f));
    crest.setPoint(2, sf::Vector2f(-12.0f, -18.0f));
    crest.setPoint(3, sf::Vector2f(-6.0f, -14.0f));
    crest.setFillColor(sf::Color(225, 45, 45)); // Vibrant crimson
    crest.setOutlineColor(outlineColor);
    crest.setOutlineThickness(1.5f);
    window.draw(crest, states);

    // 2. Main Bird Body: Vibrant Golden-Yellow Oval
    sf::CircleShape body(17.5f);
    body.setScale(1.15f, 0.98f);
    body.setOrigin(17.5f, 17.5f);
    body.setFillColor(sf::Color(255, 205, 30));
    body.setOutlineColor(outlineColor);
    body.setOutlineThickness(2.0f);
    window.draw(body, states);

    // 3. Creamy Underbelly Highlight
    sf::CircleShape belly(13.0f);
    belly.setScale(1.05f, 0.75f);
    belly.setOrigin(13.0f, 13.0f);
    belly.setPosition(2.0f, 5.0f);
    belly.setFillColor(sf::Color(255, 242, 140, 220));
    window.draw(belly, states);

    // 4. Soft Cheek Blush
    sf::CircleShape blush(4.2f);
    blush.setOrigin(4.2f, 4.2f);
    blush.setPosition(6.0f, 3.5f);
    blush.setFillColor(sf::Color(255, 110, 130, 160));
    window.draw(blush, states);

    // 5. Beak: Rich Orange with distinct top & bottom bill
    // Top Bill
    sf::ConvexShape topBeak;
    topBeak.setPointCount(3);
    topBeak.setPoint(0, sf::Vector2f(10.0f, -2.0f));
    topBeak.setPoint(1, sf::Vector2f(25.0f, 2.0f));
    topBeak.setPoint(2, sf::Vector2f(10.0f, 3.5f));
    topBeak.setFillColor(sf::Color(255, 120, 20));
    topBeak.setOutlineColor(outlineColor);
    topBeak.setOutlineThickness(1.8f);
    window.draw(topBeak, states);

    // Bottom Bill
    sf::ConvexShape btmBeak;
    btmBeak.setPointCount(3);
    btmBeak.setPoint(0, sf::Vector2f(10.0f, 3.5f));
    btmBeak.setPoint(1, sf::Vector2f(22.0f, 4.5f));
    btmBeak.setPoint(2, sf::Vector2f(9.0f, 8.0f));
    btmBeak.setFillColor(sf::Color(240, 95, 15));
    btmBeak.setOutlineColor(outlineColor);
    btmBeak.setOutlineThickness(1.8f);
    window.draw(btmBeak, states);

    // 6. Expressive Eye: Large White Sclera + Dark Pupil + Specular Sparkle
    sf::CircleShape eyeWhite(6.5f);
    eyeWhite.setOrigin(6.5f, 6.5f);
    eyeWhite.setPosition(7.0f, -6.5f);
    eyeWhite.setFillColor(sf::Color(255, 255, 255));
    eyeWhite.setOutlineColor(outlineColor);
    eyeWhite.setOutlineThickness(1.8f);
    window.draw(eyeWhite, states);

    // Pupil
    sf::CircleShape eyePupil(3.8f);
    eyePupil.setOrigin(3.8f, 3.8f);
    eyePupil.setPosition(8.8f, -6.5f);
    eyePupil.setFillColor(sf::Color(22, 20, 26));
    window.draw(eyePupil, states);

    // Specular Highlight
    sf::CircleShape eyeShine(1.8f);
    eyeShine.setOrigin(1.8f, 1.8f);
    eyeShine.setPosition(7.8f, -8.0f);
    eyeShine.setFillColor(sf::Color(255, 255, 255));
    window.draw(eyeShine, states);

    // 7. Animated Flapping Wing: Crimson & Golden Highlights
    float wingAngle = std::sin(m_wingCycle) * 32.0f;
    sf::Transform wingTransform = states.transform;
    wingTransform.translate(-4.0f, -1.0f);
    wingTransform.rotate(wingAngle);

    sf::RenderStates wingStates;
    wingStates.transform = wingTransform;

    // Wing Outer Shell (Crimson)
    sf::ConvexShape wing;
    wing.setPointCount(5);
    wing.setPoint(0, sf::Vector2f(0.0f, -4.0f));
    wing.setPoint(1, sf::Vector2f(-16.0f, -8.0f));
    wing.setPoint(2, sf::Vector2f(-20.0f, 0.0f));
    wing.setPoint(3, sf::Vector2f(-12.0f, 8.0f));
    wing.setPoint(4, sf::Vector2f(2.0f, 4.0f));
    wing.setFillColor(sf::Color(230, 50, 45));
    wing.setOutlineColor(outlineColor);
    wing.setOutlineThickness(1.8f);
    window.draw(wing, wingStates);

    // Wing Inner Golden Feathers
    sf::ConvexShape wingInner;
    wingInner.setPointCount(4);
    wingInner.setPoint(0, sf::Vector2f(-2.0f, -2.0f));
    wingInner.setPoint(1, sf::Vector2f(-12.0f, -4.0f));
    wingInner.setPoint(2, sf::Vector2f(-14.0f, 1.0f));
    wingInner.setPoint(3, sf::Vector2f(-4.0f, 3.0f));
    wingInner.setFillColor(sf::Color(255, 215, 65));
    window.draw(wingInner, wingStates);

    // Draw Equipped Hat (Headwear)
    if (cosmetics) {
        cosmetics->drawHat(window, m_position, m_rotation, 1.0f, cosmetics->getEquippedHat(), time);
    }
}
