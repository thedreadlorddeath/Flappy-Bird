#include "Game.hpp"
#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

Game::Game()
#if defined(__ANDROID__)
    : m_window(sf::VideoMode::getDesktopMode(),
               "Flappy Bird - Sunset Arcade",
               sf::Style::Default)
#else
    : m_window(sf::VideoMode(static_cast<unsigned int>(Config::WINDOW_WIDTH),
                             static_cast<unsigned int>(Config::WINDOW_HEIGHT)),
               "Flappy Bird - Sunset Arcade",
               sf::Style::Titlebar | sf::Style::Close)
#endif
    , m_state(GameState::GetReady)
    , m_previousState(GameState::GetReady)
    , m_score(0)
    , m_bestScore(0)
    , m_isNewBest(false)
    , m_globalTime(0.0f) {
    m_window.setFramerateLimit(60);

    m_defaultView.setSize(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    m_defaultView.setCenter(Config::WINDOW_WIDTH * 0.5f, Config::WINDOW_HEIGHT * 0.5f);

    sf::Vector2u winSize = m_window.getSize();
    updateViewLetterbox(winSize.x, winSize.y);
    m_window.setView(m_defaultView);

    loadHighScore();
}

void Game::loadHighScore() {
    m_bestScore = 0;
    try {
        std::ifstream file(Config::getHighScoreFilePath());
        if (file.is_open()) {
            file >> m_bestScore;
            if (file.fail() || m_bestScore < 0) {
                m_bestScore = 0;
            }
            file.close();
        }
    } catch (...) {
        m_bestScore = 0;
    }
}

void Game::saveHighScore() {
    try {
        std::ofstream file(Config::getHighScoreFilePath());
        if (file.is_open()) {
            file << m_bestScore << std::endl;
            file.close();
        }
    } catch (...) {
        // Silently ignore save errors
    }
}

void Game::run() {
    sf::Clock clock;

    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        processEvents();
        update(dt);
        render();
    }
}

