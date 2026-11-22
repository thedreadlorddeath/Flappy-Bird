#include "UI.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace {
    MedalTier getTier(int score) {
        if (score >= 999) return MedalTier::Omnipotent;
        if (score >= 600) return MedalTier::Cosmic;
        if (score >= 300) return MedalTier::Platinum;
        if (score >= 100) return MedalTier::Gold;
        if (score >= 35)  return MedalTier::Silver;
        if (score >= 10)  return MedalTier::Bronze;
        return MedalTier::None;
    }
}

UI::UI()
    : m_fontLoaded(false)
    , m_scoreScale(1.0f)
    , m_pulseTimer(0.0f)
    , m_cardProgress(0.0f)
    , m_announcedStage(-1)
    , m_stageBannerTimer(0.0f)
    , m_stageBannerTitle("")
    , m_stageBannerSubtitle("")
    , m_apotheosisBanner(false) {
    loadFont();
}

void UI::loadFont() {
    const char* fontCandidates[] = {
        // Bundled assets
        "font.ttf",
        "assets/font.ttf",
        // Android system fonts
        "/system/fonts/Roboto-Regular.ttf",
        "/system/fonts/Roboto-Bold.ttf",
        "/system/fonts/DroidSans.ttf",
        "/system/fonts/DroidSans-Bold.ttf",
        "/system/fonts/NotoSans-Regular.ttf",
        // Windows system fonts
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/consola.ttf",
        // Linux system fonts
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf"
    };

    for (const char* path : fontCandidates) {
        if (m_font.loadFromFile(path)) {
            m_fontLoaded = true;
            return;
        }
    }

    std::cerr << "Warning: Could not load any font." << std::endl;
}

void UI::reset() {
    m_scoreScale = 1.0f;
    m_pulseTimer = 0.0f;
    m_cardProgress = 0.0f;
    m_announcedStage = -1;
    m_stageBannerTimer = 0.0f;
    m_apotheosisBanner = false;
}

void UI::triggerScorePop() {
    m_scoreScale = 1.45f;
}

void UI::triggerStageBanner(int stage) {
    m_announcedStage = stage;
    m_stageBannerTimer = 2.8f;
    m_apotheosisBanner = (stage >= 9);
    m_stageBannerTitle = "STAGE " + std::to_string(stage + 1) + ": " + Config::getStageName(stage);
    m_stageBannerSubtitle = (stage >= 9) ? "THE FINAL APOTHEOSIS! MAXIMUM SPEED!" : "DIFFICULTY & SPEED ELEVATED!";
}

void UI::triggerApotheosisBanner() {
    m_stageBannerTimer = 4.0f;
    m_apotheosisBanner = true;
    m_stageBannerTitle = "SCORE 999: OMNIPOTENT FOOL!";
    m_stageBannerSubtitle = "DIVINE ASCENSION COMPLETE! SUPERNOVA UNLOCKED!";
}

void UI::update(float dt) {
    m_pulseTimer += dt;
    m_scoreScale += (1.0f - m_scoreScale) * 11.0f * dt;

    if (m_cardProgress < 1.0f) {
        m_cardProgress = std::min(1.0f, m_cardProgress + dt * 2.8f);
    }

    if (m_stageBannerTimer > 0.0f) {
        m_stageBannerTimer -= dt;
    }
}

void UI::drawTextShadowed(sf::RenderWindow& window, const std::string& str,
                          sf::Vector2f position, unsigned int size,
                          sf::Color fill, sf::Color shadow,
                          bool centered) {
    if (!m_fontLoaded) return;

    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setStyle(sf::Text::Bold);

    if (centered) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    }

    // Drop shadow
    text.setPosition(position + sf::Vector2f(2.5f, 2.8f));
    text.setFillColor(shadow);
    window.draw(text);

    // Main text
    text.setPosition(position);
    text.setFillColor(fill);
    window.draw(text);
}

