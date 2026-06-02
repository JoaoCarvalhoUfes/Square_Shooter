#include <string.h>
#include "./player.h"
#include "config.h"

Player player_create(const char *name, int id, Vector2 player_position)
{
    Player p;

    p.id = id;
    p.name = strdup(name);
    p.life = MAX_PLAYER_LIFE;

    // Correção da posição
    Vector2 position = {player_position.x - (PLAYER_WIDTH) / 2, player_position.y - (PLAYER_HEIGHT) / 2};
    p.position = position;

    return p;
}

Vector2 get_player_position(Player p)
{
    return p.position;
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