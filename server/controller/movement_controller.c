#include "./movement_controller.h"
#include "../validation/movement_validation.h"

void process_movement_packet(Player *player, PacketMove packet) {
    Vector2 next_position = VerifyCollisionWithWalls(player, packet.delta_movement);
    player_move(player, next_position);
}