#ifndef CLIENT_CONTROLLER
#define CLIENT_CONTROLLER_H

#include "../../shared/packets.h"
#include "../../config.h"

void process_join_accept_packet(PacketJoinAccept *packet, ClientGame *client_game);
void process_snapshot_packet(PacketSnapshot *packet, ClientGame *client_game);

// process local movement


#endif