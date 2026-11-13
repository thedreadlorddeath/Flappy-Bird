#include "UI.hpp"
#include "Config.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
    MedalTier getTier(int score) {
        if (score >= 40) return MedalTier::Platinum;
        if (score >= 30) return MedalTier::Gold;
        if (score >= 20) return MedalTier::Silver;
        if (score >= 10) return MedalTier::Bronze;
        return MedalTier::None;
    }
}

UI::UI()
    : m_fontLoaded(false)
    , m_scoreScale(1.0f)
    , m_pulseTimer(0.0f)
    , m_cardProgress(0.0f) {
    loadFont();
}

void UI::loadFont() {
    // Try reliable Android, Windows, and Linux font paths
    const char* fontCandidates[] = {
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

    std::cerr << "Warning: Could not load default Windows font." << std::endl;
}

void UI::reset() {
    m_scoreScale = 1.0f;
    m_pulseTimer = 0.0f;
    m_cardProgress = 0.0f;
}

void UI::update(float dt) {
    m_pulseTimer += dt;

    // Smoothly ease score scale back to 1.0f
    m_scoreScale += (1.0f - m_scoreScale) * 11.0f * dt;

    // Card entry animation (ease out cubic)
    if (m_cardProgress < 1.0f) {
        m_cardProgress = std::min(1.0f, m_cardProgress + dt * 2.8f);
    }
}

void UI::triggerScorePop() {
    m_scoreScale = 1.45f;
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

    // Shadow
    text.setPosition(position + sf::Vector2f(2.5f, 3.0f));
    text.setFillColor(shadow);
    window.draw(text);

    // Main text
    text.setPosition(position);
    text.setFillColor(fill);
    window.draw(text);
}

void UI::drawHUD(sf::RenderWindow& window, int score) {
    if (!m_fontLoaded) return;

    std::string scoreStr = std::to_string(score);
    sf::Text text;
    text.setFont(m_font);
    text.setString(scoreStr);
    text.setCharacterSize(48);
    text.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    text.setScale(m_scoreScale, m_scoreScale);

    sf::Vector2f centerPos(Config::WINDOW_WIDTH * 0.5f, 65.0f);

    // Deep drop shadow
    text.setPosition(centerPos + sf::Vector2f(3.0f, 3.5f));
    text.setFillColor(sf::Color(15, 10, 30, 200));
    window.draw(text);

    // Crisp white text with golden rim
    text.setPosition(centerPos);
    text.setFillColor(sf::Color(255, 255, 255));
    text.setOutlineColor(sf::Color(25, 20, 35));
    text.setOutlineThickness(3.5f);
    window.draw(text);
}

void UI::drawGetReady(sf::RenderWindow& window) {
    // 1. Arcade Title: FLAPPY BIRD
    float titleY = 145.0f;
    drawTextShadowed(window, "FLAPPY BIRD", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, titleY),
                     42, sf::Color(255, 215, 45), sf::Color(25, 15, 35, 220));

    // 2. Subtitle: GET READY! with gentle floating
    float getReadyY = 215.0f + std::sin(m_pulseTimer * 4.0f) * 4.0f;
    drawTextShadowed(window, "GET READY!", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, getReadyY),
                     30, sf::Color(255, 125, 50), sf::Color(30, 15, 40, 220));

    // 3. Instructions Prompt box with pulsing opacity
    float alpha = 180.0f + std::sin(m_pulseTimer * 5.0f) * 65.0f;
    sf::Uint8 promptAlpha = static_cast<sf::Uint8>(std::clamp(alpha, 0.0f, 255.0f));

    sf::RectangleShape promptCard(sf::Vector2f(320.0f, 65.0f));
    promptCard.setOrigin(160.0f, 32.5f);
    promptCard.setPosition(Config::WINDOW_WIDTH * 0.5f, 480.0f);
    promptCard.setFillColor(sf::Color(20, 15, 35, 190));
    promptCard.setOutlineColor(sf::Color(255, 190, 60, promptAlpha));
    promptCard.setOutlineThickness(2.0f);
    window.draw(promptCard);

    drawTextShadowed(window, "PRESS SPACE OR CLICK", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 468.0f),
                     18, sf::Color(255, 255, 240, promptAlpha), sf::Color(10, 10, 20, 220));

    drawTextShadowed(window, "TO FLAP", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 492.0f),
                     16, sf::Color(255, 210, 80, promptAlpha), sf::Color(10, 10, 20, 220));

    // 4. Navigation Buttons: [C] WARDROBE  |  [S] SCORES
    sf::FloatRect wardBounds = getWardrobeButtonBounds();
    sf::RectangleShape wardBtn(sf::Vector2f(wardBounds.width, wardBounds.height));
    wardBtn.setPosition(wardBounds.left, wardBounds.top);
    wardBtn.setFillColor(sf::Color(35, 25, 55, 220));
    wardBtn.setOutlineColor(sf::Color(255, 185, 45));
    wardBtn.setOutlineThickness(2.0f);
    window.draw(wardBtn);
    drawTextShadowed(window, "[C] WARDROBE", sf::Vector2f(wardBounds.left + wardBounds.width * 0.5f, wardBounds.top + wardBounds.height * 0.5f),
                     13, sf::Color(255, 225, 80), sf::Color(10, 10, 20));

    sf::FloatRect scoreBounds = getScoreboardButtonBounds();
    sf::RectangleShape scoreBtn(sf::Vector2f(scoreBounds.width, scoreBounds.height));
    scoreBtn.setPosition(scoreBounds.left, scoreBounds.top);
    scoreBtn.setFillColor(sf::Color(35, 25, 55, 220));
    scoreBtn.setOutlineColor(sf::Color(0, 235, 255));
    scoreBtn.setOutlineThickness(2.0f);
    window.draw(scoreBtn);
    drawTextShadowed(window, "[S] SCORES", sf::Vector2f(scoreBounds.left + scoreBounds.width * 0.5f, scoreBounds.top + scoreBounds.height * 0.5f),
                     13, sf::Color(180, 245, 255), sf::Color(10, 10, 20));
}

