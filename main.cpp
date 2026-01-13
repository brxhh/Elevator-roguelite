#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <algorithm>

const int W_WIDTH = 800;
const int W_HEIGHT = 600;

enum class GameState { Menu, Playing, Shop, GameOver };
enum class EnemyType { Normal, Tank, Fast };

struct Particle {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
};

struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float damage;
};

struct MedKit {
    sf::CircleShape shape;
    int healAmount = 20;
};

struct Enemy {
    sf::RectangleShape shape;
    EnemyType type;
    float hp;
    float maxHp;
    float speed;
};

sf::Vector2f normalize(sf::Vector2f v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    return (len != 0) ? v / len : v;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({W_WIDTH, W_HEIGHT}), "Roguelike SFML 3.0");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.openFromFile("arial.ttf") || font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    sf::RectangleShape player({30, 30});
    player.setFillColor(sf::Color::Cyan);
    player.setOrigin({15, 15});
    player.setPosition({W_WIDTH / 2, W_HEIGHT / 2});

    float pSpeed = 250.f;
    float pDmg = 20.f;
    int pHp = 100;
    int score = 0;
    int floor = 1;
    int nextFloorScore = 100;
    float difficulty = 1.0f;

    float dashTimer = 0.f;
    float dashCooldown = 1.0f;
    bool isDashing = false;
    sf::Vector2f dashDir;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;
    std::vector<MedKit> medkits;

    sf::Text uiText(font, "");
    uiText.setCharacterSize(18);
    uiText.setFillColor(sf::Color::White);

    sf::Clock spawnClock, shootClock, gameClock;
    GameState state = GameState::Menu;

    while (window.isOpen()) {
        float dt = gameClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (state == GameState::Menu && k->code == sf::Keyboard::Key::Enter) state = GameState::Playing;
                if (state == GameState::Shop) {
                    bool picked = false;
                    if (k->code == sf::Keyboard::Key::Num1) { pDmg += 15; picked = true; }
                    if (k->code == sf::Keyboard::Key::Num2) { pSpeed += 40; picked = true; }
                    if (k->code == sf::Keyboard::Key::Num3) { pHp = 100; picked = true; }
                    if (picked) { state = GameState::Playing; floor++; nextFloorScore += 150; enemies.clear(); }
                }
                if (state == GameState::GameOver && k->code == sf::Keyboard::Key::Enter) {
                    pHp = 100; score = 0; floor = 1; nextFloorScore = 100; difficulty = 1.0f;
                    enemies.clear(); bullets.clear(); medkits.clear();
                    player.setPosition({W_WIDTH / 2, W_HEIGHT / 2});
                    state = GameState::Playing;
                }
            }
        }

        if (state == GameState::Playing) {
            sf::Vector2f moveVec(0, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveVec.y -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveVec.y += 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveVec.x -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveVec.x += 1;

            dashTimer -= dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && dashTimer <= 0 && (moveVec.x != 0 || moveVec.y != 0)) {
                isDashing = true;
                dashTimer = dashCooldown;
                dashDir = normalize(moveVec);
            }

            if (isDashing) {
                player.move(dashDir * pSpeed * 3.f * dt);
                if (dashTimer < dashCooldown - 0.15f) isDashing = false;
            } else {
                player.move(normalize(moveVec) * pSpeed * dt);
            }

            sf::Vector2f pPos = player.getPosition();
            player.setPosition({std::clamp(pPos.x, 15.f, 785.f), std::clamp(pPos.y, 15.f, 585.f)});

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && shootClock.getElapsedTime().asSeconds() > 0.25f) {
                sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                Bullet b;
                b.shape.setRadius(5); b.shape.setFillColor(sf::Color::Yellow); b.shape.setOrigin({5, 5});
                b.shape.setPosition(player.getPosition());
                b.velocity = normalize(mPos - player.getPosition()) * 700.f;
                b.damage = pDmg;
                bullets.push_back(b);
                shootClock.restart();
            }

            if (spawnClock.getElapsedTime().asSeconds() > (1.0f / (difficulty * 0.8f + floor * 0.2f))) {
                Enemy e;
                int typeRoll = rand() % 10;
                if (typeRoll < 6) {
                    e.type = EnemyType::Normal; e.hp = 40 + floor * 10; e.speed = 120; e.shape.setSize({30, 30}); e.shape.setFillColor(sf::Color::Red);
                } else if (typeRoll < 9) {
                    e.type = EnemyType::Fast; e.hp = 20 + floor * 5; e.speed = 220; e.shape.setSize({20, 20}); e.shape.setFillColor(sf::Color::Magenta);
                } else {
                    e.type = EnemyType::Tank; e.hp = 150 + floor * 20; e.speed = 60; e.shape.setSize({45, 45}); e.shape.setFillColor(sf::Color(150, 0, 0));
                }
                e.maxHp = e.hp;
                e.shape.setOrigin(e.shape.getSize() / 2.f);

                int side = rand() % 4;
                if (side == 0) e.shape.setPosition({(float)(rand() % W_WIDTH), -50});
                else if (side == 1) e.shape.setPosition({(float)(rand() % W_WIDTH), 650});
                else if (side == 2) e.shape.setPosition({-50, (float)(rand() % W_HEIGHT)});
                else e.shape.setPosition({850, (float)(rand() % W_HEIGHT)});

                enemies.push_back(e);
                spawnClock.restart();
            }

            for (size_t i = 0; i < bullets.size();) {
                bullets[i].shape.move(bullets[i].velocity * dt);
                bool removed = false;
                for (size_t j = 0; j < enemies.size(); j++) {
                    if (bullets[i].shape.getGlobalBounds().findIntersection(enemies[j].shape.getGlobalBounds())) {
                        enemies[j].hp -= bullets[i].damage;
                        removed = true;
                        if (enemies[j].hp <= 0) {
                            if (rand() % 100 < 20) {
                                MedKit m; m.shape.setRadius(8); m.shape.setPointCount(4);
                                m.shape.setFillColor(sf::Color::Green); m.shape.setOrigin({8,8});
                                m.shape.setPosition(enemies[j].shape.getPosition());
                                medkits.push_back(m);
                            }
                            score += 15;
                            enemies.erase(enemies.begin() + j);
                        }
                        break;
                    }
                }
                if (removed || std::abs(bullets[i].shape.getPosition().x) > 1000) bullets.erase(bullets.begin() + i);
                else i++;
            }

            for (size_t i = 0; i < enemies.size();) {
                enemies[i].shape.move(normalize(player.getPosition() - enemies[i].shape.getPosition()) * enemies[i].speed * dt);
                if (enemies[i].shape.getGlobalBounds().findIntersection(player.getGlobalBounds())) {
                    pHp -= 15; enemies.erase(enemies.begin() + i);
                } else i++;
            }

            for (size_t i = 0; i < medkits.size();) {
                if (medkits[i].shape.getGlobalBounds().findIntersection(player.getGlobalBounds())) {
                    pHp = std::min(100, pHp + 25);
                    medkits.erase(medkits.begin() + i);
                } else i++;
            }

            if (score >= nextFloorScore) state = GameState::Shop;
            if (pHp <= 0) state = GameState::GameOver;
        }

        window.clear(sf::Color(20, 20, 25));

        for (auto& m : medkits) window.draw(m.shape);
        for (auto& b : bullets) window.draw(b.shape);
        for (auto& e : enemies) {
            window.draw(e.shape);
            sf::RectangleShape hbBg({e.shape.getSize().x, 4});
            hbBg.setFillColor(sf::Color::Black);
            hbBg.setPosition({e.shape.getPosition().x - e.shape.getSize().x/2, e.shape.getPosition().y - e.shape.getSize().y/2 - 10});
            sf::RectangleShape hbFill({(e.hp / e.maxHp) * e.shape.getSize().x, 4});
            hbFill.setFillColor(sf::Color::Green);
            hbFill.setPosition(hbBg.getPosition());
            window.draw(hbBg); window.draw(hbFill);
        }
        window.draw(player);

        if (fontLoaded) {
            if (state == GameState::Menu) uiText.setString("ROGUE SQUARE\nPress Enter to Start\nDifficulty: " + std::to_string(difficulty));
            else if (state == GameState::Playing) uiText.setString("Floor: " + std::to_string(floor) + " | HP: " + std::to_string(pHp) + " | Score: " + std::to_string(score) + (dashTimer <= 0 ? " | DASH READY" : ""));
            else if (state == GameState::Shop) uiText.setString("FLOOR CLEAR! Pick Upgrade:\n1: +Damage\n2: +Speed\n3: Full Heal");
            else if (state == GameState::GameOver) uiText.setString("DIED ON FLOOR " + std::to_string(floor) + "\nFinal Score: " + std::to_string(score) + "\nPress Enter to restart");

            uiText.setPosition({10, 10});
            window.draw(uiText);
        }
        window.display();
    }
    return 0;
}