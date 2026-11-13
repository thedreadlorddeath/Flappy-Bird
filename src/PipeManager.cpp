#include "PipeManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    bool circleIntersectsAABB(sf::Vector2f center, float radius, const sf::FloatRect& box) {
        float closestX = std::clamp(center.x, box.left, box.left + box.width);
        float closestY = std::clamp(center.y, box.top, box.top + box.height);

        float dx = center.x - closestX;
        float dy = center.y - closestY;

        return (dx * dx + dy * dy) < (radius * radius);
    }

    float randomFloat(float min, float max) {
        return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
}

sf::FloatRect PipePair::getTopBodyRect() const {
    float topCapY = gapY - gapHeight * 0.5f - Config::PIPE_CAP_HEIGHT;
    return sf::FloatRect(x, 0.0f, Config::PIPE_WIDTH, std::max(0.0f, topCapY));
}

sf::FloatRect PipePair::getTopCapRect() const {
    float capX = x - (Config::PIPE_CAP_WIDTH - Config::PIPE_WIDTH) * 0.5f;
    float capY = gapY - gapHeight * 0.5f - Config::PIPE_CAP_HEIGHT;
    return sf::FloatRect(capX, capY, Config::PIPE_CAP_WIDTH, Config::PIPE_CAP_HEIGHT);
}

sf::FloatRect PipePair::getBottomCapRect() const {
    float capX = x - (Config::PIPE_CAP_WIDTH - Config::PIPE_WIDTH) * 0.5f;
    float capY = gapY + gapHeight * 0.5f;
    return sf::FloatRect(capX, capY, Config::PIPE_CAP_WIDTH, Config::PIPE_CAP_HEIGHT);
}

sf::FloatRect PipePair::getBottomBodyRect() const {
    float bodyY = gapY + gapHeight * 0.5f + Config::PIPE_CAP_HEIGHT;
    float bodyHeight = Config::PLAY_HEIGHT - bodyY;
    return sf::FloatRect(x, bodyY, Config::PIPE_WIDTH, std::max(0.0f, bodyHeight));
}

PipeManager::PipeManager() {
    reset();
}

void PipeManager::reset() {
    m_pipes.clear();
    m_lastGapY = (Config::MIN_PIPE_GAP_Y + Config::MAX_PIPE_GAP_Y) * 0.5f;
    m_spawnDistance = 0.0f;
}

void PipeManager::spawnPipe() {
    // Generate new gap center reachable from last pipe
    float delta = randomFloat(-Config::MAX_GAP_DELTA, Config::MAX_GAP_DELTA);
    float newGapY = std::clamp(m_lastGapY + delta, Config::MIN_PIPE_GAP_Y, Config::MAX_PIPE_GAP_Y);
    m_lastGapY = newGapY;

    PipePair pair;
    pair.x = Config::WINDOW_WIDTH + 20.0f;
    pair.gapY = newGapY;
    pair.gapHeight = Config::PIPE_GAP;
    pair.scored = false;

    m_pipes.push_back(pair);
}

void PipeManager::update(float dt) {
    // Spawn tracking
    m_spawnDistance += Config::PIPE_SPEED * dt;
    if (m_pipes.empty() || m_spawnDistance >= Config::PIPE_SPACING) {
        spawnPipe();
        m_spawnDistance = 0.0f;
    }

    // Move pipes left
    for (auto& pipe : m_pipes) {
        pipe.x -= Config::PIPE_SPEED * dt;
    }

    // Remove pipes off-screen
    while (!m_pipes.empty() && m_pipes.front().x < -Config::PIPE_CAP_WIDTH) {
        m_pipes.erase(m_pipes.begin());
    }
}

bool PipeManager::checkCollision(sf::Vector2f birdPos, float birdRadius) const {
    for (const auto& pipe : m_pipes) {
        // Quick AABB bounds prune
        if (pipe.x - Config::PIPE_CAP_WIDTH > birdPos.x + birdRadius ||
            pipe.x + Config::PIPE_CAP_WIDTH * 2.0f < birdPos.x - birdRadius) {
            continue;
        }

        if (circleIntersectsAABB(birdPos, birdRadius, pipe.getTopBodyRect()) ||
            circleIntersectsAABB(birdPos, birdRadius, pipe.getTopCapRect()) ||
            circleIntersectsAABB(birdPos, birdRadius, pipe.getBottomCapRect()) ||
            circleIntersectsAABB(birdPos, birdRadius, pipe.getBottomBodyRect())) {
            return true;
        }
    }
    return false;
}

bool PipeManager::checkScored(float birdX) {
    for (auto& pipe : m_pipes) {
        if (!pipe.scored && (pipe.x + Config::PIPE_WIDTH * 0.5f) < birdX) {
            pipe.scored = true;
            return true;
        }
    }
    return false;
}

