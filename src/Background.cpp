#include "Background.hpp"
#include "Config.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    constexpr float PI = 3.14159265358979323846f;

    sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
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
    : m_screenHeight(Config::DEFAULT_PLAY_HEIGHT + Config::GROUND_HEIGHT)
    , m_time(0.0f)
    , m_bgOffset(0.0f)
    , m_midOffset(0.0f)
    , m_fgOffset(0.0f)
    , m_groundOffset(0.0f) {
    initDecorations();
}

void Background::reset() {
    m_time = 0.0f;
    m_bgOffset = 0.0f;
    m_midOffset = 0.0f;
    m_fgOffset = 0.0f;
    m_groundOffset = 0.0f;
    initDecorations();
}

void Background::initDecorations() {
    m_decorations.clear();

    // Spawn 10 decorative items for each of the 10 stages (100 total items)
    for (int stg = 0; stg < 10; ++stg) {
        for (int item = 0; item < 10; ++item) {
            DecItem d;
            d.stage = stg;
            d.itemType = item;
            d.x = static_cast<float>((item * 65 + (stg * 37)) % static_cast<int>(Config::WINDOW_WIDTH + 180)) - 50.0f;
            d.y = 80.0f + (item * 48.0f + stg * 17.0f);
            d.speed = 18.0f + (item % 5) * 8.0f;
            d.scale = 0.85f + (item % 4) * 0.15f;
            d.alpha = 180.0f + (item % 3) * 35.0f;
            d.param = static_cast<float>(item) * 0.7f;
            m_decorations.push_back(d);
        }
    }
}

void Background::update(float dt, int score) {
    m_time += dt;

    float currentSpeed = Config::getCurrentPipeSpeed(score);
    float speedRatio = currentSpeed / Config::BASE_PIPE_SPEED;

    m_bgOffset += 20.0f * speedRatio * dt;
    m_midOffset += 55.0f * speedRatio * dt;
    m_fgOffset += 95.0f * speedRatio * dt;
    m_groundOffset += currentSpeed * dt;

    if (m_bgOffset > Config::WINDOW_WIDTH) m_bgOffset -= Config::WINDOW_WIDTH;
    if (m_midOffset > Config::WINDOW_WIDTH) m_midOffset -= Config::WINDOW_WIDTH;
    if (m_fgOffset > Config::WINDOW_WIDTH) m_fgOffset -= Config::WINDOW_WIDTH;
    if (m_groundOffset > 24.0f) m_groundOffset = std::fmod(m_groundOffset, 24.0f);

    // Update position of decorative items
    for (auto& item : m_decorations) {
        item.x -= item.speed * speedRatio * dt;
        if (item.x < -140.0f) {
            item.x = Config::WINDOW_WIDTH + 80.0f;
            // Re-randomize height within fair midground bounds
            float groundY = m_screenHeight - Config::GROUND_HEIGHT;
            item.y = 65.0f + static_cast<float>((std::rand() % static_cast<int>(std::max(100.0f, groundY - 140.0f))));
        }
    }
}

void Background::drawSky(sf::RenderWindow& window, int score) {
    int stage = Config::getStageIndex(score);
    float groundY = m_screenHeight - Config::GROUND_HEIGHT;

    // Stage sky gradients (Top Color, Horizon Color)
    sf::Color topColor, btmColor;
    switch (stage) {
        case 0: // Sunset Arcade
            topColor = sf::Color(35, 20, 65);
            btmColor = sf::Color(255, 140, 50);
            break;
        case 1: // Emerald Forest
            topColor = sf::Color(15, 45, 50);
            btmColor = sf::Color(45, 180, 110);
            break;
        case 2: // Prehistoric Dino
            topColor = sf::Color(55, 20, 25);
            btmColor = sf::Color(230, 85, 30);
            break;
        case 3: // Cyberpunk
            topColor = sf::Color(25, 10, 45);
            btmColor = sf::Color(210, 30, 140);
            break;
        case 4: // Space
            topColor = sf::Color(5, 5, 12);
            btmColor = sf::Color(30, 25, 65);
            break;
        case 5: // Universe
            topColor = sf::Color(10, 5, 35);
            btmColor = sf::Color(85, 35, 150);
            break;
        case 6: // Xianxia
            topColor = sf::Color(65, 50, 110);
            btmColor = sf::Color(245, 215, 145);
            break;
        case 7: // Lord of the Mysteries
            topColor = sf::Color(25, 25, 30);
            btmColor = sf::Color(165, 25, 40); // Crimson fog horizon
            break;
        case 8: // Astral Rift
            topColor = sf::Color(15, 15, 40);
            btmColor = sf::Color(60, 190, 235);
            break;
        case 9: // Fool's Apotheosis
            topColor = sf::Color(55, 25, 95);
            btmColor = sf::Color(255, 225, 110);
            break;
    }

    sf::VertexArray sky(sf::TriangleStrip, 4);
    sky[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), topColor);
    sky[1] = sf::Vertex(sf::Vector2f(Config::WINDOW_WIDTH, 0.0f), topColor);
    sky[2] = sf::Vertex(sf::Vector2f(0.0f, groundY), btmColor);
    sky[3] = sf::Vertex(sf::Vector2f(Config::WINDOW_WIDTH, groundY), btmColor);
    window.draw(sky);
}

