#include "./movement_validation.h"
#include "raylib.h"


static Vector2 NormilizeMovement(Vector2 playerNextPosition)
{
    float length = sqrt(playerNextPosition.x * playerNextPosition.x + playerNextPosition.y * playerNextPosition.y);
    if (length > 0)
    {
        playerNextPosition.x /= length;
        playerNextPosition.y /= length;
    }
    return playerNextPosition;
}

Vector2 VerifyCollisionWithWalls(Player player, Vector2 playerMovement)
{ 
    Vector2 PlayerNextPosition = (Vector2){0, 0};

    Vector2 playerPosition = get_player_position(player);
    playerMovement = NormilizeMovement(playerMovement);

    if (playerPosition.x + playerMovement.x > 0 && playerPosition.x + PLAYER_WIDTH + playerMovement.x < GetScreenWidth())
    {
        PlayerNextPosition.x = playerPosition.x + playerMovement.x; // No collision detected
    }
    else
    {
        PlayerNextPosition.x = playerPosition.x; // Collision detected
    }

    if (playerPosition.y + playerMovement.y > 0 && playerPosition.y + PLAYER_HEIGHT + playerMovement.y < GetScreenHeight())
    {
        PlayerNextPosition.y = playerPosition.y + playerMovement.y; // No collision detected
    }
    else
    {
        PlayerNextPosition.y = playerPosition.y; // Collision detected
    }

    return PlayerNextPosition; // Return the final position
}