void PipeManager::drawSinglePipe(sf::RenderWindow& window, float x, float topY, float btmY, bool isTop) {
    float height = btmY - topY;
    if (height <= 0.0f) return;

    const sf::Color pipeOutline(12, 48, 26);
    const sf::Color bodyBase(34, 172, 74);       // Vibrant emerald
    const sf::Color bodyShadow(16, 92, 42);      // Dark forest green shadow
    const sf::Color bodyHighlight(140, 248, 182, 220); // Cyan-mint gloss highlight

    // 1. Pipe Column Body Base
    sf::RectangleShape body(sf::Vector2f(Config::PIPE_WIDTH, height));
    body.setPosition(x, topY);
    body.setFillColor(bodyBase);
    body.setOutlineColor(pipeOutline);
    body.setOutlineThickness(2.0f);
    window.draw(body);

    // 2. Glossy Highlight Strip (down left side)
    sf::RectangleShape highlight(sf::Vector2f(10.0f, height));
    highlight.setPosition(x + 7.0f, topY);
    highlight.setFillColor(bodyHighlight);
    window.draw(highlight);

    // Secondary soft sheen stripe
    sf::RectangleShape sheen(sf::Vector2f(5.0f, height));
    sheen.setPosition(x + 21.0f, topY);
    sheen.setFillColor(sf::Color(255, 255, 255, 60));
    window.draw(sheen);

    // 3. Dark Cylindrical 3D Shadow Strip (down right side)
    sf::RectangleShape shadow(sf::Vector2f(16.0f, height));
    shadow.setPosition(x + Config::PIPE_WIDTH - 16.0f, topY);
    shadow.setFillColor(bodyShadow);
    window.draw(shadow);
}

void PipeManager::draw(sf::RenderWindow& window) {
    const sf::Color pipeOutline(12, 48, 26);
    const sf::Color capBase(40, 185, 80);
    const sf::Color capHighlight(165, 255, 205);
    const sf::Color capShadow(14, 82, 38);

    for (const auto& pipe : m_pipes) {
        float topCapY = pipe.gapY - pipe.gapHeight * 0.5f - Config::PIPE_CAP_HEIGHT;
        float btmCapY = pipe.gapY + pipe.gapHeight * 0.5f;
        float capX = pipe.x - (Config::PIPE_CAP_WIDTH - Config::PIPE_WIDTH) * 0.5f;

        // 1. Draw Top Pipe Column
        drawSinglePipe(window, pipe.x, 0.0f, topCapY, true);

        // 2. Draw Top Lip Cap
        sf::RectangleShape topCap(sf::Vector2f(Config::PIPE_CAP_WIDTH, Config::PIPE_CAP_HEIGHT));
        topCap.setPosition(capX, topCapY);
        topCap.setFillColor(capBase);
        topCap.setOutlineColor(pipeOutline);
        topCap.setOutlineThickness(2.2f);
        window.draw(topCap);

        // Top Cap Highlight Stripe
        sf::RectangleShape topCapHi(sf::Vector2f(12.0f, Config::PIPE_CAP_HEIGHT));
        topCapHi.setPosition(capX + 9.0f, topCapY);
        topCapHi.setFillColor(capHighlight);
        window.draw(topCapHi);

        // Top Cap Right Shadow
        sf::RectangleShape topCapSh(sf::Vector2f(18.0f, Config::PIPE_CAP_HEIGHT));
        topCapSh.setPosition(capX + Config::PIPE_CAP_WIDTH - 18.0f, topCapY);
        topCapSh.setFillColor(capShadow);
        window.draw(topCapSh);

        // Top Cap Lip Rim (facing gap)
        sf::RectangleShape topLipRim(sf::Vector2f(Config::PIPE_CAP_WIDTH, 4.0f));
        topLipRim.setPosition(capX, topCapY + Config::PIPE_CAP_HEIGHT - 4.0f);
        topLipRim.setFillColor(sf::Color(190, 255, 220));
        window.draw(topLipRim);

        // 3. Draw Bottom Pipe Column
        drawSinglePipe(window, pipe.x, btmCapY + Config::PIPE_CAP_HEIGHT, Config::PLAY_HEIGHT, false);

        // 4. Draw Bottom Lip Cap
        sf::RectangleShape btmCap(sf::Vector2f(Config::PIPE_CAP_WIDTH, Config::PIPE_CAP_HEIGHT));
        btmCap.setPosition(capX, btmCapY);
        btmCap.setFillColor(capBase);
        btmCap.setOutlineColor(pipeOutline);
        btmCap.setOutlineThickness(2.2f);
        window.draw(btmCap);

        // Bottom Cap Highlight Stripe
        sf::RectangleShape btmCapHi(sf::Vector2f(12.0f, Config::PIPE_CAP_HEIGHT));
        btmCapHi.setPosition(capX + 9.0f, btmCapY);
        btmCapHi.setFillColor(capHighlight);
        window.draw(btmCapHi);

        // Bottom Cap Right Shadow
        sf::RectangleShape btmCapSh(sf::Vector2f(18.0f, Config::PIPE_CAP_HEIGHT));
        btmCapSh.setPosition(capX + Config::PIPE_CAP_WIDTH - 18.0f, btmCapY);
        btmCapSh.setFillColor(capShadow);
        window.draw(btmCapSh);

        // Bottom Cap Lip Rim (facing gap)
        sf::RectangleShape btmLipRim(sf::Vector2f(Config::PIPE_CAP_WIDTH, 4.0f));
        btmLipRim.setPosition(capX, btmCapY);
        btmLipRim.setFillColor(sf::Color(190, 255, 220));
        window.draw(btmLipRim);
    }
}
