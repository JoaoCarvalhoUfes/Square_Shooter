#include "raylib.h"
#include "../../config.h"
#include "./movement_validation.h"

// Returns the next position of the player
Vector2 VerifyCollisionWithWalls(Player *player, Vector2 playerMovement)
{
    Vector2 PlayerNextPosition = (Vector2){0, 0};
    Vector2 playerPosition = get_player_position(player);


    if (playerPosition.x + playerMovement.x > 0 && playerPosition.x + PLAYER_WIDTH + playerMovement.x < MAP_WIDTH)
    {
        PlayerNextPosition.x = playerPosition.x + playerMovement.x;
    }
    else
    {
        PlayerNextPosition.x = playerPosition.x;
    }

    if (playerPosition.y + playerMovement.y > 0 && playerPosition.y + PLAYER_HEIGHT + playerMovement.y < MAP_HEIGHT)
    {
        PlayerNextPosition.y = playerPosition.y + playerMovement.y;
    }
    else
    {
        PlayerNextPosition.y = playerPosition.y;
    }

    return PlayerNextPosition;
}