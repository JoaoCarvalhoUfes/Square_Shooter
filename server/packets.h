#ifndef PACKETS_H
#define PACKETS_H

#include "raylib.h"
#include "../config.h"

typedef enum {
    JOIN_ACCEPT,
    MOVEMENT,
    SNAPSHOT,
    SHOOT
} type_packet;

typedef struct {
    type_packet type;
    int player_id;
} PacketJoinAccept;

typedef struct {
    type_packet type;
    int player_id;
    Vector2 delta_movement;
} PacketMove;

typedef struct {
    type_packet type;
    SnapShotGame snapshot;
} PacketSnapshot;



#endif