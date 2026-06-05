#include "aim.h"
#include "raymath.h"
#include "../config.h"
#include <math.h>

void aim_init(Aim *a, float radius, Color color)
{
    a->pos = (Vector2){0, 0};
    a->radius = radius;
    // position distance from player is based on player size constant
    a->distance = (PLAYER_WIDTH / 2.0f) + 15.0f; // Adjusted to match original context
    a->color = color;
    a->visible = true;
    a->dir = (Vector2){1.0f, 0.0f}; // Initialize direction
}

void aim_update(Aim *a, Vector2 player_center, Camera2D camera)
{
    Vector2 mouse_screen = GetMousePosition();
    Vector2 mouse_world = GetScreenToWorld2D(mouse_screen, camera);
    Vector2 dir = Vector2Subtract(mouse_world, player_center);
    float len = Vector2Length(dir);
    if (len < 0.0001f)
    {
        dir = (Vector2){1.0f, 0.0f};
    }
    else
    {
        dir = Vector2Scale(dir, 1.0f / len);
    }

    a->dir = dir; // normalized direction from player center to aim
    a->pos = Vector2Add(player_center, Vector2Scale(dir, a->distance));
}

void aim_draw(Aim *a, int shape)
{
    if (!a->visible)
        return;

    if (shape == 1)
    {
        // circle (pistol)
        DrawCircleV(a->pos, a->radius, a->color);
    }
    else if (shape == 2)
    {
        // square (shotgun)
        float half = a->radius;
        Rectangle rec = {a->pos.x - half, a->pos.y - half, half * 2, half * 2};
        DrawRectangleRec(rec, a->color);
    }
    else if (shape == 3)
    {
        // ellipse (sniper): elongated horizontally using line+circle pattern
        float halfLen = a->radius * 2.5f;
        float halfWid = a->radius * 0.8f;
        Vector2 endA = Vector2Add(a->pos, Vector2Scale(a->dir, -halfLen));
        Vector2 endB = Vector2Add(a->pos, Vector2Scale(a->dir, halfLen));
        DrawLineEx(endA, endB, halfWid * 2, a->color);
        DrawCircleV(endA, halfWid, a->color);
        DrawCircleV(endB, halfWid, a->color);
    }
    else
    {
        // fallback: circle
        DrawCircleV(a->pos, a->radius, a->color);
    }
}
