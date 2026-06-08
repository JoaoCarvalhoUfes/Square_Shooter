#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
#include "./shared/player.h"

// ========= DEFINES ==============
#define WINDOW_NAME "Square Shooter"

#define MAX_PLAYER_LIFE 100
#define MAX_PLAYERS 30

#define MAP_WIDTH 2000
#define MAP_HEIGHT 2000 

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define PLAYER_SPEED 300.0f
#define MOVE_INTERPOLATION_FACTOR 0.25f

#define BUFFER_SIZE 4000 // 4kB
#define FONT_SIZE 24
#define MAX_INPUT_CHARS 8
#define FONT_COLOR WHITE

// ========= WEAPON SETTINGS ==============
// PISTOL
#define PISTOL_DAMAGE 10
#define PISTOL_SPEED 800.0f
#define PISTOL_RANGE 600.0f
#define PISTOL_LIFE 10.0f
#define PISTOL_AIM_RADIUS 8.0f
#define PISTOL_PROJ_RADIUS 8

// SHOTGUN
#define SHOTGUN_DAMAGE 6
#define SHOTGUN_SPEED 600.0f
#define SHOTGUN_RANGE 300.0f
#define SHOTGUN_PROJ_COUNT 6
#define SHOTGUN_SPREAD 20.0f
#define SHOTGUN_AIM_RADIUS 8.0f
#define SHOTGUN_PROJ_SIZE 10

// SNIPER
#define SNIPER_DAMAGE 50
#define SNIPER_SPEED 1800.0f
#define SNIPER_RANGE 1400.0f
#define SNIPER_LASER_RANGE 3000.0f
#define SNIPER_LASER_LIFE 0.1f
#define SNIPER_AIM_RADIUS 6.0f
#define SNIPER_LASER_WIDTH 3.0f

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