void UI::drawHUD(sf::RenderWindow& window, int score, int bestScore) {
    if (!m_fontLoaded) return;

    int stage = Config::getStageIndex(score);
    float speedMult = Config::getSpeedMultiplier(score);

    // 1. Top Bar: Stage & Speed Badges
    // Stage Badge (Top Left)
    sf::RectangleShape stagePill(sf::Vector2f(136.0f, 32.0f));
    stagePill.setPosition(16.0f, 18.0f);
    stagePill.setFillColor(sf::Color(20, 16, 36, 210));
    stagePill.setOutlineColor(sf::Color(255, 195, 60, 220));
    stagePill.setOutlineThickness(1.8f);
    window.draw(stagePill);

    std::string stageStr = "STG " + std::to_string(stage + 1) + "/10";
    drawTextShadowed(window, stageStr, sf::Vector2f(84.0f, 33.0f), 14,
                     sf::Color(255, 235, 120), sf::Color(10, 8, 20));

    // Speed Multiplier Badge (Top Right)
    sf::RectangleShape speedPill(sf::Vector2f(110.0f, 32.0f));
    speedPill.setPosition(Config::WINDOW_WIDTH - 126.0f, 18.0f);
    speedPill.setFillColor(sf::Color(20, 16, 36, 210));
    speedPill.setOutlineColor(sf::Color(0, 235, 255, 220));
    speedPill.setOutlineThickness(1.8f);
    window.draw(speedPill);

    std::stringstream speedSs;
    speedSs << std::fixed << std::setprecision(2) << speedMult << "x";
    drawTextShadowed(window, speedSs.str(), sf::Vector2f(Config::WINDOW_WIDTH - 71.0f, 33.0f), 14,
                     sf::Color(180, 245, 255), sf::Color(10, 8, 20));

    // 2. Large Central Score
    std::string scoreStr = std::to_string(score);
    sf::Text text;
    text.setFont(m_font);
    text.setString(scoreStr);
    text.setCharacterSize(52);
    text.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    text.setScale(m_scoreScale, m_scoreScale);

    sf::Vector2f centerPos(Config::WINDOW_WIDTH * 0.5f, 68.0f);

    // Deep shadow
    text.setPosition(centerPos + sf::Vector2f(3.5f, 4.0f));
    text.setFillColor(sf::Color(15, 10, 30, 220));
    window.draw(text);

    // Gold/White Core
    text.setPosition(centerPos);
    if (score >= 999) {
        text.setFillColor(sf::Color(255, 220, 100));
        text.setOutlineColor(sf::Color(255, 60, 150));
    } else {
        text.setFillColor(sf::Color(255, 255, 255));
        text.setOutlineColor(sf::Color(25, 20, 40));
    }
    text.setOutlineThickness(4.0f);
    window.draw(text);

    // 3. Stage Transition Announcement Banner
    if (m_stageBannerTimer > 0.0f) {
        float alphaT = std::min(1.0f, m_stageBannerTimer * 1.5f);
        sf::Uint8 alpha = static_cast<sf::Uint8>(std::clamp(alphaT * 240.0f, 0.0f, 255.0f));

        sf::RectangleShape banner(sf::Vector2f(440.0f, 74.0f));
        banner.setOrigin(220.0f, 37.0f);
        banner.setPosition(Config::WINDOW_WIDTH * 0.5f, 155.0f);
        banner.setFillColor(sf::Color(18, 12, 34, alpha));
        banner.setOutlineColor(m_apotheosisBanner ? sf::Color(255, 215, 45, alpha) : sf::Color(0, 235, 255, alpha));
        banner.setOutlineThickness(2.8f);
        window.draw(banner);

        drawTextShadowed(window, m_stageBannerTitle, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 142.0f),
                         19, m_apotheosisBanner ? sf::Color(255, 225, 80, alpha) : sf::Color(255, 255, 255, alpha),
                         sf::Color(10, 8, 20, alpha));

        drawTextShadowed(window, m_stageBannerSubtitle, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 168.0f),
                         13, m_apotheosisBanner ? sf::Color(255, 160, 220, alpha) : sf::Color(255, 195, 60, alpha),
                         sf::Color(10, 8, 20, alpha));
    }
}