void Game::handleScreenClicks(sf::Vector2f clickPos) {
    if (m_state == GameState::Cosmetics) {
        if (m_ui.getBackButtonBounds().contains(clickPos)) {
            m_state = m_previousState;
        } else if (m_ui.getHatPrevBounds().contains(clickPos)) {
            m_cosmetics.cyclePrevHat(m_bestScore);
        } else if (m_ui.getHatNextBounds().contains(clickPos)) {
            m_cosmetics.cycleNextHat(m_bestScore);
        } else if (m_ui.getOutfitPrevBounds().contains(clickPos)) {
            m_cosmetics.cyclePrevOutfit(m_bestScore);
        } else if (m_ui.getOutfitNextBounds().contains(clickPos)) {
            m_cosmetics.cycleNextOutfit(m_bestScore);
        }
        return;
    }

    if (m_state == GameState::Scoreboard) {
        if (m_ui.getBackButtonBounds().contains(clickPos)) {
            m_state = m_previousState;
        }
        return;
    }

    if (m_state == GameState::GetReady || m_state == GameState::GameOver) {
        if (m_ui.getWardrobeButtonBounds().contains(clickPos)) {
            m_previousState = m_state;
            m_state = GameState::Cosmetics;
            return;
        }
        if (m_ui.getScoreboardButtonBounds().contains(clickPos)) {
            m_previousState = m_state;
            m_state = GameState::Scoreboard;
            return;
        }
    }

    // Default: Flap or start
    flapAction();
}

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                if (m_state == GameState::Cosmetics || m_state == GameState::Scoreboard) {
                    m_state = m_previousState;
                } else {
                    m_window.close();
                }
            } else if (event.key.code == sf::Keyboard::C) {
                // Open Wardrobe / Cosmetics
                if (m_state == GameState::GetReady || m_state == GameState::GameOver) {
                    m_previousState = m_state;
                    m_state = GameState::Cosmetics;
                } else if (m_state == GameState::Cosmetics) {
                    m_state = m_previousState;
                }
            } else if (event.key.code == sf::Keyboard::S) {
                // Open Scoreboard
                if (m_state == GameState::GetReady || m_state == GameState::GameOver) {
                    m_previousState = m_state;
                    m_state = GameState::Scoreboard;
                } else if (m_state == GameState::Scoreboard) {
                    m_state = m_previousState;
                }
            } else if (event.key.code == sf::Keyboard::Left) {
                if (m_state == GameState::Cosmetics) {
                    m_cosmetics.cyclePrevHat(m_bestScore);
                }
            } else if (event.key.code == sf::Keyboard::Right) {
                if (m_state == GameState::Cosmetics) {
                    m_cosmetics.cycleNextHat(m_bestScore);
                }
            } else if (event.key.code == sf::Keyboard::Down) {
                if (m_state == GameState::Cosmetics) {
                    m_cosmetics.cycleNextOutfit(m_bestScore);
                }
            } else if (event.key.code == sf::Keyboard::Up) {
                if (m_state == GameState::Cosmetics) {
                    m_cosmetics.cyclePrevOutfit(m_bestScore);
                } else {
                    flapAction();
                }
            } else if (event.key.code == sf::Keyboard::P) {
                if (m_state == GameState::Playing) {
                    m_state = GameState::Paused;
                } else if (m_state == GameState::Paused) {
                    m_state = GameState::Playing;
                }
            } else if (event.key.code == sf::Keyboard::R) {
                if (m_state == GameState::GameOver) {
                    restartGame();
                }
            } else if (event.key.code == sf::Keyboard::Space) {
                if (m_state == GameState::Cosmetics || m_state == GameState::Scoreboard) {
                    m_state = m_previousState;
                } else {
                    flapAction();
                }
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clickPos = m_window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y), m_defaultView);
                handleScreenClicks(clickPos);
            }
        } else if (event.type == sf::Event::TouchBegan) {
            sf::Vector2f clickPos = m_window.mapPixelToCoords(sf::Vector2i(event.touch.x, event.touch.y), m_defaultView);
            handleScreenClicks(clickPos);
        } else if (event.type == sf::Event::Resized) {
            updateViewLetterbox(event.size.width, event.size.height);
        }
    }
}

void Game::updateViewLetterbox(unsigned int width, unsigned int height) {
    if (width == 0 || height == 0) return;

    float windowRatio = static_cast<float>(width) / static_cast<float>(height);
    float viewRatio = Config::WINDOW_WIDTH / Config::WINDOW_HEIGHT;
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio >= viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX = (1.0f - sizeX) * 0.5f;
    } else {
        sizeY = windowRatio / viewRatio;
        posY = (1.0f - sizeY) * 0.5f;
    }

    m_defaultView.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}

void Game::flapAction() {
    if (m_state == GameState::GetReady) {
        m_state = GameState::Playing;
        m_bird.flap();
        m_particleSystem.emitFlapParticles(m_bird.getPosition());
        m_soundGen.playFlap();
    } else if (m_state == GameState::Playing) {
        m_bird.flap();
        m_particleSystem.emitFlapParticles(m_bird.getPosition());
        m_soundGen.playFlap();
    } else if (m_state == GameState::GameOver) {
        restartGame();
    }
}

void Game::onCollision() {
    if (m_state != GameState::Playing) return;

    m_state = GameState::GameOver;
    m_bird.setAlive(false);

    m_particleSystem.triggerShake(1.0f);
    m_particleSystem.triggerFlash(1.0f);
    m_particleSystem.emitImpactParticles(m_bird.getPosition());

    m_soundGen.playHit();

    if (m_score > m_bestScore) {
        m_bestScore = m_score;
        m_isNewBest = true;
        saveHighScore();
        m_soundGen.playMedal();
    } else {
        m_isNewBest = false;
    }
}