void UI::drawPauseOverlay(sf::RenderWindow& window) {
    // Semi-transparent backdrop
    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    veil.setFillColor(sf::Color(14, 10, 28, 175));
    window.draw(veil);

    // Pause Dialog Box
    sf::RectangleShape box(sf::Vector2f(280.0f, 160.0f));
    box.setOrigin(140.0f, 80.0f);
    box.setPosition(Config::WINDOW_WIDTH * 0.5f, Config::WINDOW_HEIGHT * 0.42f);
    box.setFillColor(sf::Color(24, 18, 42, 240));
    box.setOutlineColor(sf::Color(255, 195, 60));
    box.setOutlineThickness(3.0f);
    window.draw(box);

    drawTextShadowed(window, "PAUSED", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, Config::WINDOW_HEIGHT * 0.38f),
                     34, sf::Color(255, 225, 80), sf::Color(15, 10, 25));

    drawTextShadowed(window, "PRESS [P] TO RESUME", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, Config::WINDOW_HEIGHT * 0.46f),
                     17, sf::Color(240, 245, 255), sf::Color(15, 10, 25));
}

void UI::drawMedal(sf::RenderWindow& window, sf::Vector2f center, MedalTier tier) {
    if (tier == MedalTier::None) {
        // Empty medal slot
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
    default:
        break;
    }

    // Outer ribbon hanging
    sf::ConvexShape ribbonL;
    ribbonL.setPointCount(3);
    ribbonL.setPoint(0, sf::Vector2f(center.x - 14.0f, center.y + 10.0f));
    ribbonL.setPoint(1, sf::Vector2f(center.x - 22.0f, center.y + 36.0f));
    ribbonL.setPoint(2, sf::Vector2f(center.x - 8.0f, center.y + 26.0f));
    ribbonL.setFillColor(sf::Color(220, 45, 45));
    window.draw(ribbonL);

    sf::ConvexShape ribbonR;
    ribbonR.setPointCount(3);
    ribbonR.setPoint(0, sf::Vector2f(center.x + 14.0f, center.y + 10.0f));
    ribbonR.setPoint(1, sf::Vector2f(center.x + 22.0f, center.y + 36.0f));
    ribbonR.setPoint(2, sf::Vector2f(center.x + 8.0f, center.y + 26.0f));
    ribbonR.setFillColor(sf::Color(200, 35, 35));
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

    // Specular Highlight Arc
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
    // Smooth entry animation: slide up from below
    float t = m_cardProgress;
    float ease = 1.0f - std::pow(1.0f - t, 3.0f); // Ease out cubic

    float targetY = Config::WINDOW_HEIGHT * 0.40f;
    float startY = Config::WINDOW_HEIGHT + 200.0f;
    float currentY = startY + (targetY - startY) * ease;

    // "GAME OVER" header
    drawTextShadowed(window, "GAME OVER", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, currentY - 110.0f),
                     38, sf::Color(255, 80, 70), sf::Color(20, 10, 25));

    // Card Panel
    const float cardW = 340.0f;
    const float cardH = 175.0f;
    sf::RectangleShape card(sf::Vector2f(cardW, cardH));
    card.setOrigin(cardW * 0.5f, cardH * 0.5f);
    card.setPosition(Config::WINDOW_WIDTH * 0.5f, currentY);
    card.setFillColor(sf::Color(24, 18, 40, 245));
    card.setOutlineColor(sf::Color(245, 185, 45)); // Glossy golden arcade trim
    card.setOutlineThickness(3.5f);
    window.draw(card);

    // Inner card divider
    sf::RectangleShape divider(sf::Vector2f(2.0f, cardH - 30.0f));
    divider.setOrigin(1.0f, (cardH - 30.0f) * 0.5f);
    divider.setPosition(Config::WINDOW_WIDTH * 0.5f - 30.0f, currentY);
    divider.setFillColor(sf::Color(55, 42, 80));
    window.draw(divider);

    // Left Side: MEDAL
    float medalCenterX = Config::WINDOW_WIDTH * 0.5f - 95.0f;
    drawTextShadowed(window, "MEDAL", sf::Vector2f(medalCenterX, currentY - 55.0f),
                     16, sf::Color(255, 210, 75), sf::Color(15, 10, 25));

    MedalTier tier = getTier(score);
    drawMedal(window, sf::Vector2f(medalCenterX, currentY + 5.0f), tier);

    // Right Side: SCORE and BEST
    float labelX = Config::WINDOW_WIDTH * 0.5f + 45.0f;
    float valX = Config::WINDOW_WIDTH * 0.5f + 130.0f;

    // Score
    drawTextShadowed(window, "SCORE", sf::Vector2f(labelX, currentY - 45.0f),
                     16, sf::Color(255, 195, 65), sf::Color(15, 10, 25), false);
    drawTextShadowed(window, std::to_string(score), sf::Vector2f(valX, currentY - 45.0f),
                     22, sf::Color(255, 255, 255), sf::Color(15, 10, 25), false);

    // Best Score
    drawTextShadowed(window, "BEST", sf::Vector2f(labelX, currentY + 15.0f),
                     16, sf::Color(255, 195, 65), sf::Color(15, 10, 25), false);
    drawTextShadowed(window, std::to_string(bestScore), sf::Vector2f(valX, currentY + 15.0f),
                     22, sf::Color(255, 255, 255), sf::Color(15, 10, 25), false);

    // New Best badge!
    if (isNewBest && score > 0) {
        sf::RectangleShape newBadge(sf::Vector2f(44.0f, 18.0f));
        newBadge.setOrigin(22.0f, 9.0f);
        newBadge.setPosition(labelX - 16.0f, currentY + 15.0f);
        newBadge.setFillColor(sf::Color(240, 45, 45));
        window.draw(newBadge);

        drawTextShadowed(window, "NEW", sf::Vector2f(labelX - 16.0f, currentY + 15.0f),
                         11, sf::Color(255, 255, 255), sf::Color(15, 10, 25));
    }

    // Restart prompt below card
    if (m_cardProgress >= 0.9f) {
        float promptAlpha = 180.0f + std::sin(m_pulseTimer * 6.0f) * 65.0f;
        sf::Uint8 a = static_cast<sf::Uint8>(std::clamp(promptAlpha, 0.0f, 255.0f));

        drawTextShadowed(window, "PRESS [R] OR [SPACE] TO RESTART",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, currentY + 130.0f),
                         17, sf::Color(255, 235, 120, a), sf::Color(15, 10, 25, a));

        // Quick Navigation Buttons on Game Over
        sf::FloatRect wardBounds = getWardrobeButtonBounds();
        sf::RectangleShape wardBtn(sf::Vector2f(wardBounds.width, wardBounds.height));
        wardBtn.setPosition(wardBounds.left, wardBounds.top);
        wardBtn.setFillColor(sf::Color(35, 25, 55, 220));
        wardBtn.setOutlineColor(sf::Color(255, 185, 45));
        wardBtn.setOutlineThickness(2.0f);
        window.draw(wardBtn);
        drawTextShadowed(window, "[C] WARDROBE", sf::Vector2f(wardBounds.left + wardBounds.width * 0.5f, wardBounds.top + wardBounds.height * 0.5f),
                         13, sf::Color(255, 225, 80), sf::Color(10, 10, 20));

        sf::FloatRect scoreBounds = getScoreboardButtonBounds();
        sf::RectangleShape scoreBtn(sf::Vector2f(scoreBounds.width, scoreBounds.height));
        scoreBtn.setPosition(scoreBounds.left, scoreBounds.top);
        scoreBtn.setFillColor(sf::Color(35, 25, 55, 220));
        scoreBtn.setOutlineColor(sf::Color(0, 235, 255));
        scoreBtn.setOutlineThickness(2.0f);
        window.draw(scoreBtn);
        drawTextShadowed(window, "[S] SCORES", sf::Vector2f(scoreBounds.left + scoreBounds.width * 0.5f, scoreBounds.top + scoreBounds.height * 0.5f),
                         13, sf::Color(180, 245, 255), sf::Color(10, 10, 20));
    }
}