void UI::drawGetReady(sf::RenderWindow& window) {
    float screenH = window.getView().getSize().y;

    // 1. Arcade Title: FLAPPY BIRD - SUNSET ARCADE
    float titleY = screenH * 0.16f;
    drawTextShadowed(window, "FLAPPY BIRD", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, titleY),
                     44, sf::Color(255, 215, 45), sf::Color(25, 15, 35, 240));

    drawTextShadowed(window, "SUNSET ARCADE  -  10 REALMS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, titleY + 38.0f),
                     15, sf::Color(255, 145, 65), sf::Color(20, 10, 30, 220));

    // 2. Subtitle: GET READY! with gentle floating
    float getReadyY = screenH * 0.28f + std::sin(m_pulseTimer * 4.0f) * 4.0f;
    drawTextShadowed(window, "GET READY!", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, getReadyY),
                     30, sf::Color(255, 110, 50), sf::Color(30, 15, 40, 220));

    // 3. Instructions Prompt box with pulsing outline
    float promptAlpha = 180.0f + std::sin(m_pulseTimer * 5.0f) * 65.0f;
    sf::Uint8 a = static_cast<sf::Uint8>(std::clamp(promptAlpha, 0.0f, 255.0f));

    float promptY = screenH * 0.60f;
    sf::RectangleShape promptCard(sf::Vector2f(340.0f, 68.0f));
    promptCard.setOrigin(170.0f, 34.0f);
    promptCard.setPosition(Config::WINDOW_WIDTH * 0.5f, promptY);
    promptCard.setFillColor(sf::Color(20, 15, 36, 210));
    promptCard.setOutlineColor(sf::Color(255, 195, 60, a));
    promptCard.setOutlineThickness(2.2f);
    window.draw(promptCard);

    drawTextShadowed(window, "TAP SCREEN OR PRESS SPACE", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, promptY - 12.0f),
                     17, sf::Color(255, 255, 245, a), sf::Color(10, 10, 20, 220));

    drawTextShadowed(window, "TO FLAP & ASCEND", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, promptY + 14.0f),
                     15, sf::Color(255, 210, 80, a), sf::Color(10, 10, 20, 220));

    // 4. Navigation Buttons: [C] WARDROBE  |  [S] SCORES
    sf::FloatRect wardBounds = getWardrobeButtonBounds(screenH);
    sf::RectangleShape wardBtn(sf::Vector2f(wardBounds.width, wardBounds.height));
    wardBtn.setPosition(wardBounds.left, wardBounds.top);
    wardBtn.setFillColor(sf::Color(36, 26, 58, 230));
    wardBtn.setOutlineColor(sf::Color(255, 185, 45));
    wardBtn.setOutlineThickness(2.2f);
    window.draw(wardBtn);
    drawTextShadowed(window, "[C] WARDROBE",
                     sf::Vector2f(wardBounds.left + wardBounds.width * 0.5f, wardBounds.top + wardBounds.height * 0.5f),
                     15, sf::Color(255, 225, 80), sf::Color(10, 10, 20));

    sf::FloatRect scoreBounds = getScoreboardButtonBounds(screenH);
    sf::RectangleShape scoreBtn(sf::Vector2f(scoreBounds.width, scoreBounds.height));
    scoreBtn.setPosition(scoreBounds.left, scoreBounds.top);
    scoreBtn.setFillColor(sf::Color(36, 26, 58, 230));
    scoreBtn.setOutlineColor(sf::Color(0, 235, 255));
    scoreBtn.setOutlineThickness(2.2f);
    window.draw(scoreBtn);
    drawTextShadowed(window, "[S] SCORES",
                     sf::Vector2f(scoreBounds.left + scoreBounds.width * 0.5f, scoreBounds.top + scoreBounds.height * 0.5f),
                     15, sf::Color(180, 245, 255), sf::Color(10, 10, 20));
}

void UI::drawPauseOverlay(sf::RenderWindow& window) {
    float screenH = window.getView().getSize().y;

    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, screenH));
    veil.setFillColor(sf::Color(14, 10, 28, 185));
    window.draw(veil);

    sf::RectangleShape box(sf::Vector2f(300.0f, 170.0f));
    box.setOrigin(150.0f, 85.0f);
    box.setPosition(Config::WINDOW_WIDTH * 0.5f, screenH * 0.42f);
    box.setFillColor(sf::Color(24, 18, 44, 245));
    box.setOutlineColor(sf::Color(255, 195, 60));
    box.setOutlineThickness(3.0f);
    window.draw(box);

    drawTextShadowed(window, "PAUSED", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, screenH * 0.38f),
                     34, sf::Color(255, 225, 80), sf::Color(15, 10, 25));

    drawTextShadowed(window, "PRESS [P] TO RESUME", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, screenH * 0.46f),
                     17, sf::Color(240, 245, 255), sf::Color(15, 10, 25));
}

