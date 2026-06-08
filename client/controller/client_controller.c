#include <string.h>
#include <math.h>
#include "./client_controller.h"
#include "raymath.h"

void process_join_accept_packet(PacketJoinAccept *packet, ClientGame *client_game) {
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
        client_game->list_all_players[i].id = i;
        client_game->list_all_players[i].target_position = packet->snapshot.list_all_players[i].position;
        client_game->list_all_players[i].is_connected = packet->snapshot.list_all_players[i].is_connected;
        strcpy(client_game->list_all_players[i].name, packet->snapshot.list_all_players[i].name);
        
        // Copia a vida e kills
        client_game->list_all_players[i].life = packet->snapshot.list_all_players[i].life;
        client_game->list_all_players[i].kills = packet->snapshot.list_all_players[i].kills;

        // A mira do client ja eh feita localmente
        if(client_game->client_player_id != i) {
            memcpy(&client_game->list_all_players[i].aim, &packet->snapshot.list_all_players[i].aim, sizeof(Aim));
            client_game->list_all_players[i].weapon = packet->snapshot.list_all_players[i].weapon;
        } else {
            // Se for o próprio player, verifica se a distância do server é muito grande ou se acabou de dar respawn
            float dist = Vector2Distance(client_game->list_all_players[i].position, packet->snapshot.list_all_players[i].position);
            
            static int last_life = 100; // Começa com vida máxima
            
            // Se a vida acabou de ser restaurada pro máximo (respawn) ou distância ABSURDA
            if ((last_life <= 0 && client_game->list_all_players[i].life == 100) || dist > 200.0f) {
                client_game->list_all_players[i].position = packet->snapshot.list_all_players[i].position;
            }
            
            last_life = client_game->list_all_players[i].life;
        }
    }
}


// ======================================================================================
// Process local movements
// Removed functions to use shared/utils