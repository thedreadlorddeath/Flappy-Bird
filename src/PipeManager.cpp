#include "PipeManager.hpp"
#include "Config.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    constexpr float PI = 3.14159265358979323846f;

    bool circleIntersectsAABB(sf::Vector2f center, float radius, const sf::FloatRect& box) {
        float closestX = std::clamp(center.x, box.left, box.left + box.width);
        float closestY = std::clamp(center.y, box.top, box.top + box.height);

        float dx = center.x - closestX;
        float dy = center.y - closestY;

        return (dx * dx + dy * dy) < (radius * radius);
    }

    float randomFloat(float min, float max) {
        return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) / (max - min));
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
    float bodyHeight = playHeight - bodyY;
    return sf::FloatRect(x, bodyY, Config::PIPE_WIDTH, std::max(0.0f, bodyHeight));
}

PipeManager::PipeManager()
    : m_lastGapY(300.0f)
    , m_spawnDistance(0.0f)
    , m_playHeight(Config::DEFAULT_PLAY_HEIGHT)
    , m_time(0.0f) {
    reset();
}

void PipeManager::reset() {
    m_pipes.clear();
    m_lastGapY = m_playHeight * 0.5f;
    m_spawnDistance = 0.0f;
    m_time = 0.0f;
}

void PipeManager::spawnPipe(int score) {
    int stage = Config::getStageIndex(score);
    float gapHeight = Config::getCurrentGapHeight(score);

    float minGapY = 80.0f + gapHeight * 0.5f;
    float maxGapY = m_playHeight - 70.0f - gapHeight * 0.5f;

    float delta = randomFloat(-Config::MAX_GAP_DELTA, Config::MAX_GAP_DELTA);
    float newGapY = std::clamp(m_lastGapY + delta, minGapY, maxGapY);
    m_lastGapY = newGapY;

    PipePair pair;
    pair.x = Config::WINDOW_WIDTH + 25.0f;
    pair.baseGapY = newGapY;
    pair.gapY = newGapY;
    pair.gapHeight = gapHeight;
    pair.oscPhase = randomFloat(0.0f, 2.0f * PI);
    pair.stage = stage;
    pair.scored = false;
    pair.playHeight = m_playHeight;

    m_pipes.push_back(pair);
}

void PipeManager::update(float dt, int score) {
    m_time += dt;

    float currentSpeed = Config::getCurrentPipeSpeed(score);
    float horizontalSpacing = Config::getHorizontalSpacing(score);

    m_spawnDistance += currentSpeed * dt;
    if (m_pipes.empty() || m_spawnDistance >= horizontalSpacing) {
        spawnPipe(score);
        m_spawnDistance = 0.0f;
    }

    float oscAmp = Config::getOscillationAmplitude(score);
    float oscFreq = Config::getOscillationFrequency(score);

    // Update pipes motion and gap oscillation
    for (auto& pipe : m_pipes) {
        pipe.x -= currentSpeed * dt;
        pipe.playHeight = m_playHeight;

        if (oscAmp > 0.0f) {
            float osc = std::sin(pipe.oscPhase + m_time * oscFreq) * oscAmp;
            float targetY = pipe.baseGapY + osc;

            // Secondary pulse for stage >= 3
            if (pipe.stage >= 3) {
                targetY += std::cos(pipe.oscPhase * 1.5f + m_time * (oscFreq * 0.7f)) * (oscAmp * 0.25f);
            }

            float minGapY = 75.0f + pipe.gapHeight * 0.5f;
            float maxGapY = m_playHeight - 65.0f - pipe.gapHeight * 0.5f;
            pipe.gapY = std::clamp(targetY, minGapY, maxGapY);
        } else {
            pipe.gapY = pipe.baseGapY;
        }
    }

    // Remove off-screen pipes
    while (!m_pipes.empty() && m_pipes.front().x < -Config::PIPE_CAP_WIDTH - 20.0f) {
        m_pipes.erase(m_pipes.begin());
    }
}

bool PipeManager::checkCollision(sf::Vector2f birdPos, float birdRadius) const {
    for (const auto& pipe : m_pipes) {
        // Quick AABB prune
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
        if (!pipe.scored && pipe.x + Config::PIPE_WIDTH * 0.5f < birdX) {
            pipe.scored = true;
            return true;
        }
    }
    return false;
}

