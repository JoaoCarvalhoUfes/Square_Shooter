#ifndef API_H
#define API_H

#include "raylib.h"

void request_join(int server_sockfd, char *name);
void request_move_player(int server_sockfd, int player_id, Vector2 delta_movement);




#endif