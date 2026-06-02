#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
#include "./player.h"

// ========= DEFINES ==============
#define WINDOW_NAME "Square Shooter"

#define MAX_PLAYER_LIFE 5
#define MAX_PLAYERS 30

#define MAP_WIDTH 2000
#define MAP_HEIGHT 2000 

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40

// ========= STRUCTS ==============

typedef struct {
    Player list_all_players[MAX_PLAYERS];
} SnapShotGame;


// Obs.: RGBA
#define BACKGROUND_COLOR CLITERAL(Color){50, 50, 50, 50}

#endif