void Background::drawParallax(sf::RenderWindow& window, int score) {
    int stage = Config::getStageIndex(score);
    float groundY = m_screenHeight - Config::GROUND_HEIGHT;

    // Distant mountain/spire ridges
    sf::Color ridgeColor(40, 20, 50, 140);
    switch (stage) {
        case 1: ridgeColor = sf::Color(20, 60, 40, 150); break;
        case 2: ridgeColor = sf::Color(60, 25, 20, 160); break;
        case 3: ridgeColor = sf::Color(30, 15, 60, 170); break;
        case 4: ridgeColor = sf::Color(20, 15, 35, 120); break;
        case 5: ridgeColor = sf::Color(35, 15, 65, 140); break;
        case 6: ridgeColor = sf::Color(80, 70, 110, 130); break;
        case 7: ridgeColor = sf::Color(45, 25, 30, 160); break;
        case 8: ridgeColor = sf::Color(25, 30, 60, 150); break;
        case 9: ridgeColor = sf::Color(90, 45, 120, 150); break;
    }

    // Procedural distant ridges
    sf::VertexArray mountains(sf::TriangleStrip, 12);
    for (int i = 0; i < 6; ++i) {
        float x = (i * 100.0f) - m_bgOffset * 0.4f;
        while (x < -100.0f) x += 600.0f;
        float peakY = groundY - 130.0f - std::sin(i * 1.4f) * 35.0f;
        mountains[i * 2] = sf::Vertex(sf::Vector2f(x, peakY), ridgeColor);
        mountains[i * 2 + 1] = sf::Vertex(sf::Vector2f(x, groundY), sf::Color(ridgeColor.r, ridgeColor.g, ridgeColor.b, 60));
    }
    window.draw(mountains);

    // Draw the 10 themed decorative items for this stage
    drawDecorations(window, stage);
}

void Background::drawDecorations(sf::RenderWindow& window, int stage) {
    for (const auto& item : m_decorations) {
        if (item.stage == stage) {
            drawSingleDecItem(window, item, m_time);
        }
    }
}

