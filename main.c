#include "raylib.h"
#include "config.h"
#include "player.h"

void init_setup();

int main() {
    init_setup();

    // Creating player
    // initial_position = metade da tela
    Vector2 initial_position = {GetScreenWidth()/2, GetScreenHeight()/2};
    Vector2 size = {40, 40};
    Player *player = player_create("Ronald", size, initial_position);

    while(!WindowShouldClose()) {
        // init draw
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            DrawRectangleV(get_player_position(player), get_player_size(player), RED);
        EndDrawing();
    }

    CloseWindow();


    // frees
    player_destroy(player);
}

void init_setup() {
    InitWindow(800, 400, WINDOW_NAME);
    // ToggleFullscreen();
}