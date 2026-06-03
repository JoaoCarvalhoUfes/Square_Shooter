#include <string.h>
#include "./client_controller.h"

void process_join_packet(PacketJoinAccept *packet, ClientGame *client_game) {
    client_game->client_player_id = packet->player_id;
}

void process_snapshot_packet(PacketSnapshot *packet, ClientGame *client_game) {
    // Copiando players
    //memcpy(&client_game->list_all_players, packet->snapshot.list_all_players, sizeof(Player)*MAX_PLAYERS);
    for(int i = 0; i < MAX_PLAYERS; i++) {
        client_game->list_all_players[i].target_position = packet->snapshot.list_all_players[i].position; 
        client_game->list_all_players[i].target_position = packet->snapshot.list_all_players[i].position;
        client_game->list_all_players[i].is_connected = packet->snapshot.list_all_players[i].is_connected;
    }
}