#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include "config.h"

typedef struct player Player;

Player *player_create(const char *name, Vector2 size, Vector2 player_position);
Vector2 get_player_position(Player *p);
Vector2 get_player_size(Player *p);
void player_move(Player *p, Vector2 direction);

void player_destroy(Player *p);

#endif