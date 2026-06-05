#ifndef AIM_H
#define AIM_H

#include "raylib.h"

typedef struct {
    Vector2 pos;
    float radius;    // visual radius
    float distance;  // distance from player center
    Vector2 dir;     // normalized direction from player to aim
    Color color;
    bool visible;
} Aim;

void aim_init(Aim *a, float radius, Color color);
void aim_update(Aim *a, Vector2 player_center, Camera2D camera);
// shape: 1 = circle, 2 = square, 3 = ellipse perpendicular to player
void aim_draw(Aim *a, int shape);

#endif
