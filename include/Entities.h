#ifndef ENTITIES_H
#define ENTITIES_H

#include <SFML/Graphics.hpp>
#include "Global.h"

struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float damage;
};

struct MedKit {
    sf::CircleShape shape;
};

struct Particle {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float lifetime;
};

struct Enemy {
    sf::RectangleShape shape;
    EnemyType type;
    float hp;
    float maxHp;
    float speed;
};

#endif