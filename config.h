#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

// aux structs
typedef struct {
    int width;
    int height;
} Size;

// ============

#define WINDOW_NAME "Top-down Maverick (?)"
#define MAX_PLAYER_LIFE 5
// Obs.: RGBA
#define BACKGROUND_COLOR CLITERAL(Color) {50, 50, 50, 50}


#endif
