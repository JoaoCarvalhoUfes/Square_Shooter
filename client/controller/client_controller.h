#ifndef CLIENT_CONTROLLER
#define CLIENT_CONTROLLER

#include "../packets.h"
#include "../../config.h"

void process_join_packet(PacketJoinAccept *packet, ClientGame *client_game);
void process_snapshot_packet(PacketSnapshot *packet, ClientGame *client_game);

// process local movement
Vector2 VerifyCollisionWithWalls(Player *player, Vector2 playerMovement);


#endif