void UI::drawMedal(sf::RenderWindow& window, sf::Vector2f center, MedalTier tier) {
    if (tier == MedalTier::None) {
        sf::CircleShape emptySlot(24.0f);
        emptySlot.setOrigin(24.0f, 24.0f);
        emptySlot.setPosition(center);
        emptySlot.setFillColor(sf::Color(35, 28, 52));
        emptySlot.setOutlineColor(sf::Color(55, 45, 75));
        emptySlot.setOutlineThickness(2.0f);
        window.draw(emptySlot);

        sf::RectangleShape dash(sf::Vector2f(14.0f, 3.0f));
        dash.setOrigin(7.0f, 1.5f);
        dash.setPosition(center);
        dash.setFillColor(sf::Color(75, 65, 95));
        window.draw(dash);
        return;
    }

    sf::Color baseColor;
    sf::Color rimColor;
    sf::Color highlightColor;
    sf::Color starColor;

    switch (tier) {
    case MedalTier::Bronze:
        baseColor = sf::Color(205, 120, 55);
        rimColor = sf::Color(135, 70, 30);
        highlightColor = sf::Color(245, 185, 130);
        starColor = sf::Color(255, 230, 200);
        break;
    case MedalTier::Silver:
        baseColor = sf::Color(215, 220, 225);
        rimColor = sf::Color(140, 150, 160);
        highlightColor = sf::Color(255, 255, 255);
        starColor = sf::Color(245, 250, 255);
        break;
    case MedalTier::Gold:
        baseColor = sf::Color(255, 205, 35);
        rimColor = sf::Color(190, 130, 20);
        highlightColor = sf::Color(255, 245, 140);
        starColor = sf::Color(255, 255, 255);
        break;
    case MedalTier::Platinum:
        baseColor = sf::Color(190, 245, 255);
        rimColor = sf::Color(110, 180, 215);
        highlightColor = sf::Color(255, 255, 255);
        starColor = sf::Color(255, 255, 255);
        break;
    case MedalTier::Cosmic:
        baseColor = sf::Color(165, 80, 255);
        rimColor = sf::Color(90, 30, 180);
        highlightColor = sf::Color(230, 190, 255);
        starColor = sf::Color(0, 245, 255);
        break;
    case MedalTier::Omnipotent:
        baseColor = sf::Color(255, 225, 80);
        rimColor = sf::Color(255, 70, 160);
        highlightColor = sf::Color(255, 255, 255);
        starColor = sf::Color(255, 255, 255);
        break;
    default:
        break;
    }

    // Outer ribbon
    sf::ConvexShape ribbonL;
    ribbonL.setPointCount(3);
    ribbonL.setPoint(0, sf::Vector2f(center.x - 14.0f, center.y + 10.0f));
    ribbonL.setPoint(1, sf::Vector2f(center.x - 22.0f, center.y + 36.0f));
    ribbonL.setPoint(2, sf::Vector2f(center.x - 8.0f, center.y + 26.0f));
    ribbonL.setFillColor((tier >= MedalTier::Cosmic) ? sf::Color(140, 40, 220) : sf::Color(220, 45, 45));
    window.draw(ribbonL);

    sf::ConvexShape ribbonR;
    ribbonR.setPointCount(3);
    ribbonR.setPoint(0, sf::Vector2f(center.x + 14.0f, center.y + 10.0f));
    ribbonR.setPoint(1, sf::Vector2f(center.x + 22.0f, center.y + 36.0f));
    ribbonR.setPoint(2, sf::Vector2f(center.x + 8.0f, center.y + 26.0f));
    ribbonR.setFillColor((tier >= MedalTier::Cosmic) ? sf::Color(100, 20, 180) : sf::Color(200, 35, 35));
    window.draw(ribbonR);

    // Medal Outer Rim
    sf::CircleShape outerRim(26.0f);
    outerRim.setOrigin(26.0f, 26.0f);
    outerRim.setPosition(center);
    outerRim.setFillColor(rimColor);
    window.draw(outerRim);

    // Medal Body
    sf::CircleShape medalBody(22.0f);
    medalBody.setOrigin(22.0f, 22.0f);
    medalBody.setPosition(center);
    medalBody.setFillColor(baseColor);
    window.draw(medalBody);

    // Sheen
    sf::CircleShape sheen(16.0f);
    sheen.setScale(1.0f, 0.45f);
    sheen.setOrigin(16.0f, 16.0f);
    sheen.setPosition(center.x, center.y - 7.0f);
    sheen.setFillColor(highlightColor);
    window.draw(sheen);

    // Central Star
    sf::ConvexShape star;
    star.setPointCount(10);
    const float innerR = 4.5f;
    const float outerR = 9.5f;
    const float step = 3.14159265f / 5.0f;
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? outerR : innerR;
        float angle = i * step - 3.14159265f * 0.5f;
        star.setPoint(i, sf::Vector2f(center.x + std::cos(angle) * r, center.y + std::sin(angle) * r));
    }
    star.setFillColor(starColor);
    window.draw(star);
}