void Game::restartGame() {
    m_state = GameState::GetReady;
    m_score = 0;
    m_isNewBest = false;

    m_bird.reset();
    m_pipeManager.reset();
    m_background.reset();
    m_particleSystem.reset();
    m_ui.reset();
}

void Game::update(float dt) {
    if (m_state == GameState::Paused) {
        return;
    }

    m_globalTime += dt;
    m_ui.update(dt);
    m_particleSystem.update(dt);

    if (m_state == GameState::GetReady || m_state == GameState::Cosmetics || m_state == GameState::Scoreboard) {
        m_bird.updateBobbing(dt);
        m_background.update(dt, m_score);
    } else if (m_state == GameState::Playing) {
        m_bird.update(dt);
        m_background.update(dt, m_score);
        m_pipeManager.update(dt);

        if (m_pipeManager.checkScored(m_bird.getPosition().x)) {
            m_score++;
            m_ui.triggerScorePop();
            m_soundGen.playScore();
        }

        if (m_pipeManager.checkCollision(m_bird.getPosition(), m_bird.getHitRadius())) {
            onCollision();
        }

        if (m_bird.getPosition().y + m_bird.getHitRadius() >= Config::PLAY_HEIGHT) {
            m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, Config::PLAY_HEIGHT - m_bird.getHitRadius()));
            onCollision();
        }
    } else if (m_state == GameState::GameOver) {
        if (m_bird.getPosition().y + m_bird.getHitRadius() < Config::PLAY_HEIGHT) {
            m_bird.update(dt);
            if (m_bird.getPosition().y + m_bird.getHitRadius() >= Config::PLAY_HEIGHT) {
                m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, Config::PLAY_HEIGHT - m_bird.getHitRadius()));
            }
        }
    }
}

void Game::render() {
    m_window.clear();

    sf::Vector2f shakeOffset = m_particleSystem.getShakeOffset();
    sf::View activeView = m_defaultView;
    activeView.setCenter(m_defaultView.getCenter() + shakeOffset);
    m_window.setView(activeView);

    // 1. Dynamic Gradient Sky
    m_background.drawSky(m_window, m_score);

    // 2. Parallax Layers
    m_background.drawParallax(m_window);

    // 3. Pipes
    m_pipeManager.draw(m_window);

    // 4. Ground Strip
    m_background.drawGround(m_window);

    // 5. Particles
    m_particleSystem.draw(m_window);

    // 6. Bird with equipped cosmetics!
    if (m_state != GameState::Cosmetics) {
        m_bird.draw(m_window, &m_cosmetics, m_globalTime);
    }

    // 7. Impact Screen Flash
    m_particleSystem.drawFlash(m_window);

    // Reset view for static UI
    m_window.setView(m_defaultView);

    // 8. Game State UI & Screens
    if (m_state == GameState::GetReady) {
        m_ui.drawGetReady(m_window);
    } else if (m_state == GameState::Playing) {
        m_ui.drawHUD(m_window, m_score);
    } else if (m_state == GameState::Paused) {
        m_ui.drawHUD(m_window, m_score);
        m_ui.drawPauseOverlay(m_window);
    } else if (m_state == GameState::GameOver) {
        m_ui.drawGameOverCard(m_window, m_score, m_bestScore, m_isNewBest);
    } else if (m_state == GameState::Cosmetics) {
        // Floating preview bird in Wardrobe
        Bird previewBird;
        float bob = std::sin(m_globalTime * 4.0f) * 6.0f;
        previewBird.setPosition(sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 195.0f + bob));
        previewBird.draw(m_window, &m_cosmetics, m_globalTime);

        m_ui.drawCosmeticsScreen(m_window, m_cosmetics, m_bestScore);
    } else if (m_state == GameState::Scoreboard) {
        m_ui.drawScoreboardScreen(m_window, m_cosmetics, m_bestScore);
    }

    m_window.display();
}
