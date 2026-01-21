#ifndef GLOBAL_H
#define GLOBAL_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <SFML/Window/VideoMode.hpp>

const int W_WIDTH = 800;
const int W_HEIGHT = 600;

enum class GameState { Menu, Playing, Shop, GameOver };
enum class EnemyType { Normal, Tank, Fast };

// Professional utility for frame-independent movement
inline sf::Vector2f normalize(sf::Vector2f v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    return (len != 0) ? v / len : v;
}

#endif