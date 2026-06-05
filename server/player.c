#include <string.h>
#include "./player.h"
#include "../config.h"

Player player_create(const char *name, int id, Vector2 player_position)
{
    Player p;

    p.id = id;
    strcpy(p.name, name);
    p.life = MAX_PLAYER_LIFE;
    p.is_connected = true;

    // Correção da posição
    p.position.x = player_position.x - (PLAYER_WIDTH) / 2;
    p.position.y = player_position.y - (PLAYER_HEIGHT) / 2;

    aim_init(&p.aim, PISTOL_AIM_RADIUS, WHITE);
    p.weapon = WEAPON_PISTOL;

    return p;
}

void set_radius_player_aim(Player *p, float radius) {
    aim_init(&p->aim, radius, WHITE);
}

void update_player_aim(Player *p, Camera2D camera) {
    Vector2 player_center = (Vector2){p->position.x + (PLAYER_WIDTH / 2.0f), p->position.y + (PLAYER_HEIGHT / 2.0f)};
    aim_update(&p->aim, player_center, camera);
}

Aim *get_player_aim(Player *p) {
    return &p->aim;
}

Vector2 get_player_aim_position(Player *p) {
    return p->aim.pos;
}

Vector2 get_player_position(Player *p)
{
    return p->position;
}

char *get_player_name(Player *p)
{
    return p->name;
}

WeaponType get_player_weapon(Player *p) {
    return p->weapon;
}

void set_player_weapon(Player *p, WeaponType weapon) {
    p->weapon = weapon;
}

void set_player_name(Player *p, const char *name)
{
    strcpy(p->name, name);
}

void player_move(Player *p, Vector2 direction)
{
    p->position = direction;
}

void connect_player(Player *p)
{
    p->is_connected = true;
}

void disconnect_player(Player *p)
{
    p->is_connected = false;
}

bool player_is_connected(Player *p)
{
    return p->is_connected;
}
