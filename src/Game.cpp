#include "../include/Game.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <filesystem>

Game::Game() :
    mWindow(sf::VideoMode({(unsigned int)W_WIDTH, (unsigned int)W_HEIGHT}), "Roguelite SFML 3.0"),
    mUiText(mFont, "", 18)
{
    mWindow.setFramerateLimit(60);

    if (!mFont.openFromFile("arial.ttf")) {
        if (!mFont.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            std::cerr << "CRITICAL ERROR: arial.ttf not found!" << std::endl;
        }
    }

    mUiText.setFillColor(sf::Color::White);
    reset();
}

void Game::reset() {
    mPlayer.reset();
    mScore = 0; mFloor = 1; mNextFloorScore = 100;
    mState = GameState::Menu;
    mEnemies.clear(); mBullets.clear(); mMedkits.clear(); mParticles.clear();
}

void Game::spawnEnemy() {
    Enemy e;
    int roll = rand() % 10;
    if (roll < 6) { e.type = EnemyType::Normal; e.hp = 40 + mFloor * 10; e.speed = 120; e.shape.setSize({30, 30}); e.shape.setFillColor(sf::Color::Red); }
    else if (roll < 9) { e.type = EnemyType::Fast; e.hp = 20 + mFloor * 5; e.speed = 220; e.shape.setSize({20, 20}); e.shape.setFillColor(sf::Color::Magenta); }
    else { e.type = EnemyType::Tank; e.hp = 150 + mFloor * 20; e.speed = 60; e.shape.setSize({45, 45}); e.shape.setFillColor(sf::Color(150, 0, 0)); }

    e.maxHp = e.hp;
    e.shape.setOrigin(e.shape.getSize() / 2.f);
    e.shape.setPosition({(float)(rand() % W_WIDTH), -50.f});
    mEnemies.push_back(e);
}

void Game::update(float dt) {
    if (mState != GameState::Playing) return;

    mPlayer.handleInput(dt);

    // Shooting logic
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && mShootClock.getElapsedTime().asSeconds() > 0.25f) {
        sf::Vector2f mPos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        Bullet b;
        b.shape.setRadius(5); b.shape.setFillColor(sf::Color::Yellow); b.shape.setOrigin({5, 5});
        b.shape.setPosition(mPlayer.shape.getPosition());
        b.velocity = normalize(mPos - mPlayer.shape.getPosition()) * 700.f;
        b.damage = mPlayer.damage;
        mBullets.push_back(b);
        mShootClock.restart();
    }

    // Spawn
    if (mSpawnClock.getElapsedTime().asSeconds() > 1.5f / (1.0f + mFloor * 0.1f)) {
        spawnEnemy();
        mSpawnClock.restart();
    }

    // Bullet-Enemy collisions
    for (size_t i = 0; i < mBullets.size();) {
        mBullets[i].shape.move(mBullets[i].velocity * dt);
        bool hit = false;
        for (size_t j = 0; j < mEnemies.size(); j++) {
            if (mBullets[i].shape.getGlobalBounds().findIntersection(mEnemies[j].shape.getGlobalBounds())) {
                mEnemies[j].hp -= mBullets[i].damage;
                hit = true;
                if (mEnemies[j].hp <= 0) {
                    // Spawn particles
                    for(int k=0; k<8; k++) {
                        Particle p; p.shape.setSize({4,4}); p.shape.setFillColor(mEnemies[j].shape.getFillColor());
                        p.shape.setPosition(mEnemies[j].shape.getPosition());
                        p.velocity = {(float)(rand()%200-100), (float)(rand()%200-100)};
                        p.lifetime = 0.6f; mParticles.push_back(p);
                    }
                    // Loot
                    if (rand() % 100 < 20) {
                        MedKit m; m.shape.setRadius(8); m.shape.setFillColor(sf::Color::Green);
                        m.shape.setOrigin({8,8}); m.shape.setPosition(mEnemies[j].shape.getPosition());
                        mMedkits.push_back(m);
                    }
                    mScore += 15; mEnemies.erase(mEnemies.begin() + j);
                }
                break;
            }
        }
        if (hit || std::abs(mBullets[i].shape.getPosition().x) > 1000) mBullets.erase(mBullets.begin() + i);
        else i++;
    }

    // Particles update
    for (size_t i = 0; i < mParticles.size();) {
        mParticles[i].lifetime -= dt;
        mParticles[i].shape.move(mParticles[i].velocity * dt);
        if (mParticles[i].lifetime <= 0) mParticles.erase(mParticles.begin() + i);
        else i++;
    }

    // Enemy movement and player collision
    for (size_t i = 0; i < mEnemies.size();) {
        mEnemies[i].shape.move(normalize(mPlayer.shape.getPosition() - mEnemies[i].shape.getPosition()) * mEnemies[i].speed * dt);
        if (mEnemies[i].shape.getGlobalBounds().findIntersection(mPlayer.shape.getGlobalBounds())) {
            mPlayer.hp -= 15; mEnemies.erase(mEnemies.begin() + i);
        } else i++;
    }

    // Medkit collection
    for (size_t i = 0; i < mMedkits.size();) {
        if (mMedkits[i].shape.getGlobalBounds().findIntersection(mPlayer.shape.getGlobalBounds())) {
            mPlayer.hp = std::min(100, mPlayer.hp + 25);
            mMedkits.erase(mMedkits.begin() + i);
        } else i++;
    }

    if (mScore >= mNextFloorScore) mState = GameState::Shop;
    if (mPlayer.hp <= 0) mState = GameState::GameOver;
}