void Background::drawSingleDecItem(sf::RenderWindow& window, const DecItem& item, float time) {
    sf::Transform t;
    t.translate(item.x, item.y);
    t.scale(item.scale, item.scale);

    switch (item.stage) {
        case 0: { // Classic Sunset (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Setting Sun Disc
                    sf::CircleShape sun(22.0f);
                    sun.setOrigin(22.0f, 22.0f);
                    sun.setFillColor(sf::Color(255, 225, 110, 200));
                    window.draw(sun, t);
                    break;
                }
                case 1: { // 2. Distant Sunbeams
                    sf::Vertex beam[3] = {
                        sf::Vertex(sf::Vector2f(0.0f, -10.0f), sf::Color(255, 230, 140, 80)),
                        sf::Vertex(sf::Vector2f(-20.0f, 40.0f), sf::Color(255, 180, 50, 0)),
                        sf::Vertex(sf::Vector2f(20.0f, 40.0f), sf::Color(255, 180, 50, 0))
                    };
                    window.draw(beam, 3, sf::Triangles, t);
                    break;
                }
                case 2: { // 3. Warm Cumulus Cloud
                    sf::CircleShape puff(14.0f);
                    puff.setFillColor(sf::Color(255, 230, 210, 160));
                    puff.setPosition(-10.0f, 0.0f); window.draw(puff, t);
                    puff.setPosition(6.0f, -4.0f);  window.draw(puff, t);
                    puff.setPosition(18.0f, 2.0f);  window.draw(puff, t);
                    break;
                }
                case 3: { // 4. Sunset Cirrus Streak
                    sf::RectangleShape streak(sf::Vector2f(44.0f, 3.0f));
                    streak.setFillColor(sf::Color(255, 190, 120, 140));
                    window.draw(streak, t);
                    break;
                }
                case 4: { // 5. Silhouette Mesa
                    sf::ConvexShape mesa;
                    mesa.setPointCount(4);
                    mesa.setPoint(0, sf::Vector2f(-25.0f, 20.0f));
                    mesa.setPoint(1, sf::Vector2f(-12.0f, 0.0f));
                    mesa.setPoint(2, sf::Vector2f(12.0f, 0.0f));
                    mesa.setPoint(3, sf::Vector2f(25.0f, 20.0f));
                    mesa.setFillColor(sf::Color(45, 25, 60, 170));
                    window.draw(mesa, t);
                    break;
                }
                case 5: { // 6. Distant Palm Ridge
                    sf::CircleShape hill(28.0f);
                    hill.setScale(1.6f, 0.7f);
                    hill.setFillColor(sf::Color(55, 30, 75, 150));
                    window.draw(hill, t);
                    break;
                }
                case 6: { // 7. Amber Horizon Haze
                    sf::RectangleShape haze(sf::Vector2f(60.0f, 6.0f));
                    haze.setFillColor(sf::Color(255, 160, 50, 90));
                    window.draw(haze, t);
                    break;
                }
                case 7: { // 8. Flocking Silhouetted Birds
                    float flap = std::sin(time * 12.0f + item.param) * 3.0f;
                    sf::Vertex vbird[3] = {
                        sf::Vertex(sf::Vector2f(-6.0f, flap), sf::Color(30, 20, 40, 200)),
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(30, 20, 40, 200)),
                        sf::Vertex(sf::Vector2f(6.0f, flap), sf::Color(30, 20, 40, 200))
                    };
                    window.draw(vbird, 3, sf::LineStrip, t);
                    break;
                }
                case 8: { // 9. Retro Arcade Grid Line
                    sf::RectangleShape line(sf::Vector2f(50.0f, 1.5f));
                    line.setFillColor(sf::Color(255, 120, 160, 110));
                    window.draw(line, t);
                    break;
                }
                case 9: { // 10. Grassy Turf Blades
                    sf::Vertex grass[3] = {
                        sf::Vertex(sf::Vector2f(0.0f, 8.0f), sf::Color(40, 160, 60, 180)),
                        sf::Vertex(sf::Vector2f(3.0f, 0.0f), sf::Color(80, 210, 80, 180)),
                        sf::Vertex(sf::Vector2f(6.0f, 8.0f), sf::Color(40, 160, 60, 180))
                    };
                    window.draw(grass, 3, sf::Triangles, t);
                    break;
                }
            }
            break;
        }
        case 1: { // Forest (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Tall Pine Silhouette
                    sf::ConvexShape pine;
                    pine.setPointCount(3);
                    pine.setPoint(0, sf::Vector2f(-10.0f, 30.0f));
                    pine.setPoint(1, sf::Vector2f(0.0f, -10.0f));
                    pine.setPoint(2, sf::Vector2f(10.0f, 30.0f));
                    pine.setFillColor(sf::Color(20, 55, 30, 180));
                    window.draw(pine, t);
                    break;
                }
                case 1: { // 2. Giant Redwood Trunk
                    sf::RectangleShape trunk(sf::Vector2f(18.0f, 45.0f));
                    trunk.setFillColor(sf::Color(65, 40, 25, 160));
                    window.draw(trunk, t);
                    break;
                }
                case 2: { // 3. Hanging Moss Vine
                    float sway = std::sin(time * 3.0f + item.param) * 5.0f;
                    sf::Vertex vine[3] = {
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(40, 110, 45)),
                        sf::Vertex(sf::Vector2f(sway, 15.0f), sf::Color(60, 140, 55)),
                        sf::Vertex(sf::Vector2f(sway * 1.5f, 30.0f), sf::Color(35, 95, 35))
                    };
                    window.draw(vine, 3, sf::LineStrip, t);
                    break;
                }
                case 3: { // 4. Glowing Forest Firefly
                    float pulse = (std::sin(time * 8.0f + item.param) + 1.0f) * 0.5f;
                    sf::CircleShape bug(3.0f);
                    bug.setFillColor(sf::Color(190, 255, 60, static_cast<sf::Uint8>(120 + pulse * 135)));
                    window.draw(bug, t);
                    break;
                }
                case 4: { // 5. Fern Cluster
                    sf::CircleShape fern(8.0f);
                    fern.setScale(1.5f, 0.6f);
                    fern.setFillColor(sf::Color(50, 160, 70, 190));
                    window.draw(fern, t);
                    break;
                }
                case 5: { // 6. Wild Mushroom Patch
                    sf::CircleShape cap(6.0f);
                    cap.setFillColor(sf::Color(220, 50, 40));
                    window.draw(cap, t);
                    break;
                }
                case 6: { // 7. Dappled Sun Shaft
                    sf::Vertex ray[3] = {
                        sf::Vertex(sf::Vector2f(-5.0f, -10.0f), sf::Color(255, 245, 160, 70)),
                        sf::Vertex(sf::Vector2f(-20.0f, 40.0f), sf::Color(255, 230, 120, 0)),
                        sf::Vertex(sf::Vector2f(10.0f, 40.0f), sf::Color(255, 230, 120, 0))
                    };
                    window.draw(ray, 3, sf::Triangles, t);
                    break;
                }
                case 7: { // 8. Bushy Shrub
                    sf::CircleShape shrub(12.0f);
                    shrub.setFillColor(sf::Color(30, 95, 45, 170));
                    window.draw(shrub, t);
                    break;
                }
                case 8: { // 9. Fallen Leaf Drift
                    float rot = time * 4.0f + item.param;
                    sf::ConvexShape leaf;
                    leaf.setPointCount(4);
                    leaf.setPoint(0, sf::Vector2f(0.0f, -4.0f));
                    leaf.setPoint(1, sf::Vector2f(4.0f, 0.0f));
                    leaf.setPoint(2, sf::Vector2f(0.0f, 4.0f));
                    leaf.setPoint(3, sf::Vector2f(-4.0f, 0.0f));
                    leaf.setFillColor(sf::Color(210, 120, 30, 200));
                    leaf.setRotation(rot * 50.0f);
                    window.draw(leaf, t);
                    break;
                }
                case 9: { // 10. Woodland Mist Layer
                    sf::RectangleShape mist(sf::Vector2f(60.0f, 10.0f));
                    mist.setFillColor(sf::Color(180, 225, 200, 60));
                    window.draw(mist, t);
                    break;
                }
            }
            break;
        }
        case 2: { // Dino: Volcano & Prehistoric Valley (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Smoking Volcano Peak
                    sf::ConvexShape vol;
                    vol.setPointCount(4);
                    vol.setPoint(0, sf::Vector2f(-30.0f, 30.0f));
                    vol.setPoint(1, sf::Vector2f(-6.0f, 0.0f));
                    vol.setPoint(2, sf::Vector2f(6.0f, 0.0f));
                    vol.setPoint(3, sf::Vector2f(30.0f, 30.0f));
                    vol.setFillColor(sf::Color(55, 30, 35, 190));
                    window.draw(vol, t);
                    break;
                }
                case 1: { // 2. Lava Flow Streak
                    sf::Vertex lava[2] = {
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(255, 90, 20, 220)),
                        sf::Vertex(sf::Vector2f(8.0f, 20.0f), sf::Color(255, 200, 40, 220))
                    };
                    window.draw(lava, 2, sf::Lines, t);
                    break;
                }
                case 2: { // 3. Cycad Palm Frond
                    sf::CircleShape frond(10.0f);
                    frond.setScale(1.4f, 0.5f);
                    frond.setFillColor(sf::Color(80, 120, 40, 170));
                    window.draw(frond, t);
                    break;
                }
                case 3: { // 4. Brontosaurus Silhouette
                    sf::CircleShape body(12.0f);
                    body.setFillColor(sf::Color(40, 25, 30, 180));
                    window.draw(body, t);
                    sf::RectangleShape neck(sf::Vector2f(4.0f, 18.0f));
                    neck.setRotation(-25.0f);
                    neck.setFillColor(sf::Color(40, 25, 30, 180));
                    window.draw(neck, t);
                    break;
                }
                case 4: { // 5. Pterodactyl Glider
                    float flap = std::sin(time * 6.0f + item.param) * 3.0f;
                    sf::Vertex wing[3] = {
                        sf::Vertex(sf::Vector2f(-12.0f, flap), sf::Color(45, 25, 25, 200)),
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(45, 25, 25, 200)),
                        sf::Vertex(sf::Vector2f(12.0f, flap), sf::Color(45, 25, 25, 200))
                    };
                    window.draw(wing, 3, sf::LineStrip, t);
                    break;
                }
                case 5: { // 6. Giant Fern Tree
                    sf::RectangleShape trunk(sf::Vector2f(6.0f, 26.0f));
                    trunk.setFillColor(sf::Color(55, 35, 25, 170));
                    window.draw(trunk, t);
                    break;
                }
                case 6: { // 7. Amber Fossil Crystal
                    sf::CircleShape crystal(5.0f, 6);
                    crystal.setFillColor(sf::Color(255, 180, 30, 210));
                    window.draw(crystal, t);
                    break;
                }
                case 7: { // 8. Cracked Basalt Boulder
                    sf::CircleShape boulder(14.0f, 5);
                    boulder.setFillColor(sf::Color(40, 35, 45, 190));
                    window.draw(boulder, t);
                    break;
                }
                case 8: { // 9. Ash Ember Particle
                    float rise = std::fmod(time * 20.0f + item.param * 10.0f, 25.0f);
                    sf::CircleShape spark(2.0f);
                    spark.setPosition(0.0f, -rise);
                    spark.setFillColor(sf::Color(255, 120, 30, 230));
                    window.draw(spark, t);
                    break;
                }
                case 9: { // 10. Tar Pit Ripple
                    sf::CircleShape tar(16.0f);
                    tar.setScale(1.8f, 0.4f);
                    tar.setFillColor(sf::Color(20, 15, 25, 200));
                    window.draw(tar, t);
                    break;
                }
            }
            break;
        }
        case 3: { // Cyberpunk: Neon City Skyline (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Megastructure Spire
                    sf::RectangleShape tower(sf::Vector2f(24.0f, 75.0f));
                    tower.setFillColor(sf::Color(25, 20, 40, 180));
                    window.draw(tower, t);
                    break;
                }
                case 1: { // 2. Neon Signboard Signifier
                    sf::RectangleShape sign(sf::Vector2f(12.0f, 24.0f));
                    sign.setFillColor(sf::Color(255, 20, 140, 220));
                    window.draw(sign, t);
                    break;
                }
                case 2: { // 3. Hovering Spinner / Drone
                    sf::RectangleShape drone(sf::Vector2f(16.0f, 5.0f));
                    drone.setFillColor(sf::Color(0, 240, 255, 230));
                    window.draw(drone, t);
                    break;
                }
                case 3: { // 4. Hologram Billboard
                    sf::RectangleShape board(sf::Vector2f(28.0f, 16.0f));
                    board.setFillColor(sf::Color(0, 255, 200, 120));
                    window.draw(board, t);
                    break;
                }
                case 4: { // 5. Rain Streak Line
                    sf::RectangleShape rain(sf::Vector2f(1.5f, 18.0f));
                    rain.setRotation(15.0f);
                    rain.setFillColor(sf::Color(100, 220, 255, 130));
                    window.draw(rain, t);
                    break;
                }
                case 5: { // 6. Rooftop Antenna Tower
                    sf::RectangleShape mast(sf::Vector2f(2.0f, 30.0f));
                    mast.setFillColor(sf::Color(200, 40, 90, 190));
                    window.draw(mast, t);
                    break;
                }
                case 6: { // 7. Cyber Grid Horizon
                    sf::RectangleShape grid(sf::Vector2f(60.0f, 1.5f));
                    grid.setFillColor(sf::Color(255, 50, 180, 100));
                    window.draw(grid, t);
                    break;
                }
                case 7: { // 8. Glowing Conduit Pipe
                    sf::RectangleShape conduit(sf::Vector2f(35.0f, 3.5f));
                    conduit.setFillColor(sf::Color(0, 255, 255, 160));
                    window.draw(conduit, t);
                    break;
                }
                case 8: { // 9. Streetlamp Beacon
                    sf::CircleShape lamp(4.0f);
                    lamp.setFillColor(sf::Color(255, 230, 90, 210));
                    window.draw(lamp, t);
                    break;
                }
                case 9: { // 10. Digital Scanline Band
                    sf::RectangleShape band(sf::Vector2f(80.0f, 2.0f));
                    band.setFillColor(sf::Color(0, 255, 255, 70));
                    window.draw(band, t);
                    break;
                }
            }
            break;
        }
        case 4: { // Space: Meteors & Orbit (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Twinkling Starfield
                    sf::CircleShape star(2.0f);
                    float tw = (std::sin(time * 5.0f + item.param) + 1.0f) * 0.5f;
                    star.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(100 + tw * 155)));
                    window.draw(star, t);
                    break;
                }
                case 1: { // 2. Crescent Moon
                    sf::CircleShape moon(14.0f);
                    moon.setFillColor(sf::Color(245, 245, 220, 180));
                    window.draw(moon, t);
                    break;
                }
                case 2: { // 3. Ringed Gas Planet
                    sf::CircleShape planet(12.0f);
                    planet.setFillColor(sf::Color(180, 140, 100, 200));
                    window.draw(planet, t);
                    sf::CircleShape ring(18.0f);
                    ring.setScale(1.0f, 0.35f);
                    ring.setFillColor(sf::Color::Transparent);
                    ring.setOutlineThickness(2.0f);
                    ring.setOutlineColor(sf::Color(210, 180, 140, 160));
                    window.draw(ring, t);
                    break;
                }
                case 3: { // 4. Distant Nebula Cloud
                    sf::CircleShape nebula(25.0f);
                    nebula.setFillColor(sf::Color(130, 40, 160, 60));
                    window.draw(nebula, t);
                    break;
                }
                case 4: { // 5. Shooting Star Streak
                    sf::RectangleShape streak(sf::Vector2f(35.0f, 2.0f));
                    streak.setRotation(-30.0f);
                    streak.setFillColor(sf::Color(255, 255, 255, 210));
                    window.draw(streak, t);
                    break;
                }
                case 5: { // 6. Orbital Satellite
                    sf::RectangleShape sat(sf::Vector2f(8.0f, 6.0f));
                    sat.setFillColor(sf::Color(190, 210, 230, 200));
                    window.draw(sat, t);
                    break;
                }
                case 6: { // 7. Asteroid Belt Pebbles
                    sf::CircleShape pebble(2.5f, 5);
                    pebble.setFillColor(sf::Color(130, 115, 120, 180));
                    window.draw(pebble, t);
                    break;
                }
                case 7: { // 8. Solar Flare Glow
                    sf::CircleShape flare(20.0f);
                    flare.setFillColor(sf::Color(255, 140, 20, 50));
                    window.draw(flare, t);
                    break;
                }
                case 8: { // 9. Planetary Horizon Arc
                    sf::CircleShape arc(50.0f);
                    arc.setFillColor(sf::Color::Transparent);
                    arc.setOutlineThickness(3.0f);
                    arc.setOutlineColor(sf::Color(100, 180, 255, 90));
                    window.draw(arc, t);
                    break;
                }
                case 9: { // 10. Cosmic Dust Specks
                    sf::CircleShape dust(1.5f);
                    dust.setFillColor(sf::Color(220, 240, 255, 150));
                    window.draw(dust, t);
                    break;
                }
            }
            break;
        }
        case 5: { // Universe: Multiverse & Deep Space (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Spiral Galaxy Core
                    sf::CircleShape core(8.0f);
                    core.setFillColor(sf::Color(255, 250, 220, 220));
                    window.draw(core, t);
                    break;
                }
                case 1: { // 2. Quasar Light Beam
                    sf::RectangleShape beam(sf::Vector2f(50.0f, 2.0f));
                    beam.setRotation(45.0f);
                    beam.setFillColor(sf::Color(140, 230, 255, 180));
                    window.draw(beam, t);
                    break;
                }
                case 2: { // 3. Cosmic Web Filament
                    sf::Vertex filament[3] = {
                        sf::Vertex(sf::Vector2f(-15.0f, 0.0f), sf::Color(160, 70, 240, 120)),
                        sf::Vertex(sf::Vector2f(0.0f, 15.0f), sf::Color(120, 100, 255, 120)),
                        sf::Vertex(sf::Vector2f(20.0f, -5.0f), sf::Color(160, 70, 240, 120))
                    };
                    window.draw(filament, 3, sf::LineStrip, t);
                    break;
                }
                case 3: { // 4. Supernova Remnant
                    sf::CircleShape nova(18.0f);
                    nova.setFillColor(sf::Color::Transparent);
                    nova.setOutlineThickness(2.0f);
                    nova.setOutlineColor(sf::Color(255, 90, 180, 140));
                    window.draw(nova, t);
                    break;
                }
                case 4: { // 5. Binary Star Pair
                    sf::CircleShape starA(3.0f);
                    starA.setFillColor(sf::Color(100, 200, 255));
                    starA.setPosition(-5.0f, 0.0f); window.draw(starA, t);
                    sf::CircleShape starB(3.0f);
                    starB.setFillColor(sf::Color(255, 180, 80));
                    starB.setPosition(5.0f, 0.0f); window.draw(starB, t);
                    break;
                }
                case 5: { // 6. Dark Matter Cloud
                    sf::CircleShape dm(22.0f);
                    dm.setFillColor(sf::Color(20, 10, 40, 90));
                    window.draw(dm, t);
                    break;
                }
                case 6: { // 7. Gravitational Lens Ring
                    sf::CircleShape lens(14.0f);
                    lens.setFillColor(sf::Color::Transparent);
                    lens.setOutlineThickness(1.8f);
                    lens.setOutlineColor(sf::Color(255, 235, 150, 160));
                    window.draw(lens, t);
                    break;
                }
                case 7: { // 8. Star Cluster Sparkle
                    sf::CircleShape cluster(5.0f, 4);
                    cluster.setFillColor(sf::Color(240, 240, 255, 210));
                    window.draw(cluster, t);
                    break;
                }
                case 8: { // 9. Deep Void Aurora
                    sf::Vertex aurora[3] = {
                        sf::Vertex(sf::Vector2f(-20.0f, 0.0f), sf::Color(70, 220, 190, 70)),
                        sf::Vertex(sf::Vector2f(0.0f, 8.0f), sf::Color(160, 80, 255, 90)),
                        sf::Vertex(sf::Vector2f(20.0f, 0.0f), sf::Color(70, 220, 190, 70))
                    };
                    window.draw(aurora, 3, sf::LineStrip, t);
                    break;
                }
                case 9: { // 10. Pulsar Beacon
                    float rot = time * 80.0f;
                    sf::RectangleShape pulse(sf::Vector2f(24.0f, 2.0f));
                    pulse.setOrigin(12.0f, 1.0f);
                    pulse.setRotation(rot);
                    pulse.setFillColor(sf::Color(255, 255, 255, 190));
                    window.draw(pulse, t);
                    break;
                }
            }
            break;
        }
        case 6: { // Xianxia: Spirit Mountains & Flying Swords (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Floating Spirit Peak
                    sf::ConvexShape peak;
                    peak.setPointCount(3);
                    peak.setPoint(0, sf::Vector2f(-18.0f, 0.0f));
                    peak.setPoint(1, sf::Vector2f(0.0f, 24.0f));
                    peak.setPoint(2, sf::Vector2f(18.0f, 0.0f));
                    peak.setFillColor(sf::Color(65, 85, 95, 180));
                    window.draw(peak, t);
                    break;
                }
                case 1: { // 2. Auspicious Immortal Cloud
                    sf::CircleShape puff(10.0f);
                    puff.setFillColor(sf::Color(245, 235, 255, 170));
                    puff.setPosition(-8.0f, 0.0f); window.draw(puff, t);
                    puff.setPosition(4.0f, -2.0f); window.draw(puff, t);
                    break;
                }
                case 2: { // 3. Flying Sword Silhouette
                    sf::RectangleShape sword(sf::Vector2f(20.0f, 3.0f));
                    sword.setRotation(-15.0f);
                    sword.setFillColor(sf::Color(220, 245, 255, 220));
                    window.draw(sword, t);
                    break;
                }
                case 3: { // 4. Spirit Lotus Bloom
                    sf::CircleShape lotus(6.0f, 6);
                    lotus.setFillColor(sf::Color(255, 170, 210, 200));
                    window.draw(lotus, t);
                    break;
                }
                case 4: { // 5. Ancient Pagoda Tier
                    sf::RectangleShape tier(sf::Vector2f(16.0f, 8.0f));
                    tier.setFillColor(sf::Color(100, 60, 45, 190));
                    window.draw(tier, t);
                    break;
                }
                case 5: { // 6. Qi Waterfall Mist
                    sf::RectangleShape mist(sf::Vector2f(6.0f, 28.0f));
                    mist.setFillColor(sf::Color(200, 240, 255, 100));
                    window.draw(mist, t);
                    break;
                }
                case 6: { // 7. Jade Talisman Rune
                    sf::RectangleShape talisman(sf::Vector2f(7.0f, 14.0f));
                    talisman.setFillColor(sf::Color(255, 225, 60, 210));
                    window.draw(talisman, t);
                    break;
                }
                case 7: { // 8. Golden Pill / Elixir Spark
                    sf::CircleShape pill(4.0f);
                    pill.setFillColor(sf::Color(255, 215, 0, 230));
                    window.draw(pill, t);
                    break;
                }
                case 8: { // 9. Bamboo Grove
                    sf::RectangleShape bamboo(sf::Vector2f(3.0f, 32.0f));
                    bamboo.setFillColor(sf::Color(70, 160, 90, 180));
                    window.draw(bamboo, t);
                    break;
                }
                case 9: { // 10. Celestial Scroll
                    sf::RectangleShape scroll(sf::Vector2f(14.0f, 4.0f));
                    scroll.setFillColor(sf::Color(245, 235, 190, 190));
                    window.draw(scroll, t);
                    break;
                }
            }
            break;
        }
        case 7: { // Lord of the Mysteries (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Crimson Moon in Fog
                    sf::CircleShape moon(18.0f);
                    moon.setFillColor(sf::Color(220, 25, 45, 220));
                    window.draw(moon, t);
                    break;
                }
                case 1: { // 2. Swirling Gray Fog Bank
                    sf::CircleShape fog(24.0f);
                    fog.setFillColor(sf::Color(160, 160, 170, 75));
                    window.draw(fog, t);
                    break;
                }
                case 2: { // 3. Brass Pocket Watch / Gear
                    sf::CircleShape gear(8.0f, 8);
                    gear.setFillColor(sf::Color(190, 145, 40, 210));
                    window.draw(gear, t);
                    break;
                }
                case 3: { // 4. Top Hat Silhouette
                    sf::RectangleShape hat(sf::Vector2f(12.0f, 10.0f));
                    hat.setFillColor(sf::Color(20, 15, 25, 220));
                    window.draw(hat, t);
                    break;
                }
                case 4: { // 5. Sealed Antique Door
                    sf::RectangleShape door(sf::Vector2f(10.0f, 22.0f));
                    door.setFillColor(sf::Color(70, 40, 25, 190));
                    window.draw(door, t);
                    break;
                }
                case 5: { // 6. Flickering Gaslamp
                    sf::CircleShape lamp(3.5f);
                    float flicker = (std::sin(time * 15.0f + item.param) + 1.0f) * 0.5f;
                    lamp.setFillColor(sf::Color(255, 220, 100, static_cast<sf::Uint8>(130 + flicker * 120)));
                    window.draw(lamp, t);
                    break;
                }
                case 6: { // 7. Raven Silhouette
                    sf::CircleShape raven(4.0f);
                    raven.setFillColor(sf::Color(15, 10, 20, 230));
                    window.draw(raven, t);
                    break;
                }
                case 7: { // 8. Mystical Quill
                    sf::RectangleShape quill(sf::Vector2f(2.0f, 14.0f));
                    quill.setRotation(25.0f);
                    quill.setFillColor(sf::Color(180, 20, 35, 200));
                    window.draw(quill, t);
                    break;
                }
                case 8: { // 9. Shadow Tendril
                    float sway = std::sin(time * 5.0f + item.param) * 4.0f;
                    sf::Vertex tendril[3] = {
                        sf::Vertex(sf::Vector2f(0.0f, 20.0f), sf::Color(10, 5, 15, 200)),
                        sf::Vertex(sf::Vector2f(sway, 10.0f), sf::Color(25, 10, 35, 200)),
                        sf::Vertex(sf::Vector2f(sway * 1.5f, 0.0f), sf::Color(40, 15, 55, 200))
                    };
                    window.draw(tendril, 3, sf::LineStrip, t);
                    break;
                }
                case 9: { // 10. Tarot Eye Emblem
                    sf::CircleShape tarotEye(6.0f, 4); // diamond
                    tarotEye.setFillColor(sf::Color(210, 175, 50, 220));
                    window.draw(tarotEye, t);
                    break;
                }
            }
            break;
        }
        case 8: { // Astral Rift (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Shattered Reality Shard
                    sf::ConvexShape shard;
                    shard.setPointCount(4);
                    shard.setPoint(0, sf::Vector2f(0.0f, -8.0f));
                    shard.setPoint(1, sf::Vector2f(6.0f, 0.0f));
                    shard.setPoint(2, sf::Vector2f(2.0f, 10.0f));
                    shard.setPoint(3, sf::Vector2f(-5.0f, 3.0f));
                    shard.setFillColor(sf::Color(140, 230, 255, 190));
                    window.draw(shard, t);
                    break;
                }
                case 1: { // 2. Astral Whirlpool
                    sf::CircleShape pool(16.0f);
                    pool.setScale(1.0f, 0.4f);
                    pool.setFillColor(sf::Color::Transparent);
                    pool.setOutlineThickness(2.0f);
                    pool.setOutlineColor(sf::Color(100, 140, 255, 160));
                    window.draw(pool, t);
                    break;
                }
                case 2: { // 3. Astrolabe Ring
                    sf::CircleShape ring(12.0f);
                    ring.setFillColor(sf::Color::Transparent);
                    ring.setOutlineThickness(1.8f);
                    ring.setOutlineColor(sf::Color(220, 190, 80, 180));
                    window.draw(ring, t);
                    break;
                }
                case 3: { // 4. Starlight Cascade
                    sf::RectangleShape drop(sf::Vector2f(1.5f, 14.0f));
                    drop.setFillColor(sf::Color(255, 255, 255, 170));
                    window.draw(drop, t);
                    break;
                }
                case 4: { // 5. Dimensional Crack
                    sf::Vertex crack[3] = {
                        sf::Vertex(sf::Vector2f(-8.0f, -10.0f), sf::Color(120, 240, 255)),
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(255, 255, 255)),
                        sf::Vertex(sf::Vector2f(8.0f, 12.0f), sf::Color(120, 240, 255))
                    };
                    window.draw(crack, 3, sf::LineStrip, t);
                    break;
                }
                case 5: { // 6. Phantom Lantern
                    sf::CircleShape lantern(5.0f);
                    lantern.setFillColor(sf::Color(80, 220, 255, 190));
                    window.draw(lantern, t);
                    break;
                }
                case 6: { // 7. Ancient Pillar Ruin
                    sf::RectangleShape col(sf::Vector2f(8.0f, 22.0f));
                    col.setFillColor(sf::Color(160, 175, 205, 180));
                    window.draw(col, t);
                    break;
                }
                case 7: { // 8. Aurora Ribbon
                    sf::Vertex ribbon[3] = {
                        sf::Vertex(sf::Vector2f(-18.0f, 0.0f), sf::Color(70, 255, 190, 80)),
                        sf::Vertex(sf::Vector2f(0.0f, 6.0f), sf::Color(150, 100, 255, 90)),
                        sf::Vertex(sf::Vector2f(18.0f, 0.0f), sf::Color(70, 255, 190, 80))
                    };
                    window.draw(ribbon, 3, sf::LineStrip, t);
                    break;
                }
                case 8: { // 9. Void Spark Mote
                    sf::CircleShape mote(2.2f, 4);
                    mote.setFillColor(sf::Color(255, 255, 255, 220));
                    window.draw(mote, t);
                    break;
                }
                case 9: { // 10. Ethereal Comet
                    sf::Vertex comet[2] = {
                        sf::Vertex(sf::Vector2f(-20.0f, 6.0f), sf::Color(100, 180, 255, 0)),
                        sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(255, 255, 255, 220))
                    };
                    window.draw(comet, 2, sf::Lines, t);
                    break;
                }
            }
            break;
        }
        case 9: { // Fool's Apotheosis (Items 0..9)
            switch (item.itemType) {
                case 0: { // 1. Divine Halo Ring
                    sf::CircleShape halo(24.0f);
                    halo.setFillColor(sf::Color::Transparent);
                    halo.setOutlineThickness(3.0f);
                    halo.setOutlineColor(sf::Color(255, 235, 120, 180));
                    window.draw(halo, t);
                    break;
                }
                case 1: { // 2. Ascending Starlight Pillar
                    sf::RectangleShape pillar(sf::Vector2f(12.0f, 60.0f));
                    pillar.setFillColor(sf::Color(255, 245, 180, 75));
                    window.draw(pillar, t);
                    break;
                }
                case 2: { // 3. Fog of History Mist
                    sf::CircleShape mist(22.0f);
                    mist.setFillColor(sf::Color(255, 220, 130, 60));
                    window.draw(mist, t);
                    break;
                }
                case 3: { // 4. Golden Key Motif
                    sf::CircleShape head(4.0f);
                    head.setFillColor(sf::Color(255, 215, 0, 220));
                    window.draw(head, t);
                    sf::RectangleShape shaft(sf::Vector2f(2.0f, 10.0f));
                    shaft.setPosition(3.0f, 4.0f);
                    shaft.setFillColor(sf::Color(255, 215, 0, 220));
                    window.draw(shaft, t);
                    break;
                }
                case 4: { // 5. Constellation Web
                    sf::Vertex web[3] = {
                        sf::Vertex(sf::Vector2f(-12.0f, -6.0f), sf::Color(255, 235, 160, 160)),
                        sf::Vertex(sf::Vector2f(0.0f, 4.0f), sf::Color(255, 255, 255, 200)),
                        sf::Vertex(sf::Vector2f(14.0f, -4.0f), sf::Color(255, 235, 160, 160))
                    };
                    window.draw(web, 3, sf::LineStrip, t);
                    break;
                }
                case 5: { // 6. Supernova Crown
                    sf::CircleShape star(8.0f, 4);
                    star.setFillColor(sf::Color(255, 255, 255, 240));
                    window.draw(star, t);
                    break;
                }
                case 6: { // 7. Luminous Feather Drift
                    float sway = std::sin(time * 3.0f + item.param) * 5.0f;
                    sf::ConvexShape feather;
                    feather.setPointCount(4);
                    feather.setPoint(0, sf::Vector2f(0.0f, 0.0f));
                    feather.setPoint(1, sf::Vector2f(sway - 3.0f, 8.0f));
                    feather.setPoint(2, sf::Vector2f(sway, 16.0f));
                    feather.setPoint(3, sf::Vector2f(sway + 3.0f, 8.0f));
                    feather.setFillColor(sf::Color(255, 250, 215, 210));
                    window.draw(feather, t);
                    break;
                }
                case 7: { // 8. Astral Monarch Silhouette
                    sf::ConvexShape monarch;
                    monarch.setPointCount(3);
                    monarch.setPoint(0, sf::Vector2f(-10.0f, 20.0f));
                    monarch.setPoint(1, sf::Vector2f(0.0f, 0.0f));
                    monarch.setPoint(2, sf::Vector2f(10.0f, 20.0f));
                    monarch.setFillColor(sf::Color(65, 30, 85, 180));
                    window.draw(monarch, t);
                    break;
                }
                case 8: { // 9. Radiant Flare
                    sf::CircleShape flare(16.0f);
                    flare.setFillColor(sf::Color(255, 235, 140, 80));
                    window.draw(flare, t);
                    break;
                }
                case 9: { // 10. Apotheosis Spark Burst
                    float pulse = (std::sin(time * 10.0f + item.param) + 1.0f) * 0.5f;
                    sf::CircleShape spark(3.5f, 4);
                    spark.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(140 + pulse * 115)));
                    window.draw(spark, t);
                    break;
                }
            }
            break;
        }
    }
}

