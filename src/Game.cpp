#include "Game.hpp"
#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

Game::Game()
    : m_window(sf::VideoMode(static_cast<unsigned int>(Config::WINDOW_WIDTH),
                             static_cast<unsigned int>(Config::WINDOW_HEIGHT)),
               "Flappy Bird - Sunset Arcade",
               sf::Style::Titlebar | sf::Style::Close)
    , m_state(GameState::GetReady)
    , m_score(0)
    , m_bestScore(0)
    , m_isNewBest(false) {
    m_window.setFramerateLimit(60);
    m_defaultView = m_window.getDefaultView();

    loadHighScore();
}

void Game::loadHighScore() {
    m_bestScore = 0;
    std::ifstream file(Config::HIGHSCORE_FILE);
    if (file.is_open()) {
        file >> m_bestScore;
        if (file.fail() || m_bestScore < 0) {
            m_bestScore = 0;
        }
        file.close();
    }
}

void Game::saveHighScore() {
    std::ofstream file(Config::HIGHSCORE_FILE);
    if (file.is_open()) {
        file << m_bestScore << std::endl;
        file.close();
    }
}

void Game::run() {
    sf::Clock clock;

    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        // Prevent large time steps if window is dragged or framedrops occur
        dt = std::min(dt, 0.05f);

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                m_window.close();
            } else if (event.key.code == sf::Keyboard::P) {
                // Pause toggle
                if (m_state == GameState::Playing) {
                    m_state = GameState::Paused;
                } else if (m_state == GameState::Paused) {
                    m_state = GameState::Playing;
                }
            } else if (event.key.code == sf::Keyboard::R) {
                // Instant restart on Game Over
                if (m_state == GameState::GameOver) {
                    restartGame();
                }
            } else if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Up) {
                flapAction();
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                flapAction();
            }
        } else if (event.type == sf::Event::TouchBegan) {
            // Android mobile touch support
            flapAction();
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

    // Juice: punchy screen shake, bright flash, burst of feathers
    m_particleSystem.triggerShake(1.0f);
    m_particleSystem.triggerFlash(1.0f);
    m_particleSystem.emitImpactParticles(m_bird.getPosition());

    m_soundGen.playHit();

    // Check high score
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

    m_ui.update(dt);
    m_particleSystem.update(dt);

    if (m_state == GameState::GetReady) {
        m_bird.updateBobbing(dt);
        m_background.update(dt, m_score);
    } else if (m_state == GameState::Playing) {
        m_bird.update(dt);
        m_background.update(dt, m_score);
        m_pipeManager.update(dt);

        // Check if scored passing a pipe
        if (m_pipeManager.checkScored(m_bird.getPosition().x)) {
            m_score++;
            m_ui.triggerScorePop();
            m_soundGen.playScore();
        }

        // Check pipe collisions
        if (m_pipeManager.checkCollision(m_bird.getPosition(), m_bird.getHitRadius())) {
            onCollision();
        }

        // Check ground collision
        if (m_bird.getPosition().y + m_bird.getHitRadius() >= Config::PLAY_HEIGHT) {
            m_bird.setPosition(sf::Vector2f(m_bird.getPosition().x, Config::PLAY_HEIGHT - m_bird.getHitRadius()));
            onCollision();
        }
    } else if (m_state == GameState::GameOver) {
        // Allow bird to fall to ground if still in air
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

    // Apply juice: Screen Shake camera offset
    sf::Vector2f shakeOffset = m_particleSystem.getShakeOffset();
    sf::View activeView = m_defaultView;
    activeView.setCenter(m_defaultView.getCenter() + shakeOffset);
    m_window.setView(activeView);

    // 1. Dynamic Gradient Sky (shifts with score)
    m_background.drawSky(m_window, m_score);

    // 2. Parallax Mountain & Cloud Layers
    m_background.drawParallax(m_window);

    // 3. Pipes (glossy 3D emerald & cyan rimmed)
    m_pipeManager.draw(m_window);

    // 4. Textured Foreground Ground
    m_background.drawGround(m_window);

    // 5. Feather & Sparkle Particles
    m_particleSystem.draw(m_window);

    // 6. Flappy Bird (procedural shapes with dynamic tilt)
    m_bird.draw(m_window);

    // 7. Impact Screen Flash
    m_particleSystem.drawFlash(m_window);

    // Reset view for static UI elements
    m_window.setView(m_defaultView);

    // 8. Game State UI & HUD
    if (m_state == GameState::GetReady) {
        m_ui.drawGetReady(m_window);
    } else if (m_state == GameState::Playing) {
        m_ui.drawHUD(m_window, m_score);
    } else if (m_state == GameState::Paused) {
        m_ui.drawHUD(m_window, m_score);
        m_ui.drawPauseOverlay(m_window);
    } else if (m_state == GameState::GameOver) {
        m_ui.drawGameOverCard(m_window, m_score, m_bestScore, m_isNewBest);
    }

    m_window.display();
}