void PipeManager::draw(sf::RenderWindow& window, int score, float time) {
    (void)score;
    for (const auto& pipe : m_pipes) {
        drawThemedObstaclePair(window, pipe, time);
    }
}

void PipeManager::drawThemedObstaclePair(sf::RenderWindow& window, const PipePair& pipe, float time) {
    float x = pipe.x;
    float w = Config::PIPE_WIDTH;
    float capW = Config::PIPE_CAP_WIDTH;
    float capH = Config::PIPE_CAP_HEIGHT;
    float capX = x - (capW - w) * 0.5f;

    float topCapY = pipe.gapY - pipe.gapHeight * 0.5f - capH;
    float btmCapY = pipe.gapY + pipe.gapHeight * 0.5f;
    float btmBodyY = btmCapY + capH;
    float btmBodyH = std::max(0.0f, pipe.playHeight - btmBodyY);

    sf::Color outlineColor(25, 15, 20);

    switch (pipe.stage) {
        case 0: { // Classic Sunset Emerald Pipes
            sf::Color bodyMain(30, 180, 75);
            sf::Color bodyBevel(15, 115, 45);
            sf::Color highlight(120, 240, 150);

            // Top Body
            sf::RectangleShape topBody(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topBody.setPosition(x, 0.0f);
            topBody.setFillColor(bodyMain);
            topBody.setOutlineThickness(2.0f);
            topBody.setOutlineColor(outlineColor);
            window.draw(topBody);

            sf::RectangleShape topHl(sf::Vector2f(10.0f, std::max(0.0f, topCapY)));
            topHl.setPosition(x + 10.0f, 0.0f);
            topHl.setFillColor(highlight);
            window.draw(topHl);

            // Top Cap
            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(bodyMain);
            topCap.setOutlineThickness(2.5f);
            topCap.setOutlineColor(outlineColor);
            window.draw(topCap);

            sf::RectangleShape topCapHl(sf::Vector2f(12.0f, capH - 4.0f));
            topCapHl.setPosition(capX + 12.0f, topCapY + 2.0f);
            topCapHl.setFillColor(highlight);
            window.draw(topCapHl);

            // Bottom Cap
            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(bodyMain);
            btmCap.setOutlineThickness(2.5f);
            btmCap.setOutlineColor(outlineColor);
            window.draw(btmCap);

            sf::RectangleShape btmCapHl(sf::Vector2f(12.0f, capH - 4.0f));
            btmCapHl.setPosition(capX + 12.0f, btmCapY + 2.0f);
            btmCapHl.setFillColor(highlight);
            window.draw(btmCapHl);

            // Bottom Body
            sf::RectangleShape btmBody(sf::Vector2f(w, btmBodyH));
            btmBody.setPosition(x, btmBodyY);
            btmBody.setFillColor(bodyMain);
            btmBody.setOutlineThickness(2.0f);
            btmBody.setOutlineColor(outlineColor);
            window.draw(btmBody);

            sf::RectangleShape btmHl(sf::Vector2f(10.0f, btmBodyH));
            btmHl.setPosition(x + 10.0f, btmBodyY);
            btmHl.setFillColor(highlight);
            window.draw(btmHl);
            break;
        }
        case 1: { // Forest Timber Trunks & Moss Pillars with Vine Tips
            sf::Color bark(110, 68, 38);
            sf::Color moss(60, 145, 55);

            // Top trunk
            sf::RectangleShape topTrunk(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topTrunk.setPosition(x, 0.0f);
            topTrunk.setFillColor(bark);
            topTrunk.setOutlineThickness(2.0f);
            topTrunk.setOutlineColor(sf::Color(45, 25, 10));
            window.draw(topTrunk);

            // Top Cap (mossy node)
            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(moss);
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(sf::Color(30, 60, 20));
            window.draw(topCap);

            // Swaying vine tip
            float vineSway = std::sin(time * 4.0f + pipe.x * 0.05f) * 6.0f;
            sf::Vertex vine[3] = {
                sf::Vertex(sf::Vector2f(x + w * 0.5f, topCapY + capH), sf::Color(80, 180, 50)),
                sf::Vertex(sf::Vector2f(x + w * 0.5f + vineSway, topCapY + capH + 14.0f), sf::Color(100, 200, 60)),
                sf::Vertex(sf::Vector2f(x + w * 0.5f + vineSway * 1.5f, topCapY + capH + 24.0f), sf::Color(50, 130, 30))
            };
            window.draw(vine, 3, sf::LineStrip);

            // Bottom Cap
            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(moss);
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(sf::Color(30, 60, 20));
            window.draw(btmCap);

            // Bottom trunk
            sf::RectangleShape btmTrunk(sf::Vector2f(w, btmBodyH));
            btmTrunk.setPosition(x, btmBodyY);
            btmTrunk.setFillColor(bark);
            btmTrunk.setOutlineThickness(2.0f);
            btmTrunk.setOutlineColor(sf::Color(45, 25, 10));
            window.draw(btmTrunk);
            break;
        }
        case 2: { // Dino: Fossil Ribcages & Volcanic Basalt
            sf::Color basalt(45, 40, 48);
            sf::Color lava(255, 80, 20);

            // Top Basalt
            sf::RectangleShape topB(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topB.setPosition(x, 0.0f);
            topB.setFillColor(basalt);
            topB.setOutlineThickness(2.0f);
            topB.setOutlineColor(sf::Color(20, 15, 20));
            window.draw(topB);

            // Lava crack
            sf::RectangleShape lavaCrack(sf::Vector2f(4.0f, std::max(0.0f, topCapY)));
            lavaCrack.setPosition(x + 18.0f, 0.0f);
            lavaCrack.setFillColor(lava);
            window.draw(lavaCrack);

            // Top fossil bone cap
            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(sf::Color(220, 215, 195)); // bone ivory
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(sf::Color(80, 70, 60));
            window.draw(topCap);

            // Bottom fossil bone cap
            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(sf::Color(220, 215, 195));
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(sf::Color(80, 70, 60));
            window.draw(btmCap);

            // Bottom Basalt
            sf::RectangleShape btmB(sf::Vector2f(w, btmBodyH));
            btmB.setPosition(x, btmBodyY);
            btmB.setFillColor(basalt);
            btmB.setOutlineThickness(2.0f);
            btmB.setOutlineColor(sf::Color(20, 15, 20));
            window.draw(btmB);

            sf::RectangleShape btmLava(sf::Vector2f(4.0f, btmBodyH));
            btmLava.setPosition(x + 24.0f, btmBodyY);
            btmLava.setFillColor(lava);
            window.draw(btmLava);
            break;
        }
        case 3: { // Cyberpunk: Neon Cyber-Pylons & Hologram Gates
            sf::Color carbon(25, 25, 35);
            sf::Color neonCyan(0, 240, 255);
            sf::Color neonMagenta(255, 20, 150);

            // Top Pylon
            sf::RectangleShape topPylon(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topPylon.setPosition(x, 0.0f);
            topPylon.setFillColor(carbon);
            topPylon.setOutlineThickness(2.0f);
            topPylon.setOutlineColor(neonCyan);
            window.draw(topPylon);

            // Top Cap (holographic emitter)
            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(neonMagenta);
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(neonCyan);
            window.draw(topCap);

            // Holographic laser beam at opening
            float laserAlpha = (std::sin(time * 12.0f) + 1.0f) * 0.5f;
            sf::RectangleShape laserLine(sf::Vector2f(capW - 8.0f, 3.0f));
            laserLine.setPosition(capX + 4.0f, topCapY + capH + 2.0f);
            laserLine.setFillColor(sf::Color(0, 255, 255, static_cast<sf::Uint8>(120 + laserAlpha * 135)));
            window.draw(laserLine);

            // Bottom Cap
            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(neonMagenta);
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(neonCyan);
            window.draw(btmCap);

            // Bottom Pylon
            sf::RectangleShape btmPylon(sf::Vector2f(w, btmBodyH));
            btmPylon.setPosition(x, btmBodyY);
            btmPylon.setFillColor(carbon);
            btmPylon.setOutlineThickness(2.0f);
            btmPylon.setOutlineColor(neonCyan);
            window.draw(btmPylon);
            break;
        }
        case 4: { // Space: Meteors & Asteroids with Burning Atmospheric Crust
            sf::Color rockColor(95, 80, 85);
            sf::Color entryGlow(255, 130, 20);

            // Top Meteor cluster
            sf::RectangleShape topRock(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topRock.setPosition(x, 0.0f);
            topRock.setFillColor(rockColor);
            topRock.setOutlineThickness(2.0f);
            topRock.setOutlineColor(sf::Color(40, 30, 35));
            window.draw(topRock);

            // Top Meteor head
            sf::RectangleShape topHead(sf::Vector2f(capW, capH));
            topHead.setPosition(capX, topCapY);
            topHead.setFillColor(sf::Color(120, 95, 95));
            topHead.setOutlineThickness(2.5f);
            topHead.setOutlineColor(entryGlow);
            window.draw(topHead);

            // Fiery re-entry rim
            sf::RectangleShape topFire(sf::Vector2f(capW, 4.0f));
            topFire.setPosition(capX, topCapY + capH - 4.0f);
            topFire.setFillColor(entryGlow);
            window.draw(topFire);

            // Bottom Meteor head
            sf::RectangleShape btmHead(sf::Vector2f(capW, capH));
            btmHead.setPosition(capX, btmCapY);
            btmHead.setFillColor(sf::Color(120, 95, 95));
            btmHead.setOutlineThickness(2.5f);
            btmHead.setOutlineColor(entryGlow);
            window.draw(btmHead);

            sf::RectangleShape btmFire(sf::Vector2f(capW, 4.0f));
            btmFire.setPosition(capX, btmCapY);
            btmFire.setFillColor(entryGlow);
            window.draw(btmFire);

            // Bottom Rock
            sf::RectangleShape btmRock(sf::Vector2f(w, btmBodyH));
            btmRock.setPosition(x, btmBodyY);
            btmRock.setFillColor(rockColor);
            btmRock.setOutlineThickness(2.0f);
            btmRock.setOutlineColor(sf::Color(40, 30, 35));
            window.draw(btmRock);
            break;
        }
        case 5: { // Universe: Cosmic Void Gates & Galactic Filaments
            sf::Color voidColor(20, 10, 45);
            sf::Color filament(140, 60, 255);

            sf::RectangleShape topG(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topG.setPosition(x, 0.0f);
            topG.setFillColor(voidColor);
            topG.setOutlineThickness(2.0f);
            topG.setOutlineColor(filament);
            window.draw(topG);

            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(filament);
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(sf::Color(220, 180, 255));
            window.draw(topCap);

            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(filament);
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(sf::Color(220, 180, 255));
            window.draw(btmCap);

            sf::RectangleShape btmG(sf::Vector2f(w, btmBodyH));
            btmG.setPosition(x, btmBodyY);
            btmG.setFillColor(voidColor);
            btmG.setOutlineThickness(2.0f);
            btmG.setOutlineColor(filament);
            window.draw(btmG);
            break;
        }
        case 6: { // Xianxia: Spirit-Jade Pillars & Sword-Qi Formations
            sf::Color jade(180, 240, 215);
            sf::Color gold(255, 215, 60);

            sf::RectangleShape topJade(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topJade.setPosition(x, 0.0f);
            topJade.setFillColor(jade);
            topJade.setOutlineThickness(2.0f);
            topJade.setOutlineColor(sf::Color(80, 160, 130));
            window.draw(topJade);

            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(gold);
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(sf::Color(180, 130, 20));
            window.draw(topCap);

            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(gold);
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(sf::Color(180, 130, 20));
            window.draw(btmCap);

            sf::RectangleShape btmJade(sf::Vector2f(w, btmBodyH));
            btmJade.setPosition(x, btmBodyY);
            btmJade.setFillColor(jade);
            btmJade.setOutlineThickness(2.0f);
            btmJade.setOutlineColor(sf::Color(80, 160, 130));
            window.draw(btmJade);
            break;
        }
        case 7: { // Lord of the Mysteries: Victorian Gothic Clockwork & Beyonder Monoliths
            sf::Color gothic(35, 30, 38);
            sf::Color brass(195, 145, 45);

            sf::RectangleShape topM(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topM.setPosition(x, 0.0f);
            topM.setFillColor(gothic);
            topM.setOutlineThickness(2.0f);
            topM.setOutlineColor(sf::Color(180, 30, 45)); // Crimson moon tint
            window.draw(topM);

            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(brass);
            topCap.setOutlineThickness(2.0f);
            topCap.setOutlineColor(sf::Color(60, 40, 15));
            window.draw(topCap);

            // Clock gear motif on cap
            sf::CircleShape gear(6.0f);
            gear.setOrigin(6.0f, 6.0f);
            gear.setPosition(capX + capW * 0.5f, topCapY + capH * 0.5f);
            gear.setFillColor(sf::Color(130, 90, 25));
            window.draw(gear);

            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(brass);
            btmCap.setOutlineThickness(2.0f);
            btmCap.setOutlineColor(sf::Color(60, 40, 15));
            window.draw(btmCap);

            sf::CircleShape gearB(6.0f);
            gearB.setOrigin(6.0f, 6.0f);
            gearB.setPosition(capX + capW * 0.5f, btmCapY + capH * 0.5f);
            gearB.setFillColor(sf::Color(130, 90, 25));
            window.draw(gearB);

            sf::RectangleShape btmM(sf::Vector2f(w, btmBodyH));
            btmM.setPosition(x, btmBodyY);
            btmM.setFillColor(gothic);
            btmM.setOutlineThickness(2.0f);
            btmM.setOutlineColor(sf::Color(180, 30, 45));
            window.draw(btmM);
            break;
        }
        case 8: { // Astral Rift: Fractured Starlight Monoliths & Spatial Fissures
            sf::Color monolith(30, 35, 60);
            sf::Color fissure(120, 220, 255);

            sf::RectangleShape topR(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topR.setPosition(x, 0.0f);
            topR.setFillColor(monolith);
            topR.setOutlineThickness(2.0f);
            topR.setOutlineColor(fissure);
            window.draw(topR);

            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(sf::Color(70, 90, 150));
            topCap.setOutlineThickness(2.5f);
            topCap.setOutlineColor(fissure);
            window.draw(topCap);

            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(sf::Color(70, 90, 150));
            btmCap.setOutlineThickness(2.5f);
            btmCap.setOutlineColor(fissure);
            window.draw(btmCap);

            sf::RectangleShape btmR(sf::Vector2f(w, btmBodyH));
            btmR.setPosition(x, btmBodyY);
            btmR.setFillColor(monolith);
            btmR.setOutlineThickness(2.0f);
            btmR.setOutlineColor(fissure);
            window.draw(btmR);
            break;
        }
        case 9: { // Fool's Apotheosis: Sovereign Divine Obelisks & Astral Thrones
            sf::Color divineGold(255, 235, 140);
            sf::Color amethyst(160, 40, 220);
            sf::Color diamond(255, 255, 255);

            sf::RectangleShape topO(sf::Vector2f(w, std::max(0.0f, topCapY)));
            topO.setPosition(x, 0.0f);
            topO.setFillColor(divineGold);
            topO.setOutlineThickness(2.5f);
            topO.setOutlineColor(amethyst);
            window.draw(topO);

            sf::RectangleShape topCap(sf::Vector2f(capW, capH));
            topCap.setPosition(capX, topCapY);
            topCap.setFillColor(amethyst);
            topCap.setOutlineThickness(2.5f);
            topCap.setOutlineColor(diamond);
            window.draw(topCap);

            sf::RectangleShape btmCap(sf::Vector2f(capW, capH));
            btmCap.setPosition(capX, btmCapY);
            btmCap.setFillColor(amethyst);
            btmCap.setOutlineThickness(2.5f);
            btmCap.setOutlineColor(diamond);
            window.draw(btmCap);

            sf::RectangleShape btmO(sf::Vector2f(w, btmBodyH));
            btmO.setPosition(x, btmBodyY);
            btmO.setFillColor(divineGold);
            btmO.setOutlineThickness(2.5f);
            btmO.setOutlineColor(amethyst);
            window.draw(btmO);
            break;
        }
    }
}