void Background::drawGround(sf::RenderWindow& window, int score) {
    int stage = Config::getStageIndex(score);
    float groundY = m_screenHeight - Config::GROUND_HEIGHT;
    float groundH = Config::GROUND_HEIGHT;

    // Themed ground colors
    sf::Color groundTop, groundBody;
    switch (stage) {
        case 0: // Sunset Arcade (Grassy Turf)
            groundTop = sf::Color(100, 205, 55);
            groundBody = sf::Color(185, 120, 60);
            break;
        case 1: // Forest (Mossy Soil)
            groundTop = sf::Color(55, 150, 45);
            groundBody = sf::Color(75, 45, 25);
            break;
        case 2: // Dino (Volcanic Basalt)
            groundTop = sf::Color(85, 40, 35);
            groundBody = sf::Color(35, 20, 25);
            break;
        case 3: // Cyber (Neon Grid Asphalt)
            groundTop = sf::Color(0, 240, 255);
            groundBody = sf::Color(20, 15, 30);
            break;
        case 4: // Space (Moon Regolith)
            groundTop = sf::Color(140, 135, 150);
            groundBody = sf::Color(45, 40, 50);
            break;
        case 5: // Universe (Galactic Dust)
            groundTop = sf::Color(140, 70, 255);
            groundBody = sf::Color(25, 10, 40);
            break;
        case 6: // Xianxia (Jade Stone)
            groundTop = sf::Color(160, 230, 200);
            groundBody = sf::Color(80, 120, 105);
            break;
        case 7: // Lord of the Mysteries (Cobblestones)
            groundTop = sf::Color(140, 30, 45); // Crimson wet sheen
            groundBody = sf::Color(35, 30, 35);
            break;
        case 8: // Astral Rift (Crystal Floor)
            groundTop = sf::Color(100, 220, 255);
            groundBody = sf::Color(25, 35, 65);
            break;
        case 9: // Fool's Apotheosis (Golden Divine Pathway)
            groundTop = sf::Color(255, 235, 120);
            groundBody = sf::Color(120, 50, 150);
            break;
    }

    // Main ground body
    sf::RectangleShape groundRect(sf::Vector2f(Config::WINDOW_WIDTH, groundH));
    groundRect.setPosition(0.0f, groundY);
    groundRect.setFillColor(groundBody);
    window.draw(groundRect);

    // Top surface strip
    sf::RectangleShape topStrip(sf::Vector2f(Config::WINDOW_WIDTH, 14.0f));
    topStrip.setPosition(0.0f, groundY);
    topStrip.setFillColor(groundTop);
    window.draw(topStrip);

    // Scrolling ground detail blocks
    sf::Color detailColor(255, 255, 255, 40);
    for (float x = -m_groundOffset; x < Config::WINDOW_WIDTH + 24.0f; x += 24.0f) {
        sf::RectangleShape notch(sf::Vector2f(12.0f, 4.0f));
        notch.setPosition(x, groundY + 5.0f);
        notch.setFillColor(detailColor);
        window.draw(notch);
    }
}
