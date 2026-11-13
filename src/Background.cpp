#include "Background.hpp"
#include "Config.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    inline sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return sf::Color(
            static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
            static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
            static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
            static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
        );
    }
}

Background::Background()
    : m_mountainOffset(0.0f)
    , m_hillOffset(0.0f)
    , m_groundOffset(0.0f)
    , m_time(0.0f) {
    m_skyVertices.setPrimitiveType(sf::TrianglesStrip);
    m_skyVertices.resize(6);

    initMountains();
    initClouds();
    initHills();
}

void Background::initMountains() {
    // Distant mountain & skyline silhouette generated over a width of 1440px (3 screens wide)
    const float patternWidth = 1440.0f;
    const int segments = 72;
    const float segWidth = patternWidth / segments;

    m_mountainVertices.setPrimitiveType(sf::TrianglesStrip);
    m_mountainVertices.resize((segments + 1) * 2);

    float baseY = Config::PLAY_HEIGHT - 60.0f;

    for (int i = 0; i <= segments; ++i) {
        float x = i * segWidth;
        // Blend sine waves + building block shapes
        float wave1 = std::sin(i * 0.35f) * 45.0f;
        float wave2 = std::sin(i * 0.95f) * 25.0f;
        float skylineStepping = (static_cast<int>(i * 1.5f) % 4 == 0) ? -35.0f : 0.0f;
        float y = baseY - 80.0f + wave1 + wave2 + skylineStepping;

        // Top vertex (silhouette apex)
        m_mountainVertices[i * 2].position = sf::Vector2f(x, y);
        m_mountainVertices[i * 2].color = sf::Color(42, 24, 66, 230); // Deep twilight silhouette

        // Bottom vertex (ground intersection)
        m_mountainVertices[i * 2 + 1].position = sf::Vector2f(x, Config::PLAY_HEIGHT);
        m_mountainVertices[i * 2 + 1].color = sf::Color(32, 18, 50, 245);
    }
}

void Background::initHills() {
    // Closer rolling hill layer
    const float patternWidth = 1440.0f;
    const int segments = 60;
    const float segWidth = patternWidth / segments;

    m_hillVertices.setPrimitiveType(sf::TrianglesStrip);
    m_hillVertices.resize((segments + 1) * 2);

    float baseY = Config::PLAY_HEIGHT - 10.0f;

    for (int i = 0; i <= segments; ++i) {
        float x = i * segWidth;
        float wave = std::sin(i * 0.22f) * 35.0f + std::cos(i * 0.55f) * 18.0f;
        float y = baseY - 45.0f + wave;

        m_hillVertices[i * 2].position = sf::Vector2f(x, y);
        m_hillVertices[i * 2].color = sf::Color(65, 30, 75, 235); // Warmer purple foreground hill

        m_hillVertices[i * 2 + 1].position = sf::Vector2f(x, Config::PLAY_HEIGHT);
        m_hillVertices[i * 2 + 1].color = sf::Color(45, 20, 55, 255);
    }
}

void Background::initClouds() {
    m_clouds.clear();
    // 5 drifting soft clouds with varied depths
    m_clouds.push_back({ 50.0f,  110.0f, 28.0f, 1.1f, 140.0f });
    m_clouds.push_back({ 240.0f, 165.0f, 38.0f, 0.85f, 120.0f });
    m_clouds.push_back({ 390.0f,  85.0f, 22.0f, 1.3f, 150.0f });
    m_clouds.push_back({ 140.0f, 210.0f, 32.0f, 0.75f, 110.0f });
    m_clouds.push_back({ 320.0f, 260.0f, 20.0f, 0.95f, 130.0f });
}

void Background::update(float dt, int score) {
    (void)score;
    m_time += dt;

    // Parallax scrolling
    m_mountainOffset += Config::PARALLAX_MOUNTAINS * dt;
    if (m_mountainOffset >= 480.0f) m_mountainOffset -= 480.0f;

    m_hillOffset += Config::PARALLAX_HILLS * dt;
    if (m_hillOffset >= 480.0f) m_hillOffset -= 480.0f;

    m_groundOffset += Config::PARALLAX_GROUND * dt;
    if (m_groundOffset >= 36.0f) m_groundOffset -= 36.0f;

    // Drifting clouds
    for (auto& cloud : m_clouds) {
        cloud.x -= cloud.speed * dt;
        if (cloud.x < -180.0f) {
            cloud.x = Config::WINDOW_WIDTH + 80.0f;
            cloud.y = 70.0f + static_cast<float>(std::rand() % 190);
        }
    }
}

