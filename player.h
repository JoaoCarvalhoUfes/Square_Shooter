#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdlib.h>

typedef struct player Player;
struct player
{
    int id;
    char *name;
    int life;
    Vector2 position;
};

Player player_create(const char *name, int id, Vector2 player_position);
Vector2 get_player_position(Player p);
void player_move(Player *p, Vector2 direction);

void player_destroy(Player *p);

#endif