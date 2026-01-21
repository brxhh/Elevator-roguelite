#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp> // Додано для SFML 3.0
#include <vector>
#include "Global.h"
#include "Entities.h"
#include "Player.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void reset();
    void spawnEnemy();

    sf::RenderWindow mWindow;
    sf::Font mFont;
    sf::Text mUiText;

    Player mPlayer;
    GameState mState;
    bool mDebugMode = false;

    std::vector<Bullet> mBullets;
    std::vector<Enemy> mEnemies;
    std::vector<MedKit> mMedkits;
    std::vector<Particle> mParticles;

    sf::Clock mGameClock, mSpawnClock, mShootClock;
    int mScore, mFloor, mNextFloorScore;
};

#endif