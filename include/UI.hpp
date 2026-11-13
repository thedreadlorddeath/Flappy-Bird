#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "Cosmetics.hpp"

enum class MedalTier {
    None,
    Bronze,
    Silver,
    Gold,
    Platinum
};

class UI {
public:
    UI();

    void update(float dt);
    void triggerScorePop();

    void drawGetReady(sf::RenderWindow& window);
    void drawHUD(sf::RenderWindow& window, int score);
    void drawPauseOverlay(sf::RenderWindow& window);
    void drawGameOverCard(sf::RenderWindow& window, int score, int bestScore, bool isNewBest);

    // New Screens: Cosmetics Wardrobe and Scoreboard
    void drawCosmeticsScreen(sf::RenderWindow& window, const CosmeticsManager& cosmetics, int bestScore);
    void drawScoreboardScreen(sf::RenderWindow& window, const CosmeticsManager& cosmetics, int bestScore);

    // Button Bounding Boxes for Click / Touch
    sf::FloatRect getWardrobeButtonBounds() const;
    sf::FloatRect getScoreboardButtonBounds() const;
    sf::FloatRect getBackButtonBounds() const;
    sf::FloatRect getHatPrevBounds() const;
    sf::FloatRect getHatNextBounds() const;
    sf::FloatRect getOutfitPrevBounds() const;
    sf::FloatRect getOutfitNextBounds() const;

    void reset();

private:
    void loadFont();
    void drawMedal(sf::RenderWindow& window, sf::Vector2f center, MedalTier tier);
    void drawTextShadowed(sf::RenderWindow& window, const std::string& str,
                          sf::Vector2f position, unsigned int size,
                          sf::Color fill, sf::Color shadow,
                          bool centered = true);

    sf::Font m_font;
    bool m_fontLoaded;

    // Juice: Score Pop Animation
    float m_scoreScale;

    // Get Ready pulse timer
    float m_pulseTimer;

    // Game Over Card slide-in animation
    float m_cardProgress;
};
