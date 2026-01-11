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

struct Enemy {
    sf::RectangleShape shape;
    float hp;
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
    bool fontLoaded = font.openFromFile("arial.ttf") ||
                      font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

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

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;

    sf::Text uiText(font, "");
    uiText.setCharacterSize(20);
    uiText.setFillColor(sf::Color::White);

    sf::Clock spawnClock, shootClock, gameClock;
    GameState state = GameState::Menu;

    while (window.isOpen()) {
        float dt = gameClock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (state == GameState::Menu) {
                    if (k->code == sf::Keyboard::Key::Enter) state = GameState::Playing;
                    if (k->code == sf::Keyboard::Key::Up) difficulty += 0.2f;
                    if (k->code == sf::Keyboard::Key::Down) difficulty = std::max(0.5f, difficulty - 0.2f);
                }
                else if (state == GameState::Playing) {
                    if (k->code == sf::Keyboard::Key::Escape) state = GameState::Menu;
                }
                else if (state == GameState::Shop) {
                    bool picked = false;
                    if (k->code == sf::Keyboard::Key::Num1) { pDmg += 10; picked = true; }
                    if (k->code == sf::Keyboard::Key::Num2) { pSpeed += 30; picked = true; }
                    if (k->code == sf::Keyboard::Key::Num3) { pHp = 100; picked = true; }

                    if (picked) {
                        state = GameState::Playing;
                        floor++;
                        nextFloorScore += 100 * floor;
                        enemies.clear();
                    }
                }
                else if (state == GameState::GameOver) {
                    if (k->code == sf::Keyboard::Key::Enter) {

                        pHp = 100; score = 0; floor = 1; nextFloorScore = 100;
                        enemies.clear(); bullets.clear();
                        player.setPosition({W_WIDTH / 2, W_HEIGHT / 2});
                        state = GameState::Playing;
                    }
                }
            }
        }

        if (state == GameState::Playing) {

            sf::Vector2f moveVec(0, 0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveVec.y -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveVec.y += 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveVec.x -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveVec.x += 1;

            player.move(normalize(moveVec) * pSpeed * dt);

            sf::Vector2f pos = player.getPosition();
            pos.x = std::clamp(pos.x, 15.f, (float)W_WIDTH - 15.f);
            pos.y = std::clamp(pos.y, 15.f, (float)W_HEIGHT - 15.f);
            player.setPosition(pos);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && shootClock.getElapsedTime().asSeconds() > 0.3f) {
                sf::Vector2i mPosRaw = sf::Mouse::getPosition(window);
                sf::Vector2f mPos = window.mapPixelToCoords(mPosRaw);
                sf::Vector2f dir = normalize(mPos - player.getPosition());

                Bullet b;
                b.shape.setRadius(4);
                b.shape.setOrigin({4, 4});
                b.shape.setFillColor(sf::Color::Yellow);
                b.shape.setPosition(player.getPosition());
                b.velocity = dir * 600.f;
                b.damage = pDmg;
                bullets.push_back(b);
                shootClock.restart();
            }

            if (spawnClock.getElapsedTime().asSeconds() > (1.2f / difficulty)) {
                Enemy e;
                e.shape.setSize({30, 30});
                e.shape.setOrigin({15, 15});
                e.shape.setFillColor(sf::Color::Red);
                e.shape.setPosition({(float)(rand() % W_WIDTH), -30.f});
                e.hp = 30 + (floor * 10);
                e.speed = 100 + (rand() % 50);
                enemies.push_back(e);
                spawnClock.restart();
            }

            for (size_t i = 0; i < bullets.size();) {
                bullets[i].shape.move(bullets[i].velocity * dt);
                bool hit = false;

                for (size_t j = 0; j < enemies.size(); j++) {
                    if (bullets[i].shape.getGlobalBounds().findIntersection(enemies[j].shape.getGlobalBounds())) {
                        enemies[j].hp -= bullets[i].damage;
                        hit = true;
                        if (enemies[j].hp <= 0) {
                            for (int k = 0; k < 10; k++) {
                                Particle p;
                                p.shape.setSize({4, 4});
                                p.shape.setFillColor(sf::Color::Red);
                                p.shape.setPosition(enemies[j].shape.getPosition());
                                p.velocity = {(float)(rand() % 200 - 100), (float)(rand() % 200 - 100)};
                                p.lifetime = 0.5f;
                                particles.push_back(p);
                            }
                            enemies.erase(enemies.begin() + j);
                            score += 10;
                        }
                        break;
                    }
                }

                if (hit || bullets[i].shape.getPosition().y < -50 || bullets[i].shape.getPosition().y > 650
                    || bullets[i].shape.getPosition().x < -50 || bullets[i].shape.getPosition().x > 850) {
                    bullets.erase(bullets.begin() + i);
                } else {
                    i++;
                }
            }
            for (size_t i = 0; i < enemies.size();) {
                sf::Vector2f dir = normalize(player.getPosition() - enemies[i].shape.getPosition());
                enemies[i].shape.move(dir * enemies[i].speed * dt);

                if (enemies[i].shape.getGlobalBounds().findIntersection(player.getGlobalBounds())) {
                    pHp -= 10;
                    enemies.erase(enemies.begin() + i);
                } else {
                    i++;
                }
            }

            for (size_t i = 0; i < particles.size();) {
                particles[i].lifetime -= dt;
                particles[i].shape.move(particles[i].velocity * dt);
                if (particles[i].lifetime <= 0) particles.erase(particles.begin() + i);
                else i++;
            }

            if (score >= nextFloorScore) state = GameState::Shop;
            if (pHp <= 0) state = GameState::GameOver;
        }

        window.clear(sf::Color(30, 30, 35));

        for (auto& p : particles) window.draw(p.shape);
        for (auto& b : bullets) window.draw(b.shape);
        for (auto& e : enemies) window.draw(e.shape);
        window.draw(player);

        if (fontLoaded) {
            if (state == GameState::Menu) {
                uiText.setString("SETTINGS\nDifficulty: " + std::to_string(difficulty) + "\nPress Enter to Start\nUse Arrows to change Difficulty");
                uiText.setPosition({200, 200});
            } else if (state == GameState::Playing) {
                uiText.setString("Floor: " + std::to_string(floor) + " | HP: " + std::to_string(pHp) + " | Score: " + std::to_string(score));
                uiText.setPosition({10, 10});
            } else if (state == GameState::Shop) {
                uiText.setString("SHOP (Next floor: " + std::to_string(nextFloorScore) + ")\n1: +Damage\n2: +Speed\n3: Heal HP");
                uiText.setPosition({250, 250});
            } else if (state == GameState::GameOver) {
                uiText.setString("GAME OVER! Score: " + std::to_string(score) + "\nPress Enter to Restart");
                uiText.setPosition({300, 250});
            }
            window.draw(uiText);
        }

        window.display();
    }
    return 0;
}