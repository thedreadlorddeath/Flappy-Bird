#pragma once

#include <SFML/Graphics.hpp>
#include "Bird.hpp"
#include "PipeManager.hpp"
#include "Background.hpp"
#include "ParticleSystem.hpp"
#include "SoundGenerator.hpp"
#include "UI.hpp"
#include "Cosmetics.hpp"

enum class GameState {
    GetReady,
    Playing,
    Paused,
    GameOver,
    Cosmetics,
    Scoreboard
};

class Game {
public:
    Game();
    ~Game() = default;

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void updateViewLetterbox(unsigned int width, unsigned int height);

    void flapAction();
    void handleScreenClicks(sf::Vector2f clickPos);
    void onCollision();
    void restartGame();

    void loadHighScore();
    void saveHighScore();

    sf::RenderWindow m_window;
    sf::View m_defaultView;

    GameState m_state;
    GameState m_previousState;
    int m_score;
    int m_bestScore;
    bool m_isNewBest;
    float m_globalTime;

    // Core Game Systems
    Bird m_bird;
    PipeManager m_pipeManager;
    Background m_background;
    ParticleSystem m_particleSystem;
    SoundGenerator m_soundGen;
    UI m_ui;
    CosmeticsManager m_cosmetics;
};
