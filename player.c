#include "./player.h"
#include <string.h>
#include "raylib.h"

struct player
{
    char *name;
    int life;

    Vector2 size;
    Vector2 position;
};

Player *player_create(const char *name, Vector2 size, Vector2 player_position)
{
    Player *p = (Player *)malloc(sizeof(Player));
    p->name = strdup(name);
    p->life = MAX_PLAYER_LIFE;
    p->size = size;

    // Correção da posição
    Vector2 position = {player_position.x - (p->size.x) / 2, player_position.y - (p->size.y) / 2};
    p->position = position;

    return p;
}

Vector2 get_player_size(Player *p)
{
    return p->size;
}

Vector2 get_player_position(Player *p)
{
    return p->position;
}

void player_move(Player *p, Vector2 direction)
{
    p->position.x = direction.x;
    p->position.y = direction.y;
}

void player_destroy(Player *p)
{
    free(p);
}