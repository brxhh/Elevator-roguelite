#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PLAYER_SPEED = 250.f;
const float BULLET_SPEED = 600.f;
const float ENEMY_SPEED_BASE = 110.f;

struct Medkit {
    sf::RectangleShape shape;
    explicit Medkit(sf::Vector2f pos) {
        shape.setSize({15.f, 15.f});
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin({7.5f, 7.5f});
        shape.setPosition(pos);
    }
};

struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    Bullet(float x, float y, float dx, float dy) {
        shape.setRadius(4.f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setOrigin({4.f, 4.f});
        shape.setPosition({x, y});
        velocity = {dx, dy};
    }
};

struct Enemy {
    sf::RectangleShape shape;
    float hp;
    float speed;
    Enemy(float x, float y, int wave) {
        shape.setSize({30.f, 30.f});
        shape.setFillColor(sf::Color::Red);
        shape.setOrigin({15.f, 15.f});
        shape.setPosition({x, y});
        hp = 30.f + (wave * 10.f);
        speed = ENEMY_SPEED_BASE + (std::rand() % 40);
    }
};

sf::Vector2f normalize(sf::Vector2f source) {
    float length = std::sqrt(source.x * source.x + source.y * source.y);
    if (length != 0) return source / length;
    return source;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Elevator Roguelike");
    window.setFramerateLimit(60);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::Font font;
    bool fontLoaded = font.openFromFile("/Library/Fonts/Arial.ttf");
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("arial.ttf");
    }

    sf::Text uiText(font);
    uiText.setCharacterSize(22);
    uiText.setFillColor(sf::Color::White);
    uiText.setPosition({15.f, 15.f});

    sf::RectangleShape hpBarBack({200.f, 20.f});
    hpBarBack.setFillColor(sf::Color(50, 0, 0));
    hpBarBack.setPosition({15.f, 50.f});

    sf::RectangleShape hpBarFront({200.f, 20.f});
    hpBarFront.setFillColor(sf::Color::Red);
    hpBarFront.setPosition({15.f, 50.f});

    sf::RectangleShape player(sf::Vector2f({30.f, 30.f}));
    player.setFillColor(sf::Color::Cyan);
    player.setOrigin({15.f, 15.f});
    player.setPosition({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});

    int playerHp = 100;
    int score = 0;
    int wave = 1;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Medkit> medkits;

    sf::Clock clock;
    sf::Clock spawnTimer;
    sf::Clock shootTimer;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        if (playerHp <= 0) {
            window.close();
        }

        wave = 1 + (score / 100);
        float spawnInterval = std::max(0.3f, 1.2f - (wave * 0.1f));

        sf::Vector2f movement(0.f, 0.f);
        if (window.hasFocus()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.f;
        }

        if (movement.x != 0 || movement.y != 0) movement = normalize(movement);
        player.move(movement * PLAYER_SPEED * deltaTime);

        if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && shootTimer.getElapsedTime().asSeconds() > 0.25f) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f direction = sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)) - player.getPosition();
            direction = normalize(direction);
            bullets.emplace_back(player.getPosition().x, player.getPosition().y, direction.x * BULLET_SPEED, direction.y * BULLET_SPEED);
            shootTimer.restart();
        }

        if (spawnTimer.getElapsedTime().asSeconds() > spawnInterval) {
            float ex = (std::rand() % 2 == 0) ? -50.f : WINDOW_WIDTH + 50.f;
            float ey = static_cast<float>(std::rand() % WINDOW_HEIGHT);
            if (std::rand() % 2 == 0) std::swap(ex, ey);
            enemies.emplace_back(ex, ey, wave);
            spawnTimer.restart();
        }

        for (size_t i = 0; i < medkits.size();) {
            if (medkits[i].shape.getGlobalBounds().findIntersection(player.getGlobalBounds())) {
                playerHp = std::min(100, playerHp + 20);
                medkits.erase(medkits.begin() + i);
            } else { i++; }
        }

        for (size_t i = 0; i < enemies.size();) {
            sf::Vector2f dir = normalize(player.getPosition() - enemies[i].shape.getPosition());
            enemies[i].shape.move(dir * enemies[i].speed * deltaTime);

            if (enemies[i].shape.getGlobalBounds().findIntersection(player.getGlobalBounds())) {
                playerHp -= 15;
                enemies.erase(enemies.begin() + i);
                continue;
            }
            i++;
        }

        for (size_t i = 0; i < bullets.size();) {
            bullets[i].shape.move(bullets[i].velocity * deltaTime);
            bool bulletRemoved = false;

            for (size_t j = 0; j < enemies.size(); j++) {
                if (bullets[i].shape.getGlobalBounds().findIntersection(enemies[j].shape.getGlobalBounds())) {
                    enemies[j].hp -= 20;
                    if (enemies[j].hp <= 0) {
                        if (std::rand() % 5 == 0) medkits.emplace_back(enemies[j].shape.getPosition());
                        enemies.erase(enemies.begin() + j);
                        score += 10;
                    }
                    bulletRemoved = true;
                    break;
                }
            }

            if (bulletRemoved || bullets[i].shape.getPosition().x < 0 || bullets[i].shape.getPosition().x > WINDOW_WIDTH ||
                bullets[i].shape.getPosition().y < 0 || bullets[i].shape.getPosition().y > WINDOW_HEIGHT) {
                bullets.erase(bullets.begin() + i);
            } else { i++; }
        }

        if (fontLoaded) {
            uiText.setString("Score: " + std::to_string(score) + "  Wave: " + std::to_string(wave));
        }
        hpBarFront.setSize({static_cast<float>(playerHp) * 2.f, 20.f});

        window.clear(sf::Color(30, 30, 35));
        for (auto& m : medkits) window.draw(m.shape);
        for (auto& b : bullets) window.draw(b.shape);
        for (auto& e : enemies) window.draw(e.shape);
        window.draw(player);
        window.draw(hpBarBack);
        window.draw(hpBarFront);
        if (fontLoaded) window.draw(uiText);
        window.display();
    }
    return 0;
}