void Background::drawSky(sf::RenderWindow& window, int score) {
    // Dynamic color transition based on score
    // Score 0-15: Sunset Gold & Twilight Purple
    // Score 15-35: Radiant Neon Dusk & Crimson
    // Score 35+: Deep Cosmic Midnight & Starlight Cyan
    float scoreFactor = std::min(1.0f, score / 40.0f);

    sf::Color topSkySunset(24, 18, 64);
    sf::Color midSkySunset(145, 52, 95);
    sf::Color btmSkySunset(255, 130, 48);

    sf::Color topSkyDusk(10, 8, 38);
    sf::Color midSkyDusk(85, 28, 92);
    sf::Color btmSkyDusk(235, 75, 70);

    sf::Color topColor = lerpColor(topSkySunset, topSkyDusk, scoreFactor);
    sf::Color midColor = lerpColor(midSkySunset, midSkyDusk, scoreFactor);
    sf::Color btmColor = lerpColor(btmSkySunset, btmSkyDusk, scoreFactor);

    // 6-vertex strip for smooth multi-stop gradient
    // Top
    m_skyVertices[0].position = sf::Vector2f(0.0f, 0.0f);
    m_skyVertices[0].color = topColor;
    m_skyVertices[1].position = sf::Vector2f(Config::WINDOW_WIDTH, 0.0f);
    m_skyVertices[1].color = topColor;

    // Mid
    m_skyVertices[2].position = sf::Vector2f(0.0f, Config::PLAY_HEIGHT * 0.52f);
    m_skyVertices[2].color = midColor;
    m_skyVertices[3].position = sf::Vector2f(Config::WINDOW_WIDTH, Config::PLAY_HEIGHT * 0.52f);
    m_skyVertices[3].color = midColor;

    // Bottom (Horizon)
    m_skyVertices[4].position = sf::Vector2f(0.0f, Config::PLAY_HEIGHT);
    m_skyVertices[4].color = btmColor;
    m_skyVertices[5].position = sf::Vector2f(Config::WINDOW_WIDTH, Config::PLAY_HEIGHT);
    m_skyVertices[5].color = btmColor;

    window.draw(m_skyVertices);

    // Radiant Sunset / Twilight Sun
    float sunX = Config::WINDOW_WIDTH * 0.72f;
    float sunY = Config::PLAY_HEIGHT * 0.68f + std::sin(m_time * 0.5f) * 4.0f;

    // Outer glow rings
    sf::CircleShape sunGlow(65.0f);
    sunGlow.setOrigin(65.0f, 65.0f);
    sunGlow.setPosition(sunX, sunY);
    sunGlow.setFillColor(sf::Color(255, 175, 60, 45));
    window.draw(sunGlow);

    sunGlow.setRadius(48.0f);
    sunGlow.setOrigin(48.0f, 48.0f);
    sunGlow.setFillColor(sf::Color(255, 205, 90, 75));
    window.draw(sunGlow);

    // Core sun disc
    sf::CircleShape sunCore(32.0f);
    sunCore.setOrigin(32.0f, 32.0f);
    sunCore.setPosition(sunX, sunY);
    sunCore.setFillColor(sf::Color(255, 245, 180, 225));
    window.draw(sunCore);
}