void UI::drawGameOverCard(sf::RenderWindow& window, int score, int bestScore, bool isNewBest) {
    float screenH = window.getView().getSize().y;

    float t = m_cardProgress;
    float ease = 1.0f - std::pow(1.0f - t, 3.0f);

    float targetY = screenH * 0.40f;
    float startY = screenH + 200.0f;
    float currentY = startY + (targetY - startY) * ease;

    // "GAME OVER" header
    drawTextShadowed(window, "GAME OVER", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, currentY - 120.0f),
                     40, sf::Color(255, 80, 70), sf::Color(20, 10, 25));

    // Card Panel
    const float cardW = 380.0f;
    const float cardH = 195.0f;
    sf::RectangleShape card(sf::Vector2f(cardW, cardH));
    card.setOrigin(cardW * 0.5f, cardH * 0.5f);
    card.setPosition(Config::WINDOW_WIDTH * 0.5f, currentY);
    card.setFillColor(sf::Color(24, 18, 42, 245));
    card.setOutlineColor(sf::Color(245, 185, 45));
    card.setOutlineThickness(3.5f);
    window.draw(card);

    // Inner divider
    sf::RectangleShape divider(sf::Vector2f(2.0f, cardH - 30.0f));
    divider.setOrigin(1.0f, (cardH - 30.0f) * 0.5f);
    divider.setPosition(Config::WINDOW_WIDTH * 0.5f - 40.0f, currentY);
    divider.setFillColor(sf::Color(55, 42, 80));
    window.draw(divider);

    // Left Side: MEDAL & TIER
    float medalCenterX = Config::WINDOW_WIDTH * 0.5f - 110.0f;
    drawTextShadowed(window, "MEDAL", sf::Vector2f(medalCenterX, currentY - 60.0f),
                     16, sf::Color(255, 210, 75), sf::Color(15, 10, 25));

    MedalTier tier = getTier(score);
    drawMedal(window, sf::Vector2f(medalCenterX, currentY - 2.0f), tier);

    // Right Side: SCORE, BEST, STAGE, SPEED
    float labelX = Config::WINDOW_WIDTH * 0.5f - 20.0f;
    float valX = Config::WINDOW_WIDTH * 0.5f + 140.0f;

    // Score
    drawTextShadowed(window, "SCORE", sf::Vector2f(labelX, currentY - 65.0f),
                     15, sf::Color(255, 195, 65), sf::Color(15, 10, 25), false);
    drawTextShadowed(window, std::to_string(score), sf::Vector2f(valX, currentY - 65.0f),
                     22, (score >= 999) ? sf::Color(255, 220, 80) : sf::Color::White, sf::Color(15, 10, 25), false);

    // Best Score
    drawTextShadowed(window, "BEST", sf::Vector2f(labelX, currentY - 25.0f),
                     15, sf::Color(255, 195, 65), sf::Color(15, 10, 25), false);
    drawTextShadowed(window, std::to_string(bestScore), sf::Vector2f(valX, currentY - 25.0f),
                     22, sf::Color::White, sf::Color(15, 10, 25), false);

    // Stage reached
    int stage = Config::getStageIndex(score);
    drawTextShadowed(window, "STAGE", sf::Vector2f(labelX, currentY + 15.0f),
                     15, sf::Color(180, 240, 255), sf::Color(15, 10, 25), false);
    std::string stageLabel = std::to_string(stage + 1) + "/10 (" + Config::getStageName(stage).substr(0, 8) + ")";
    drawTextShadowed(window, stageLabel, sf::Vector2f(valX, currentY + 15.0f),
                     15, sf::Color(255, 235, 120), sf::Color(15, 10, 25), false);

    // Speed reached
    float speedMult = Config::getSpeedMultiplier(score);
    std::stringstream speedSs;
    speedSs << std::fixed << std::setprecision(2) << speedMult << "x";
    drawTextShadowed(window, "SPEED", sf::Vector2f(labelX, currentY + 50.0f),
                     15, sf::Color(180, 240, 255), sf::Color(15, 10, 25), false);
    drawTextShadowed(window, speedSs.str(), sf::Vector2f(valX, currentY + 50.0f),
                     16, sf::Color(0, 235, 255), sf::Color(15, 10, 25), false);

    // New Best badge!
    if (isNewBest && score > 0) {
        sf::RectangleShape newBadge(sf::Vector2f(44.0f, 18.0f));
        newBadge.setOrigin(22.0f, 9.0f);
        newBadge.setPosition(labelX + 50.0f, currentY - 25.0f);
        newBadge.setFillColor(sf::Color(240, 45, 45));
        window.draw(newBadge);

        drawTextShadowed(window, "NEW", sf::Vector2f(labelX + 50.0f, currentY - 25.0f),
                         11, sf::Color::White, sf::Color(15, 10, 25));
    }

    // Celebratory 999 badge
    if (bestScore >= 999) {
        drawTextShadowed(window, "*** OMNIPOTENT ASCENDANT UNLOCKED ***",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, currentY + 115.0f),
                         13, sf::Color(255, 225, 80), sf::Color(20, 10, 30));
    }

    // Restart prompt below card
    if (m_cardProgress >= 0.9f) {
        float promptAlpha = 180.0f + std::sin(m_pulseTimer * 6.0f) * 65.0f;
        sf::Uint8 a = static_cast<sf::Uint8>(std::clamp(promptAlpha, 0.0f, 255.0f));

        drawTextShadowed(window, "PRESS [R] OR TAP SCREEN TO RESTART",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, currentY + 140.0f),
                         16, sf::Color(255, 235, 120, a), sf::Color(15, 10, 25, a));

        // Navigation Buttons on Game Over
        sf::FloatRect wardBounds = getWardrobeButtonBounds(screenH);
        sf::RectangleShape wardBtn(sf::Vector2f(wardBounds.width, wardBounds.height));
        wardBtn.setPosition(wardBounds.left, wardBounds.top);
        wardBtn.setFillColor(sf::Color(36, 26, 58, 230));
        wardBtn.setOutlineColor(sf::Color(255, 185, 45));
        wardBtn.setOutlineThickness(2.2f);
        window.draw(wardBtn);
        drawTextShadowed(window, "[C] WARDROBE",
                         sf::Vector2f(wardBounds.left + wardBounds.width * 0.5f, wardBounds.top + wardBounds.height * 0.5f),
                         15, sf::Color(255, 225, 80), sf::Color(10, 10, 20));

        sf::FloatRect scoreBounds = getScoreboardButtonBounds(screenH);
        sf::RectangleShape scoreBtn(sf::Vector2f(scoreBounds.width, scoreBounds.height));
        scoreBtn.setPosition(scoreBounds.left, scoreBounds.top);
        scoreBtn.setFillColor(sf::Color(36, 26, 58, 230));
        scoreBtn.setOutlineColor(sf::Color(0, 235, 255));
        scoreBtn.setOutlineThickness(2.2f);
        window.draw(scoreBtn);
        drawTextShadowed(window, "[S] SCORES",
                         sf::Vector2f(scoreBounds.left + scoreBounds.width * 0.5f, scoreBounds.top + scoreBounds.height * 0.5f),
                         15, sf::Color(180, 245, 255), sf::Color(10, 10, 20));
    }
}

