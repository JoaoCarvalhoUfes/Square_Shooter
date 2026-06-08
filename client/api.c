#include <unistd.h>
#include <math.h>
#include "../shared/packets.h"
#include "api.h"
#include "../shared/utils.h"

void request_move_player(int server_sockfd, int player_id, Vector2 delta_movement){
    if(delta_movement.x == 0 && delta_movement.y == 0) return;

    delta_movement = normalize_movement(delta_movement);
    delta_movement.x *= PLAYER_SPEED * GetFrameTime();
    delta_movement.y *= PLAYER_SPEED * GetFrameTime();
    
    PacketMove packet = create_movement_packet(player_id, delta_movement);
    send_packet(server_sockfd, MOVEMENT, &packet);
}

void request_join(int server_sockfd, char *name) {
    PacketJoinRequest packet = create_join_resquest_packet(name);
    send_packet(server_sockfd, JOIN_REQUEST, &packet);
}

void request_weapon_update(int server_sockfd, WeaponType weapon) {
    PacketChangeWeapon packet = create_change_weapon_packet(weapon);
    send_packet(server_sockfd, CHANGE_WEAPON, &packet);
}

void request_aim_update(int server_sockfd, Aim *aim) {
    PacketAim packet = create_aim_packet(aim);
    send_packet(server_sockfd, AIM_UPDATE, &packet);
}

void request_shoot(int server_sockfd, int player_id) {
    PacketShoot packet = create_shoot_packet(player_id);
    send_packet(server_sockfd, SHOOT, &packet);
}