#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "aim.h"
#include "weapon.h"

typedef struct player Player;
struct player
{
    int id;

    // tem q colocar constante aqui (ainda não fiz por problemas de import circular)
    char name[8];
    bool is_connected;
    int life;

    Vector2 position;
    Vector2 target_position;

    Aim aim;
    WeaponType weapon;
};

Player player_create(const char *name, int id, Vector2 player_position);

Vector2 get_player_aim_position(Player *p);
Vector2 get_player_position(Player *p);
char *get_player_name(Player *p);
Aim *get_player_aim(Player *p);
WeaponType get_player_weapon(Player *p);

void set_player_aim(Player *p, WeaponType weapon);
void set_player_name(Player *p, const char *name);
void set_player_weapon(Player *p, WeaponType weapon);

void player_move(Player *p, Vector2 direction);
void update_player_aim(Player *p, Camera2D camera);
void connect_player(Player *p);
void disconnect_player(Player *p);
bool player_is_connected(Player *p);

#endif