sf::FloatRect UI::getWardrobeButtonBounds(float screenH) const {
    return sf::FloatRect(35.0f, screenH - 120.0f, 195.0f, 46.0f);
}

sf::FloatRect UI::getScoreboardButtonBounds(float screenH) const {
    return sf::FloatRect(250.0f, screenH - 120.0f, 195.0f, 46.0f);
}

sf::FloatRect UI::getBackButtonBounds(float screenH) const {
    return sf::FloatRect(Config::WINDOW_WIDTH * 0.5f - 85.0f, screenH - 72.0f, 170.0f, 44.0f);
}

sf::FloatRect UI::getHatPrevBounds() const {
    return sf::FloatRect(40.0f, 310.0f, 44.0f, 44.0f);
}

sf::FloatRect UI::getHatNextBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH - 84.0f, 310.0f, 44.0f, 44.0f);
}

sf::FloatRect UI::getOutfitPrevBounds() const {
    return sf::FloatRect(40.0f, 430.0f, 44.0f, 44.0f);
}

sf::FloatRect UI::getOutfitNextBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH - 84.0f, 430.0f, 44.0f, 44.0f);
}

sf::FloatRect UI::getApotheosisToggleBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH * 0.5f - 145.0f, 530.0f, 290.0f, 42.0f);
}

