#include "weapon.h"
#include "raymath.h"
#include "../config.h"
#include <math.h>

static float deg2rad(float d) { return d * (PI / 180.0f); }

int get_weapon_shape_by_type(WeaponType type) {
    if(type == WEAPON_PISTOL) {
        return 1;
    }
    else if(type == WEAPON_SHOTGUN)
    {
        return 2;
    }
    else if(type == WEAPON_SNIPER) {
        return 3;
    }
    else {
        return -1;
    }
}

void weapon_init(Weapon *w, WeaponType type)
{
    w->type = type;
    // default stats from config.h
    if (type == WEAPON_PISTOL)
    {
        w->damage = PISTOL_DAMAGE;
        w->range = PISTOL_RANGE;
        w->speed = PISTOL_SPEED;
        w->proj_count = 1;
        w->shape = 1;
    }
    else if (type == WEAPON_SHOTGUN)
    {
        w->damage = SHOTGUN_DAMAGE;
        w->range = SHOTGUN_RANGE;
        w->speed = SHOTGUN_SPEED;
        w->proj_count = SHOTGUN_PROJ_COUNT;
        w->shape = 2;
    }
    else if (type == WEAPON_SNIPER)
    {
        w->damage = SNIPER_DAMAGE;
        w->range = SNIPER_RANGE;
        w->speed = SNIPER_SPEED;
        w->proj_count = 1;
        w->shape = 3;
    }

    for (int i = 0; i < MAX_PROJECTILES; i++)
        w->projectiles[i].active = false;
    w->laser.active = false;
}

static Projectile *weapon_spawn_proj(Weapon *w)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!w->projectiles[i].active)
        {
            w->projectiles[i].active = true;
            w->projectiles[i].life = 0.0f;
            w->projectiles[i].damage = w->damage;
            w->projectiles[i].shape = w->shape;
            return &w->projectiles[i];
        }
    }
    return 0;
}

void weapon_fire(Weapon *w, Vector2 origin, Vector2 aim_pos)
{
    Vector2 dir = Vector2Subtract(aim_pos, origin);
    float len = Vector2Length(dir);
    if (len < 0.0001f)
        dir = (Vector2){1.0f, 0.0f};
    else
        dir = Vector2Scale(dir, 1.0f / len);

    if (w->type == WEAPON_PISTOL)
    {
        // pistol: projectile with infinite range
        Projectile *p = weapon_spawn_proj(w);
        if (!p)
            return;
        p->pos = origin;
        p->vel = Vector2Scale(dir, w->speed);
        p->life = PISTOL_LIFE;
    }
    else if (w->type == WEAPON_SNIPER)
    {
        // sniper: instant laser with infinite range
        w->laser.start = origin;
        w->laser.end = Vector2Add(origin, Vector2Scale(dir, SNIPER_LASER_RANGE));
        w->laser.life = SNIPER_LASER_LIFE;
        w->laser.active = true;
    }
    else if (w->type == WEAPON_SHOTGUN)
    {
        // spread angle in degrees
        int n = w->proj_count;
        float baseAngle = atan2f(dir.y, dir.x) * (180.0f / PI);
        int mid = n / 2;
        for (int i = 0; i < n; i++)
        {
            float offset = 0.0f;
            if (n > 1)
                offset = ((float)i - (float)mid) * (SHOTGUN_SPREAD / (float)(n - 1));
            float ang = baseAngle + offset;
            float rad = deg2rad(ang);
            Vector2 d = (Vector2){cosf(rad), sinf(rad)};
            Projectile *p = weapon_spawn_proj(w);
            if (!p)
                continue;
            p->pos = origin;
            p->vel = Vector2Scale(d, w->speed);
            p->life = w->range / w->speed;
        }
    }
}

void weapon_update(Weapon *w, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &w->projectiles[i];
        if (!p->active)
            continue;
        p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, dt));
        p->life -= dt;
        if (p->life <= 0.0f)
            p->active = false;
    }
    // Update laser
    if (w->laser.active)
    {
        w->laser.life -= dt;
        if (w->laser.life <= 0.0f)
            w->laser.active = false;
    }
}

void weapon_draw(Weapon *w)
{
    // Draw laser if active
    if (w->laser.active)
    {
        DrawLineEx(w->laser.start, w->laser.end, SNIPER_LASER_WIDTH, LIME);
    }

    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &w->projectiles[i];
        if (!p->active)
            continue;
        // draw based on weapon shape
        if (p->shape == 1)
        {
            DrawCircleV(p->pos, PISTOL_PROJ_RADIUS, YELLOW);
        }
        else if (p->shape == 2)
        {
            DrawRectangleV(p->pos, (Vector2){SHOTGUN_PROJ_SIZE, SHOTGUN_PROJ_SIZE}, ORANGE);
        }
        else if (p->shape == 3)
        {
            DrawCircleV(p->pos, 3, SKYBLUE);
        }
    }
}
