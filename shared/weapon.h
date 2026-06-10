#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"

#define MAX_PROJECTILES 128

typedef enum { WEAPON_PISTOL = 1, WEAPON_SHOTGUN = 2, WEAPON_SNIPER = 3 } WeaponType;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float life; // seconds remaining
    bool active;
    int damage;
    int shape; // same as aim shape
} Projectile;

typedef struct {
    Vector2 start;
    Vector2 end;
    float life; // seconds remaining
    bool active;
    bool hit_processed;
} Laser;

typedef struct {
    WeaponType type;
    int damage;
    float range;
    float speed;
    int proj_count;
    int shape; // 1/2/3

    Projectile projectiles[MAX_PROJECTILES];
    Laser laser; // for sniper
} Weapon;

void weapon_init(Weapon *w, WeaponType type);
void weapon_fire(Weapon *w, Vector2 origin, Vector2 dir);
void weapon_update(Weapon *w, float dt);
void weapon_draw(Weapon *w);

bool is_valid_weapon(WeaponType type);
int get_weapon_shape_by_type(WeaponType type);

#endif