void UI::drawCosmeticsScreen(sf::RenderWindow& window, const CosmeticsManager& cosmetics, int bestScore) {
    float screenH = window.getView().getSize().y;

    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, screenH));
    veil.setFillColor(sf::Color(16, 12, 32, 242));
    window.draw(veil);

    // Title
    drawTextShadowed(window, "WARDROBE & AURAS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 58.0f),
                     28, sf::Color(255, 215, 45), sf::Color(15, 10, 25));

    drawTextShadowed(window, "BEST SCORE: " + std::to_string(bestScore) + " PTS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 95.0f),
                     16, sf::Color(180, 240, 255), sf::Color(15, 10, 25));

    // --- HAT SECTION ---
    drawTextShadowed(window, "HEADWEAR", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 280.0f),
                     18, sf::Color(255, 185, 45), sf::Color(15, 10, 25));

    sf::FloatRect hpb = getHatPrevBounds();
    sf::RectangleShape prevHatBtn(sf::Vector2f(hpb.width, hpb.height));
    prevHatBtn.setPosition(hpb.left, hpb.top);
    prevHatBtn.setFillColor(sf::Color(45, 35, 70));
    prevHatBtn.setOutlineColor(sf::Color(255, 205, 60));
    prevHatBtn.setOutlineThickness(2.0f);
    window.draw(prevHatBtn);
    drawTextShadowed(window, "<", sf::Vector2f(hpb.left + 22.0f, hpb.top + 20.0f), 24, sf::Color::White, sf::Color::Black);

    sf::FloatRect hnb = getHatNextBounds();
    sf::RectangleShape nextHatBtn(sf::Vector2f(hnb.width, hnb.height));
    nextHatBtn.setPosition(hnb.left, hnb.top);
    nextHatBtn.setFillColor(sf::Color(45, 35, 70));
    nextHatBtn.setOutlineColor(sf::Color(255, 205, 60));
    nextHatBtn.setOutlineThickness(2.0f);
    window.draw(nextHatBtn);
    drawTextShadowed(window, ">", sf::Vector2f(hnb.left + 22.0f, hnb.top + 20.0f), 24, sf::Color::White, sf::Color::Black);

    HatId currentHat = cosmetics.getEquippedHat();
    std::string hatName = cosmetics.getHatName(currentHat);
    drawTextShadowed(window, hatName, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 325.0f),
                     19, sf::Color::White, sf::Color(10, 10, 20));

    int hatReq = cosmetics.getHats()[static_cast<int>(currentHat)].requiredScore;
    if (bestScore >= hatReq) {
        drawTextShadowed(window, "[ EQUIPPED ]", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 355.0f),
                         14, sf::Color(80, 240, 120), sf::Color(10, 10, 20));
    } else {
        drawTextShadowed(window, "[ LOCKED - SCORE " + std::to_string(hatReq) + " ]",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 355.0f),
                         14, sf::Color(245, 80, 80), sf::Color(10, 10, 20));
    }

    // --- OUTFIT / AURAS SECTION ---
    drawTextShadowed(window, "OUTFIT & AURAS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 400.0f),
                     18, sf::Color(255, 185, 45), sf::Color(15, 10, 25));

    sf::FloatRect opb = getOutfitPrevBounds();
    sf::RectangleShape prevOutBtn(sf::Vector2f(opb.width, opb.height));
    prevOutBtn.setPosition(opb.left, opb.top);
    prevOutBtn.setFillColor(sf::Color(45, 35, 70));
    prevOutBtn.setOutlineColor(sf::Color(255, 205, 60));
    prevOutBtn.setOutlineThickness(2.0f);
    window.draw(prevOutBtn);
    drawTextShadowed(window, "<", sf::Vector2f(opb.left + 22.0f, opb.top + 20.0f), 24, sf::Color::White, sf::Color::Black);

    sf::FloatRect onb = getOutfitNextBounds();
    sf::RectangleShape nextOutBtn(sf::Vector2f(onb.width, onb.height));
    nextOutBtn.setPosition(onb.left, onb.top);
    nextOutBtn.setFillColor(sf::Color(45, 35, 70));
    nextOutBtn.setOutlineColor(sf::Color(255, 205, 60));
    nextOutBtn.setOutlineThickness(2.0f);
    window.draw(nextOutBtn);
    drawTextShadowed(window, ">", sf::Vector2f(onb.left + 22.0f, onb.top + 20.0f), 24, sf::Color::White, sf::Color::Black);

    OutfitId currentOutfit = cosmetics.getEquippedOutfit();
    std::string outName = cosmetics.getOutfitName(currentOutfit);
    drawTextShadowed(window, outName, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 445.0f),
                     19, sf::Color::White, sf::Color(10, 10, 20));

    int outReq = cosmetics.getOutfits()[static_cast<int>(currentOutfit)].requiredScore;
    if (bestScore >= outReq) {
        drawTextShadowed(window, "[ EQUIPPED ]", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 475.0f),
                         14, sf::Color(80, 240, 120), sf::Color(10, 10, 20));
    } else {
        drawTextShadowed(window, "[ LOCKED - SCORE " + std::to_string(outReq) + " ]",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 475.0f),
                         14, sf::Color(245, 80, 80), sf::Color(10, 10, 20));
    }

    // Score 999 Apotheosis Toggle Button
    if (bestScore >= 999) {
        sf::FloatRect atb = getApotheosisToggleBounds();
        sf::RectangleShape apoBtn(sf::Vector2f(atb.width, atb.height));
        apoBtn.setPosition(atb.left, atb.top);
        bool apoActive = cosmetics.isApotheosisMode();
        apoBtn.setFillColor(apoActive ? sf::Color(80, 30, 95) : sf::Color(35, 25, 55));
        apoBtn.setOutlineColor(apoActive ? sf::Color(255, 215, 60) : sf::Color(150, 150, 180));
        apoBtn.setOutlineThickness(2.0f);
        window.draw(apoBtn);

        std::string apoText = apoActive ? "[T] APOTHEOSIS MODE: ON" : "[T] APOTHEOSIS MODE: OFF";
        drawTextShadowed(window, apoText, sf::Vector2f(atb.left + atb.width * 0.5f, atb.top + atb.height * 0.5f),
                         14, apoActive ? sf::Color(255, 225, 80) : sf::Color(200, 200, 220), sf::Color(10, 8, 20));
    }

    // Help Hint
    drawTextShadowed(window, "TAP < > OR USE ARROWS TO CYCLE COSMETICS",
                     sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, screenH - 120.0f),
                     13, sf::Color(200, 210, 230), sf::Color(10, 10, 20));

    // Back Button
    sf::FloatRect bb = getBackButtonBounds(screenH);
    sf::RectangleShape backBtn(sf::Vector2f(bb.width, bb.height));
    backBtn.setPosition(bb.left, bb.top);
    backBtn.setFillColor(sf::Color(55, 35, 80));
    backBtn.setOutlineColor(sf::Color(255, 185, 45));
    backBtn.setOutlineThickness(2.0f);
    window.draw(backBtn);
    drawTextShadowed(window, "[ESC] BACK", sf::Vector2f(bb.left + bb.width * 0.5f, bb.top + bb.height * 0.5f),
                     16, sf::Color::White, sf::Color::Black);
}

