#ifndef PACKETS_H
#define PACKETS_H

#include "raylib.h"
#include "../config.h"

typedef enum {
    JOIN_REQUEST,
    JOIN_ACCEPT,
    MOVEMENT,
    SNAPSHOT,
    AIM,
    SHOOT,
} type_packet;

typedef struct {
    type_packet type;
    char name[MAX_INPUT_CHARS];
} PacketJoinRequest;

typedef struct {
    type_packet type;
    int player_id;
} PacketJoinAccept;

typedef struct {
    type_packet type;
    Aim aim;
} PacketAim;

typedef struct {
    type_packet type;
    int player_id;
    Vector2 delta_movement;
} PacketMove;

typedef struct {
    type_packet type;
    SnapShot snapshot;
} PacketSnapshot;

int get_packet_size(type_packet type);
void send_packet(int dst_socket, type_packet type, void *packet);

PacketMove create_movement_packet(int player_id, Vector2 delta_movement);
PacketSnapshot create_snapshot_packet(SnapShot *snapshot);
PacketJoinAccept create_join_accept_packet(int player_id);
PacketJoinRequest create_join_resquest_packet(char *name);
PacketAim create_aim_packet(Aim *aim);

#endif