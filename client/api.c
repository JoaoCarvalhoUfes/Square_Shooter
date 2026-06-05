#include <unistd.h>
#include <math.h>
#include "./packets.h"
#include "./api.h"

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

void request_move_player(int server_sockfd, int player_id, Vector2 delta_movement){
    if(delta_movement.x == 0 && delta_movement.y == 0) return;

    delta_movement = NormilizeMovement(delta_movement);
    delta_movement.x *= PLAYER_SPEED * GetFrameTime();
    delta_movement.y *= PLAYER_SPEED * GetFrameTime();
    
    PacketMove packet = create_movement_packet(player_id, delta_movement);
    send_packet(server_sockfd, MOVEMENT, &packet);
}

void request_join(int server_sockfd, char *name) {
    PacketJoinRequest packet = create_join_resquest_packet(name);
    send_packet(server_sockfd, JOIN_REQUEST, &packet);
}

void request_aim_update(int server_sockfd, Aim *aim) {
    PacketAim packet = create_aim_packet(aim);
    send_packet(server_sockfd, AIM, &packet);
}