void Game::render() {
    mWindow.clear(sf::Color(20, 20, 25));

    for (auto& p : mParticles) mWindow.draw(p.shape);
    for (auto& m : mMedkits) mWindow.draw(m.shape);
    for (auto& b : mBullets) mWindow.draw(b.shape);
    for (auto& e : mEnemies) {
        mWindow.draw(e.shape);
        // Health bars
        sf::RectangleShape hbBg({e.shape.getSize().x, 4});
        hbBg.setFillColor(sf::Color::Black);
        hbBg.setPosition({e.shape.getPosition().x - e.shape.getSize().x/2, e.shape.getPosition().y - e.shape.getSize().y/2 - 10});
        sf::RectangleShape hbFill({(e.hp / e.maxHp) * e.shape.getSize().x, 4});
        hbFill.setFillColor(sf::Color::Green);
        hbFill.setPosition(hbBg.getPosition());
        mWindow.draw(hbBg); mWindow.draw(hbFill);
    }
    mWindow.draw(mPlayer.shape);

    // UI Logic
    if (mState == GameState::Menu) mUiText.setString("ROGUE SQUARE\nENTER: Start | ESC: Pause");
    else if (mState == GameState::Playing) mUiText.setString("Floor: " + std::to_string(mFloor) + " | HP: " + std::to_string(mPlayer.hp) + " | Score: " + std::to_string(mScore));
    else if (mState == GameState::Shop) mUiText.setString("SHOP: 1: +Dmg | 2: +Spd | 3: Heal");
    else mUiText.setString("GAME OVER\nENTER: Restart");

    mUiText.setPosition({10, 10});
    mWindow.draw(mUiText);
    mWindow.display();
}

void Game::processEvents() {
    while (const std::optional event = mWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) mWindow.close();
        if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
            if (k->code == sf::Keyboard::Key::Escape) mState = GameState::Menu;
            if (k->code == sf::Keyboard::Key::Enter && mState == GameState::Menu) {
                mState = GameState::Playing;
                mGameClock.restart();
            }
            if (mState == GameState::Shop) {
                bool picked = false;
                if (k->code == sf::Keyboard::Key::Num1) { mPlayer.damage += 15; picked = true; }
                else if (k->code == sf::Keyboard::Key::Num2) { mPlayer.speed += 40; picked = true; }
                else if (k->code == sf::Keyboard::Key::Num3) { mPlayer.hp = 100; picked = true; }
                if (picked) {
                    mState = GameState::Playing;
                    mFloor++; mNextFloorScore += 150;
                    mEnemies.clear(); mBullets.clear();
                    mGameClock.restart();
                }
            }
            if (k->code == sf::Keyboard::Key::Enter && mState == GameState::GameOver) reset();
        }
    }
}

void Game::run() {
    while (mWindow.isOpen()) {
        processEvents();
        update(mGameClock.restart().asSeconds());
        render();
    }
}