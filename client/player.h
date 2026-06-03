#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct player Player;
struct player
{
    int id;
    char *name;
    bool is_connected;

    int life;
    Vector2 position;
    Vector2 target_position;
};

Player player_create(const char *name, int id, Vector2 player_position);
Vector2 get_player_position(Player p);
void player_move(Player *p, Vector2 direction);

void connect_player(Player *p);
void disconnect_player(Player *p);
bool player_is_connected(Player *p);

#endif