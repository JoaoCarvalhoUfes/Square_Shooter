#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
#include "./client/player.h"

// ========= DEFINES ==============
#define WINDOW_NAME "Square Shooter"

#define MAX_PLAYER_LIFE 5
#define MAX_PLAYERS 30

#define MAP_WIDTH 2000
#define MAP_HEIGHT 2000 

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define PLAYER_SPEED 300.0f
#define MOVE_INTERPOLATION_FACTOR 0.10f

#define BUFFER_SIZE 4000 // 4kB

// ========= STRUCTS ==============
typedef struct {
    Player list_all_players[MAX_PLAYERS];
    int client_player_id;

    // for sockets
    char buffer[BUFFER_SIZE];
    int num_bytes_in_buf;
} ClientGame;

typedef struct {
    Player list_all_players[MAX_PLAYERS];
} SnapShot;


// Obs.: RGBA
#define BACKGROUND_COLOR CLITERAL(Color){50, 50, 50, 50}

#endif
