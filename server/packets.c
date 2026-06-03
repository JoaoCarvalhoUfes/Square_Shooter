#include "./packets.h"
#include "./unistd.h"

int get_packet_size(type_packet type) {
    if(type == MOVEMENT) {
        return sizeof(PacketMove);
    } else if (type == JOIN_ACCEPT) {
        return sizeof(PacketJoinAccept);
    } else if (type == SNAPSHOT) {
        return sizeof(PacketSnapshot);
    } else {
        return 0;
    }
}

PacketMove create_movement_packet(int player_id, Vector2 delta_movement) {
    PacketMove p = {
        .type = MOVEMENT,
        .player_id = player_id,
        .delta_movement = delta_movement
    };

    return p;
}

PacketSnapshot create_snapshot_packet(SnapShot *snapshot) {
    PacketSnapshot p = {
        .type = SNAPSHOT,
        .snapshot = *snapshot
    };

    return p;
}

PacketJoinAccept create_join_accept_packet(int player_id) {
    PacketJoinAccept p = {
        .type = JOIN_ACCEPT,
        .player_id = player_id,
    };

    return p;
}


void send_packet(int dst_socket, type_packet type, void *packet) {
    write(dst_socket, packet, get_packet_size(type));
}