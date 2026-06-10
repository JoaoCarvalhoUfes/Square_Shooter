#include "./server_controller.h"
#include <string.h>
#include "../../shared/utils.h"

void process_movement_packet(Player *player, PacketMove *packet) {
    Vector2 next_position = verify_collision_with_walls(player, packet->delta_movement);
    player_move(player, next_position);
}

void process_join_request_packet(Player *player, PacketJoinRequest *packet) {
    strcpy(player->name, packet->name);
}

void process_aim_packet(Player *player, PacketAim *packet) {
    player->aim = packet->aim;
}

void process_change_weapon_packet(Player *player, PacketChangeWeapon *packet) {
    if(is_valid_weapon(packet->weapon)) {
        player->weapon = packet->weapon;
    }
}