sf::FloatRect UI::getWardrobeButtonBounds() const {
    return sf::FloatRect(45.0f, 545.0f, 180.0f, 44.0f);
}

sf::FloatRect UI::getScoreboardButtonBounds() const {
    return sf::FloatRect(255.0f, 545.0f, 180.0f, 44.0f);
}

sf::FloatRect UI::getBackButtonBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH * 0.5f - 80.0f, 620.0f, 160.0f, 42.0f);
}

sf::FloatRect UI::getHatPrevBounds() const {
    return sf::FloatRect(45.0f, 320.0f, 40.0f, 40.0f);
}

sf::FloatRect UI::getHatNextBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH - 85.0f, 320.0f, 40.0f, 40.0f);
}

sf::FloatRect UI::getOutfitPrevBounds() const {
    return sf::FloatRect(45.0f, 440.0f, 40.0f, 40.0f);
}

sf::FloatRect UI::getOutfitNextBounds() const {
    return sf::FloatRect(Config::WINDOW_WIDTH - 85.0f, 440.0f, 40.0f, 40.0f);
}

void UI::drawCosmeticsScreen(sf::RenderWindow& window, const CosmeticsManager& cosmetics, int bestScore) {
    // Backdrop Veil
    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    veil.setFillColor(sf::Color(16, 12, 32, 235));
    window.draw(veil);

    // Title
    drawTextShadowed(window, "WARDROBE & COSMETICS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 65.0f),
                     28, sf::Color(255, 215, 45), sf::Color(15, 10, 25));

    // High score indicator
    drawTextShadowed(window, "BEST SCORE: " + std::to_string(bestScore), sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 105.0f),
                     16, sf::Color(180, 240, 255), sf::Color(15, 10, 25));

    // --- HAT SECTION ---
    drawTextShadowed(window, "HEADWEAR", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 290.0f),
                     18, sf::Color(255, 185, 45), sf::Color(15, 10, 25));

    sf::FloatRect hpb = getHatPrevBounds();
    sf::RectangleShape prevHatBtn(sf::Vector2f(hpb.width, hpb.height));
    prevHatBtn.setPosition(hpb.left, hpb.top);
    prevHatBtn.setFillColor(sf::Color(45, 35, 70));
    prevHatBtn.setOutlineColor(sf::Color(255, 205, 60));
    prevHatBtn.setOutlineThickness(1.8f);
    window.draw(prevHatBtn);
    drawTextShadowed(window, "<", sf::Vector2f(hpb.left + 20.0f, hpb.top + 18.0f), 22, sf::Color::White, sf::Color::Black);

    sf::FloatRect hnb = getHatNextBounds();
    sf::RectangleShape nextHatBtn(sf::Vector2f(hnb.width, hnb.height));
    nextHatBtn.setPosition(hnb.left, hnb.top);
    nextHatBtn.setFillColor(sf::Color(45, 35, 70));
    nextHatBtn.setOutlineColor(sf::Color(255, 205, 60));
    nextHatBtn.setOutlineThickness(1.8f);
    window.draw(nextHatBtn);
    drawTextShadowed(window, ">", sf::Vector2f(hnb.left + 20.0f, hnb.top + 18.0f), 22, sf::Color::White, sf::Color::Black);

    // Current Hat Name & Status
    HatId currentHat = cosmetics.getEquippedHat();
    std::string hatName = cosmetics.getHatName(currentHat);
    drawTextShadowed(window, hatName, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 335.0f),
                     20, sf::Color(255, 255, 255), sf::Color(10, 10, 20));

    int hatReq = cosmetics.getHats()[static_cast<int>(currentHat)].requiredScore;
    if (bestScore >= hatReq) {
        drawTextShadowed(window, "[ EQUIPPED ]", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 365.0f),
                         14, sf::Color(80, 240, 120), sf::Color(10, 10, 20));
    } else {
        drawTextShadowed(window, "[ LOCKED - SCORE " + std::to_string(hatReq) + " ]",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 365.0f),
                         14, sf::Color(245, 80, 80), sf::Color(10, 10, 20));
    }

    // --- OUTFIT SECTION ---
    drawTextShadowed(window, "OUTFIT / JACKET", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 410.0f),
                     18, sf::Color(255, 185, 45), sf::Color(15, 10, 25));

    sf::FloatRect opb = getOutfitPrevBounds();
    sf::RectangleShape prevOutBtn(sf::Vector2f(opb.width, opb.height));
    prevOutBtn.setPosition(opb.left, opb.top);
    prevOutBtn.setFillColor(sf::Color(45, 35, 70));
    prevOutBtn.setOutlineColor(sf::Color(255, 205, 60));
    prevOutBtn.setOutlineThickness(1.8f);
    window.draw(prevOutBtn);
    drawTextShadowed(window, "<", sf::Vector2f(opb.left + 20.0f, opb.top + 18.0f), 22, sf::Color::White, sf::Color::Black);

    sf::FloatRect onb = getOutfitNextBounds();
    sf::RectangleShape nextOutBtn(sf::Vector2f(onb.width, onb.height));
    nextOutBtn.setPosition(onb.left, onb.top);
    nextOutBtn.setFillColor(sf::Color(45, 35, 70));
    nextOutBtn.setOutlineColor(sf::Color(255, 205, 60));
    nextOutBtn.setOutlineThickness(1.8f);
    window.draw(nextOutBtn);
    drawTextShadowed(window, ">", sf::Vector2f(onb.left + 20.0f, onb.top + 18.0f), 22, sf::Color::White, sf::Color::Black);

    // Current Outfit Name & Status
    OutfitId currentOutfit = cosmetics.getEquippedOutfit();
    std::string outName = cosmetics.getOutfitName(currentOutfit);
    drawTextShadowed(window, outName, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 455.0f),
                     20, sf::Color(255, 255, 255), sf::Color(10, 10, 20));

    int outReq = cosmetics.getOutfits()[static_cast<int>(currentOutfit)].requiredScore;
    if (bestScore >= outReq) {
        drawTextShadowed(window, "[ EQUIPPED ]", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 485.0f),
                         14, sf::Color(80, 240, 120), sf::Color(10, 10, 20));
    } else {
        drawTextShadowed(window, "[ LOCKED - SCORE " + std::to_string(outReq) + " ]",
                         sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 485.0f),
                         14, sf::Color(245, 80, 80), sf::Color(10, 10, 20));
    }

    // Help Hint
    drawTextShadowed(window, "TAP < > OR USE ARROW KEYS TO CYCLE", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 545.0f),
                     13, sf::Color(200, 210, 230), sf::Color(10, 10, 20));

    // Back Button
    sf::FloatRect bb = getBackButtonBounds();
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
    sf::RectangleShape veil(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    veil.setFillColor(sf::Color(16, 12, 32, 240));
    window.draw(veil);

    drawTextShadowed(window, "ARCADE SCOREBOARD", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 65.0f),
                     28, sf::Color(0, 240, 255), sf::Color(15, 10, 25));

    // All Time Best Card
    sf::RectangleShape bestCard(sf::Vector2f(380.0f, 95.0f));
    bestCard.setOrigin(190.0f, 47.5f);
    bestCard.setPosition(Config::WINDOW_WIDTH * 0.5f, 155.0f);
    bestCard.setFillColor(sf::Color(28, 22, 50));
    bestCard.setOutlineColor(sf::Color(255, 205, 45));
    bestCard.setOutlineThickness(2.5f);
    window.draw(bestCard);

    drawTextShadowed(window, "PERSONAL BEST RECORD", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 130.0f),
                     16, sf::Color(255, 205, 50), sf::Color(10, 10, 20));
    drawTextShadowed(window, std::to_string(bestScore) + " PTS", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 170.0f),
                     34, sf::Color(255, 255, 255), sf::Color(10, 10, 20));

    // Current Medal Status
    MedalTier tier = getTier(bestScore);
    std::string tierName = "None";
    if (tier == MedalTier::Platinum) tierName = "Platinum Trophy (40+)";
    else if (tier == MedalTier::Gold) tierName = "Gold Medal (30+)";
    else if (tier == MedalTier::Silver) tierName = "Silver Medal (20+)";
    else if (tier == MedalTier::Bronze) tierName = "Bronze Medal (10+)";

    drawTextShadowed(window, "RANK: " + tierName, sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 230.0f),
                     18, sf::Color(255, 225, 90), sf::Color(10, 10, 20));

    // Progression Checklist Table
    drawTextShadowed(window, "COSMETIC MILESTONES", sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 275.0f),
                     17, sf::Color(180, 240, 255), sf::Color(10, 10, 20));

    int milestones[] = { 1, 5, 10, 15, 20, 25, 30, 35, 40 };
    const char* names[] = {
        "Score 1: Red Headband",
        "Score 5: Aviator Goggles & Scarf",
        "Score 10: Top Hat & Bowtie",
        "Score 15: Cyber Visor & Vest",
        "Score 20: Royal Crown & Cape",
        "Score 25: Wizard Hat & Cowl",
        "Score 30: Phoenix Crown & Plumes",
        "Score 35: Astronaut & Jetpack",
        "Score 40: Platinum Celestial Mantle"
    };

    int unlockedCount = 0;
    for (int i = 0; i < 9; ++i) {
        float y = 310.0f + i * 28.0f;
        bool unl = (bestScore >= milestones[i]);
        if (unl) unlockedCount++;

        sf::Color itemCol = unl ? sf::Color(120, 255, 150) : sf::Color(150, 150, 165);
        std::string check = unl ? "[x] " : "[ ] ";
        drawTextShadowed(window, check + names[i], sf::Vector2f(50.0f, y),
                         13, itemCol, sf::Color(10, 10, 20), false);
    }

    // Completion %
    int pct = (unlockedCount * 100) / 9;
    drawTextShadowed(window, "UNLOCKED: " + std::to_string(pct) + "% (" + std::to_string(unlockedCount) + "/9)",
                     sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 580.0f),
                     15, sf::Color(255, 215, 60), sf::Color(10, 10, 20));

    // Back Button
    sf::FloatRect bb = getBackButtonBounds();
    sf::RectangleShape backBtn(sf::Vector2f(bb.width, bb.height));
    backBtn.setPosition(bb.left, bb.top);
    backBtn.setFillColor(sf::Color(55, 35, 80));
    backBtn.setOutlineColor(sf::Color(0, 235, 255));
    backBtn.setOutlineThickness(2.0f);
    window.draw(backBtn);
    drawTextShadowed(window, "[ESC] BACK", sf::Vector2f(bb.left + bb.width * 0.5f, bb.top + bb.height * 0.5f),
                     16, sf::Color::White, sf::Color::Black);
}
