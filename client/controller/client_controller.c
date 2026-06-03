#include <string.h>
#include <math.h>
#include "./client_controller.h"

void process_join_packet(PacketJoinAccept *packet, ClientGame *client_game) {
    client_game->client_player_id = packet->player_id;
}

void process_snapshot_packet(PacketSnapshot *packet, ClientGame *client_game) 
{
    static bool initial_position_saved = false;

    if(!initial_position_saved) {
        client_game->list_all_players[client_game->client_player_id].position = packet->snapshot.list_all_players[client_game->client_player_id].position;
        initial_position_saved = true;
    }

    for(int i = 0; i < MAX_PLAYERS; i++) {
        client_game->list_all_players[i].target_position = packet->snapshot.list_all_players[i].position;
        client_game->list_all_players[i].is_connected = packet->snapshot.list_all_players[i].is_connected;
        strcpy(client_game->list_all_players[i].name, packet->snapshot.list_all_players[i].name);
    }
}


// ======================================================================================
// Process local movements


static Vector2 NormilizeMovement(Vector2 playerNextPosition)
{
    float length = sqrt(playerNextPosition.x * playerNextPosition.x + playerNextPosition.y * playerNextPosition.y);
    if (length > 0)
    {
        playerNextPosition.x /= length;
        playerNextPosition.y /= length;
    }

    playerNextPosition.x *= PLAYER_SPEED * GetFrameTime();
    playerNextPosition.y *= PLAYER_SPEED * GetFrameTime();

    return playerNextPosition;
}

Vector2 VerifyCollisionWithWalls(Player *player, Vector2 playerMovement)
{
    playerMovement = NormilizeMovement(playerMovement);

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