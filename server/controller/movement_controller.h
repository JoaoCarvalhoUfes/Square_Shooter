#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include "../packets.h"
#include "raylib.h"

void process_movement_packet(Player *player, PacketMove packet);


#endif