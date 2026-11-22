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
    , m_globalTime(0.0f)
    , m_logicalHeight(Config::WINDOW_HEIGHT)
    , m_currentStage(0)
    , m_hasReached999(false) {
    m_window.setFramerateLimit(60);

    sf::Vector2u winSize = m_window.getSize();
    updateViewLetterbox(winSize.x, winSize.y);

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

void Game::updateViewLetterbox(unsigned int width, unsigned int height) {
    if (width == 0 || height == 0) return;

    // TRUE EDGE-TO-EDGE FULLSCREEN (NO BLACK BARS)
    // Logical width is fixed at 480.0f.
    // Logical height is computed dynamically from window aspect ratio so the entire phone display is covered.
    float logicalHeight = 480.0f * (static_cast<float>(height) / static_cast<float>(width));
    m_logicalHeight = std::max(640.0f, logicalHeight);

    m_defaultView.setSize(Config::WINDOW_WIDTH, m_logicalHeight);
    m_defaultView.setCenter(Config::WINDOW_WIDTH * 0.5f, m_logicalHeight * 0.5f);
    m_defaultView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

    m_window.setView(m_defaultView);

    // Update screen height across game systems
    m_background.setScreenHeight(m_logicalHeight);
    m_pipeManager.setPlayHeight(m_logicalHeight - Config::GROUND_HEIGHT);
}

void Game::handleScreenClicks(sf::Vector2f clickPos) {
    if (m_state == GameState::Cosmetics) {
        if (m_ui.getBackButtonBounds(m_logicalHeight).contains(clickPos)) {
            m_state = m_previousState;
        } else if (m_ui.getHatPrevBounds().contains(clickPos)) {
            m_cosmetics.cyclePrevHat(m_bestScore);
        } else if (m_ui.getHatNextBounds().contains(clickPos)) {
            m_cosmetics.cycleNextHat(m_bestScore);
        } else if (m_ui.getOutfitPrevBounds().contains(clickPos)) {
            m_cosmetics.cyclePrevOutfit(m_bestScore);
        } else if (m_ui.getOutfitNextBounds().contains(clickPos)) {
            m_cosmetics.cycleNextOutfit(m_bestScore);
        } else if (m_bestScore >= 999 && m_ui.getApotheosisToggleBounds().contains(clickPos)) {
            m_cosmetics.toggleApotheosisMode(m_bestScore);
        }
        return;
    }

    if (m_state == GameState::Scoreboard) {
        if (m_ui.getBackButtonBounds(m_logicalHeight).contains(clickPos)) {
            m_state = m_previousState;
        }
        return;
    }

    if (m_state == GameState::GetReady || m_state == GameState::GameOver) {
        if (m_ui.getWardrobeButtonBounds(m_logicalHeight).contains(clickPos)) {
            m_previousState = m_state;
            m_state = GameState::Cosmetics;
            return;
        }
        if (m_ui.getScoreboardButtonBounds(m_logicalHeight).contains(clickPos)) {
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
            } else if (event.key.code == sf::Keyboard::T) {
                // Toggle Apotheosis visual mode if unlocked
                if (m_state == GameState::Cosmetics && m_bestScore >= 999) {
                    m_cosmetics.toggleApotheosisMode(m_bestScore);
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
    m_currentStage = 0;
    m_hasReached999 = false;
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
        m_pipeManager.update(dt, m_score);

        if (m_pipeManager.checkScored(m_bird.getPosition().x)) {
            m_score = std::min(m_score + 1, Config::MAX_SCORE);
            m_ui.triggerScorePop();
            m_soundGen.playScore();

            // Check stage advancement
            int newStage = Config::getStageIndex(m_score);
            if (newStage > m_currentStage) {
                m_currentStage = newStage;
                m_ui.triggerStageBanner(m_currentStage);
                m_soundGen.playMedal();
            }

            // Check Score 999 Finale
            if (m_score >= Config::MAX_SCORE && !m_hasReached999) {
                m_hasReached999 = true;
                m_ui.triggerApotheosisBanner();
                m_particleSystem.emitImpactParticles(m_bird.getPosition());
                m_particleSystem.triggerFlash(1.0f);
                m_soundGen.playMedal();
            }
        }

        if (m_pipeManager.checkCollision(m_bird.getPosition(), m_bird.getHitRadius())) {
            onCollision();
        }

        // Check ground collision
        float playHeight = m_logicalHeight - Config::GROUND_HEIGHT;
        if (m_bird.getPosition().y + m_bird.getHitRadius() >= playHeight) {
            m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, playHeight - m_bird.getHitRadius()));
            onCollision();
        }

        // Check ceiling clamp
        if (m_bird.getPosition().y - m_bird.getHitRadius() < 0.0f) {
            m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, m_bird.getHitRadius()));
        }
    } else if (m_state == GameState::GameOver) {
        float playHeight = m_logicalHeight - Config::GROUND_HEIGHT;
        if (m_bird.getPosition().y + m_bird.getHitRadius() < playHeight) {
            m_bird.update(dt);
            if (m_bird.getPosition().y + m_bird.getHitRadius() >= playHeight) {
                m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, playHeight - m_bird.getHitRadius()));
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

    // 1. Dynamic 10-Stage Gradient Sky
    m_background.drawSky(m_window, m_score);

    // 2. Parallax Layers & Themed Decorative Items
    m_background.drawParallax(m_window, m_score);

    // 3. Themed Obstacles with dynamic oscillation & speed
    m_pipeManager.draw(m_window, m_score, m_globalTime);

    // 4. Themed Ground Strip
    m_background.drawGround(m_window, m_score);

    // 5. Particles
    m_particleSystem.draw(m_window);

    // 6. Bird with equipped cosmetics, auras, & apotheosis effects
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
        m_ui.drawHUD(m_window, m_score, m_bestScore);
    } else if (m_state == GameState::Paused) {
        m_ui.drawHUD(m_window, m_score, m_bestScore);
        m_ui.drawPauseOverlay(m_window);
    } else if (m_state == GameState::GameOver) {
        m_ui.drawGameOverCard(m_window, m_score, m_bestScore, m_isNewBest);
    } else if (m_state == GameState::Cosmetics) {
        // Floating preview bird in Wardrobe
        Bird previewBird;
        float bob = std::sin(m_globalTime * 4.0f) * 6.0f;
        previewBird.setPosition(sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 185.0f + bob));
        previewBird.draw(m_window, &m_cosmetics, m_globalTime);

        m_ui.drawCosmeticsScreen(m_window, m_cosmetics, m_bestScore);
    } else if (m_state == GameState::Scoreboard) {
        m_ui.drawScoreboardScreen(m_window, m_cosmetics, m_bestScore);
    }

    m_window.display();
}
