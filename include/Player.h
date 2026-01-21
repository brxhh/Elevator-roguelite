#ifndef PLAYER_H
#define PLAYER_H

#include "Global.h"

class Player {
public:
    sf::RectangleShape shape;
    float speed = 250.f;
    float damage = 20.f;
    int hp = 100;

    float dashTimer = 0.f;
    float dashCooldown = 1.0f;
    bool isDashing = false;
    sf::Vector2f dashDir;

    Player() {
        shape.setSize({30, 30});
        shape.setFillColor(sf::Color::Cyan);
        shape.setOrigin({15, 15});
        reset();
    }

    void reset() {
        hp = 100;
        shape.setPosition({W_WIDTH / 2.f, W_HEIGHT / 2.f});
        dashTimer = 0.f;
        isDashing = false;
    }

    void handleInput(float dt) {
        sf::Vector2f moveVec(0, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveVec.y -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveVec.y += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveVec.x -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveVec.x += 1;

        dashTimer -= dt;
        // Trigger dash
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && dashTimer <= 0 && (moveVec.x != 0 || moveVec.y != 0)) {
            isDashing = true;
            dashTimer = dashCooldown;
            dashDir = normalize(moveVec);
        }

        if (isDashing) {
            shape.move(dashDir * speed * 3.f * dt);
            if (dashTimer < dashCooldown - 0.15f) isDashing = false;
        } else {
            shape.move(normalize(moveVec) * speed * dt);
        }

        // Clamp to window bounds
        sf::Vector2f pos = shape.getPosition();
        shape.setPosition({std::clamp(pos.x, 15.f, 785.f), std::clamp(pos.y, 15.f, 585.f)});
    }
};

#endif