void Background::drawParallax(sf::RenderWindow& window) {
    // 1. Draw distant mountains
    sf::RenderStates mountainStates;
    mountainStates.transform.translate(-m_mountainOffset, 0.0f);
    window.draw(m_mountainVertices, mountainStates);

    // 2. Draw drifting translucent clouds
    for (const auto& cloud : m_clouds) {
        float alpha = cloud.alpha;
        sf::Color cloudColor(255, 228, 215, static_cast<sf::Uint8>(alpha));

        float s = cloud.scale;
        // Puffy cloud cluster composed of 4 overlapping circles
        float circles[4][3] = {
            { 0.0f, 0.0f, 24.0f * s },
            { 22.0f * s, -6.0f * s, 20.0f * s },
            { -20.0f * s, 2.0f * s, 18.0f * s },
            { 38.0f * s, 4.0f * s, 16.0f * s }
        };

        for (int i = 0; i < 4; ++i) {
            sf::CircleShape c(circles[i][2]);
            c.setOrigin(circles[i][2], circles[i][2]);
            c.setPosition(cloud.x + circles[i][0], cloud.y + circles[i][1]);
            c.setFillColor(cloudColor);
            window.draw(c);
        }
    }

    // 3. Draw closer hills
    sf::RenderStates hillStates;
    hillStates.transform.translate(-m_hillOffset, 0.0f);
    window.draw(m_hillVertices, hillStates);
}

void Background::drawGround(sf::RenderWindow& window) {
    float groundY = Config::PLAY_HEIGHT;
    float groundH = Config::GROUND_HEIGHT;

    // 1. Earthy soil base rectangle
    sf::RectangleShape soil(sf::Vector2f(Config::WINDOW_WIDTH, groundH));
    soil.setPosition(0.0f, groundY);
    soil.setFillColor(sf::Color(215, 155, 80)); // Warm golden caramel
    window.draw(soil);

    // 2. Lower rich earth stratum
    sf::RectangleShape subsoil(sf::Vector2f(Config::WINDOW_WIDTH, groundH - 24.0f));
    subsoil.setPosition(0.0f, groundY + 24.0f);
    subsoil.setFillColor(sf::Color(165, 105, 50));
    window.draw(subsoil);

    // 3. Fast-scrolling diagonal soil dashes / pebbles
    const float tileWidth = 36.0f;
    int tileCount = static_cast<int>(Config::WINDOW_WIDTH / tileWidth) + 3;

    sf::ConvexShape stripe;
    stripe.setPointCount(4);
    stripe.setFillColor(sf::Color(135, 80, 35, 180));

    for (int i = -1; i < tileCount; ++i) {
        float x = i * tileWidth - m_groundOffset;
        stripe.setPoint(0, sf::Vector2f(x + 10.0f, groundY + 28.0f));
        stripe.setPoint(1, sf::Vector2f(x + 22.0f, groundY + 28.0f));
        stripe.setPoint(2, sf::Vector2f(x + 12.0f, groundY + 52.0f));
        stripe.setPoint(3, sf::Vector2f(x, groundY + 52.0f));
        window.draw(stripe);

        // Lower pebble dot
        sf::CircleShape pebble(2.5f);
        pebble.setFillColor(sf::Color(115, 65, 25, 200));
        pebble.setPosition(x + 16.0f, groundY + 68.0f);
        window.draw(pebble);
    }

    // 4. Lush top grass strip with vibrant neon-lime border
    sf::RectangleShape grass(sf::Vector2f(Config::WINDOW_WIDTH, 14.0f));
    grass.setPosition(0.0f, groundY);
    grass.setFillColor(sf::Color(85, 215, 65)); // Vibrant emerald lime
    window.draw(grass);

    // Top highlight rim on grass
    sf::RectangleShape grassRim(sf::Vector2f(Config::WINDOW_WIDTH, 3.5f));
    grassRim.setPosition(0.0f, groundY);
    grassRim.setFillColor(sf::Color(175, 255, 120)); // Glossy yellow-green rim
    window.draw(grassRim);

    // Serrated blade accents along grass border
    sf::ConvexShape blade;
    blade.setPointCount(3);
    blade.setFillColor(sf::Color(65, 180, 50));

    for (int i = -1; i < tileCount * 2; ++i) {
        float bx = i * 18.0f - m_groundOffset;
        blade.setPoint(0, sf::Vector2f(bx, groundY + 14.0f));
        blade.setPoint(1, sf::Vector2f(bx + 9.0f, groundY + 14.0f));
        blade.setPoint(2, sf::Vector2f(bx + 4.5f, groundY + 20.0f));
        window.draw(blade);
    }
}

void Background::reset() {
    m_mountainOffset = 0.0f;
    m_hillOffset = 0.0f;
    m_groundOffset = 0.0f;
    m_time = 0.0f;
}