void UI::drawScoreboardScreen(sf::RenderWindow& window, const CosmeticsManager& cosmetics, int bestScore) {
    float screenH = window.getView().getSize().y;

    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, screenH));
    veil.setFillColor(sf::Color(16, 12, 32, 245));
    window.draw(veil);

    drawTextShadowed(window, "HALL OF ASCENDANTS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 55.0f),
                     28, sf::Color(0, 240, 255), sf::Color(15, 10, 25));

    // Best Record Card
    sf::RectangleShape bestCard(sf::Vector2f(420.0f, 90.0f));
    bestCard.setOrigin(210.0f, 45.0f);
    bestCard.setPosition(Config::WINDOW_WIDTH * 0.5f, 140.0f);
    bestCard.setFillColor(sf::Color(28, 22, 50));
    bestCard.setOutlineColor(sf::Color(255, 205, 45));
    bestCard.setOutlineThickness(2.5f);
    window.draw(bestCard);

    drawTextShadowed(window, "PERSONAL BEST RECORD", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 118.0f),
                     15, sf::Color(255, 205, 50), sf::Color(10, 10, 20));
    drawTextShadowed(window, std::to_string(bestScore) + " PTS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 155.0f),
                     32, sf::Color::White, sf::Color(10, 10, 20));

    // Current Rank Title
    std::string rankTitle = "Novice Fledgling";
    if (bestScore >= 999)      rankTitle = "OMNIPOTENT FOOL (MAX)";
    else if (bestScore >= 900) rankTitle = "Void Conqueror";
    else if (bestScore >= 750) rankTitle = "Celestial Lord";
    else if (bestScore >= 600) rankTitle = "Immortal Ascendant";
    else if (bestScore >= 400) rankTitle = "Cosmic Wanderer";
    else if (bestScore >= 300) rankTitle = "Astronaut Pioneer";
    else if (bestScore >= 220) rankTitle = "Primordial Phoenix";
    else if (bestScore >= 150) rankTitle = "Arcane Mage";
    else if (bestScore >= 100) rankTitle = "Forest Monarch";
    else if (bestScore >= 65)  rankTitle = "Cyber Ace";
    else if (bestScore >= 35)  rankTitle = "Sky Ace";
    else if (bestScore >= 15)  rankTitle = "Sunset Aviator";
    else if (bestScore >= 1)   rankTitle = "Fledgling Flapper";

    drawTextShadowed(window, "RANK: " + rankTitle, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 210.0f),
                     17, (bestScore >= 999) ? sf::Color(255, 225, 80) : sf::Color(180, 240, 255), sf::Color(10, 10, 20));

    // Progression Milestones (12 milestones arranged in 2 neat columns)
    drawTextShadowed(window, "COSMETIC ASCENSION CHECKLIST", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 250.0f),
                     16, sf::Color(255, 195, 60), sf::Color(10, 10, 20));

    struct Milestone {
        int req;
        const char* name;
    };

    Milestone milestones[12] = {
        { 1,   "Score 1: Headband" },
        { 15,  "Score 15: Aviator" },
        { 35,  "Score 35: Top Hat" },
        { 65,  "Score 65: Cyber Pylon" },
        { 100, "Score 100: Monarch" },
        { 150, "Score 150: Wizard" },
        { 220, "Score 220: Phoenix" },
        { 300, "Score 300: Astronaut" },
        { 400, "Score 400: Celestial" },
        { 500, "Score 500: Void Horns" },
        { 750, "Score 750: Solar Flare" },
        { 999, "Score 999: Omnipotent" }
    };

    int unlockedCount = 0;
    for (int i = 0; i < 12; ++i) {
        int col = i / 6;
        int row = i % 6;
        float x = (col == 0) ? 35.0f : 248.0f;
        float y = 282.0f + row * 34.0f;

        bool unl = (bestScore >= milestones[i].req);
        if (unl) unlockedCount++;

        sf::Color itemCol = unl ? sf::Color(110, 255, 140) : sf::Color(140, 140, 160);
        std::string check = unl ? "[x] " : "[ ] ";
        drawTextShadowed(window, check + milestones[i].name, sf::Vector2f(x, y),
                         13, itemCol, sf::Color(10, 10, 20), false);
    }

    // Completion %
    int pct = (unlockedCount * 100) / 12;
    drawTextShadowed(window, "UNLOCKED: " + std::to_string(pct) + "% (" + std::to_string(unlockedCount) + "/12)",
                     sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 515.0f),
                     16, sf::Color(255, 215, 60), sf::Color(10, 10, 20));

    // Back Button
    sf::FloatRect bb = getBackButtonBounds(screenH);
    sf::RectangleShape backBtn(sf::Vector2f(bb.width, bb.height));
    backBtn.setPosition(bb.left, bb.top);
    backBtn.setFillColor(sf::Color(55, 35, 80));
    backBtn.setOutlineColor(sf::Color(0, 235, 255));
    backBtn.setOutlineThickness(2.0f);
    window.draw(backBtn);
    drawTextShadowed(window, "[ESC] BACK", sf::Vector2f(bb.left + bb.width * 0.5f, bb.top + bb.height * 0.5f),
                     16, sf::Color::White, sf::Color::Black);
}
