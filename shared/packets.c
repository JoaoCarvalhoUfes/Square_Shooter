#include "./packets.h"
#include <./unistd.h>
#include <string.h>

int get_packet_size(type_packet type) {
    switch (type) {
        case MOVEMENT:
            return sizeof(PacketMove);
        case JOIN_ACCEPT:
            return sizeof(PacketJoinAccept);
        case SNAPSHOT:
            return sizeof(PacketSnapshot);
        case JOIN_REQUEST:
            return sizeof(PacketJoinRequest);
        case AIM_UPDATE:
            return sizeof(PacketAim);
        case CHANGE_WEAPON:
            return sizeof(PacketChangeWeapon);
        case SHOOT:
            return sizeof(PacketShoot);
        default:
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

PacketAim create_aim_packet(Aim *aim) {
    PacketAim p = {
        .type = AIM_UPDATE,
        .aim = *aim
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

PacketJoinRequest create_join_resquest_packet(char *name) {
    PacketJoinRequest p = {
        .type = JOIN_REQUEST
    };
    strcpy(p.name, name);

    return p;
}

PacketChangeWeapon create_change_weapon_packet(WeaponType weapon) {
    PacketChangeWeapon packet;
    packet.type = CHANGE_WEAPON;
    packet.weapon = weapon;

    return packet;
}

PacketShoot create_shoot_packet(int player_id) {
    PacketShoot packet;
    packet.type = SHOOT;
    packet.player_id = player_id;
    return packet;
}

void send_packet(int dst_socket, type_packet type, void *packet) {
    write(dst_socket, packet, get_packet_size(type));
}