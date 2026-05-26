#include "raylib.h"
#include "config.h"
#include "player.h"
#include <math.h>
#include <stdio.h>

void init_setup();

Vector2 NormilizeMovement(Vector2 playerNextPosition)
{
    float length = sqrt(playerNextPosition.x * playerNextPosition.x + playerNextPosition.y * playerNextPosition.y);
    if (length > 0)
    {
        playerNextPosition.x /= length;
        playerNextPosition.y /= length;
    }
    return playerNextPosition;
}

Vector2 VerifyCollisionWithWalls(Vector2 playerMovement, Vector2 playerPosition, Vector2 playerSize)
{

    Vector2 PlayerNextPosition = (Vector2){0, 0};
    if (playerPosition.x + playerMovement.x > 0 && playerPosition.x + playerSize.x + playerMovement.x < GetScreenWidth())
    {
        PlayerNextPosition.x = playerPosition.x + playerMovement.x; // No collision detected
    }
    else
    {
        PlayerNextPosition.x = playerPosition.x; // Collision detected
    }

    if (playerPosition.y + playerMovement.y > 0 && playerPosition.y + playerSize.y + playerMovement.y < GetScreenHeight())
    {
        PlayerNextPosition.y = playerPosition.y + playerMovement.y; // No collision detected
    }
    else
    {
        PlayerNextPosition.y = playerPosition.y; // Collision detected
    }

    return PlayerNextPosition; // Return the final position
}

int main()
{
    init_setup();

    // Creating player
    // initial_position = metade da tela
    Vector2 initial_position = {GetScreenWidth() / 2, GetScreenHeight() / 2};
    Vector2 size = {40, 40};
    Player *player = player_create("Ronald", size, initial_position);

    while (!WindowShouldClose())
    {
        Vector2 playerMovement = (Vector2){0, 0};
        // movement keys
        if (IsKeyDown(KEY_A))
            // send to server to validate movement
            playerMovement.x -= 1;
        if (IsKeyDown(KEY_S))
            // send to server to validate movement
            playerMovement.y += 1;
        if (IsKeyDown(KEY_D))
            // send to server to validate movement
            playerMovement.x += 1;
        if (IsKeyDown(KEY_W))
            // send to server to validate movement
            playerMovement.y -= 1;

        player_move(player, VerifyCollisionWithWalls(NormilizeMovement(playerMovement), get_player_position(player), get_player_size(player)));
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

void init_setup()
{
    InitWindow(800, 400, WINDOW_NAME);
    // ToggleFullscreen();
}