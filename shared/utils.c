#include "utils.h"
#include "../config.h"
#include <math.h>

Vector2 normalize_movement(Vector2 dir) {
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length > 0) {
        dir.x /= length;
        dir.y /= length;
    }
    return dir;
}

Vector2 verify_collision_with_walls(Player *player, Vector2 pixelOffset) {
    Vector2 PlayerNextPosition = (Vector2){0, 0};
    Vector2 playerPosition = get_player_position(player);

    if (playerPosition.x + pixelOffset.x > 0 && playerPosition.x + PLAYER_WIDTH + pixelOffset.x < MAP_WIDTH) {
        PlayerNextPosition.x = playerPosition.x + pixelOffset.x;
    } else {
        PlayerNextPosition.x = playerPosition.x;
    }

    if (playerPosition.y + pixelOffset.y > 0 && playerPosition.y + PLAYER_HEIGHT + pixelOffset.y < MAP_HEIGHT) {
        PlayerNextPosition.y = playerPosition.y + pixelOffset.y;
    } else {
        PlayerNextPosition.y = playerPosition.y;
    }

    return PlayerNextPosition;
}
