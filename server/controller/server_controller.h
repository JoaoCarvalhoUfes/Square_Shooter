#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include "../player.h"
#include "../packets.h"
#include "raylib.h"

void process_movement_packet(Player *player, PacketMove *packet);
void process_aim_packet(Player *player, PacketAim *packet);
void process_join_request_packet(Player *player, PacketJoinRequest *packet);
void process_change_weapon_packet(Player *player, PacketChangeWeapon *packet);
#endif