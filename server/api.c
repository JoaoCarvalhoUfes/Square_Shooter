#include <unistd.h>
#include "./packets.h"
#include "./api.h"


void request_move_player(int server_sockfd, int player_id, Vector2 delta_movement){
    PacketMove packet = {
        .type = MOVEMENT, 
        .player_id = player_id,
        .delta_movement = delta_movement,
    };

    write(server_sockfd, &packet, sizeof(packet));
}