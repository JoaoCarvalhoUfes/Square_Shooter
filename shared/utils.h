#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include "player.h"

Vector2 normalize_movement(Vector2 dir);
Vector2 verify_collision_with_walls(Player *player, Vector2 pixelOffset);

#endif
