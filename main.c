#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

#include "raylib.h"
#include "config.h"
#include "player.h"

// typedef struct {
//     Player list_all_players[MAX_PLAYERS];
//     Player *client_player;
// } Game;

int connect_to_server(char *hostname, int port_number);
void init_setup();


static Vector2 NormilizeMovement(Vector2 playerNextPosition)
{
    float length = sqrt(playerNextPosition.x * playerNextPosition.x + playerNextPosition.y * playerNextPosition.y);
    if (length > 0)
    {
        playerNextPosition.x /= length;
        playerNextPosition.y /= length;
    }
    return playerNextPosition;
}

Vector2 VerifyCollisionWithWalls(Player player, Vector2 playerMovement)
{ 
    Vector2 PlayerNextPosition = (Vector2){0, 0};

    Vector2 playerPosition = get_player_position(player);
    playerMovement = NormilizeMovement(playerMovement);

    if (playerPosition.x + playerMovement.x > 0 && playerPosition.x + PLAYER_WIDTH + playerMovement.x < GetScreenWidth())
    {
        PlayerNextPosition.x = playerPosition.x + playerMovement.x; // No collision detected
    }
    else
    {
        PlayerNextPosition.x = playerPosition.x; // Collision detected
    }

    if (playerPosition.y + playerMovement.y > 0 && playerPosition.y + PLAYER_HEIGHT + playerMovement.y < GetScreenHeight())
    {
        PlayerNextPosition.y = playerPosition.y + playerMovement.y; // No collision detected
    }
    else
    {
        PlayerNextPosition.y = playerPosition.y; // Collision detected
    }

    return PlayerNextPosition; // Return the final position
}

int main(int argc, char *argv[])
{
    // ============== SERVIDOR ==============
    // Políticas de servidor
    // if(argc < 3) {
    //     fprintf(stderr, "Usage %s hostname port\n", argv[0]);
    //     exit(1);
    // }

    // char *hostname = argv[1];
    // int port_number = atoi(argv[2]);
    // int server_fd = connect_to_server(hostname, port_number);

    // =====================================
    // Se chegar aqui, conectou ao server, le o pacote de boas vindas e depois o de snapshot



    // ==============

    init_setup();

    // Creating player
    // initial_position = metade da tela
    Vector2 initial_position = {GetScreenWidth() / 2, GetScreenHeight() / 2};
    Vector2 size = {40, 40};
    Player player = player_create("Ronald", 0, initial_position);

    while (!WindowShouldClose())
    {
        Vector2 playerMovement = (Vector2){0, 0};
        // movement keys
        if (IsKeyDown(KEY_A))
            playerMovement.x -= 1;
        if (IsKeyDown(KEY_S))
            playerMovement.y += 1;
        if (IsKeyDown(KEY_D))
            playerMovement.x += 1;
        if (IsKeyDown(KEY_W))
            playerMovement.y -= 1;

        player_move(&player, VerifyCollisionWithWalls(player, playerMovement));
        // init draw
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        DrawRectangleV(get_player_position(player), (Vector2) {.x = PLAYER_WIDTH, .y = PLAYER_HEIGHT}, RED);
        EndDrawing();
    }

    CloseWindow();
}

void init_setup()
{
    InitWindow(800, 400, WINDOW_NAME);
    // ToggleFullscreen();
}

// Retorna o descritor de arquivo do servidor
int connect_to_server(char *hostname, int port_number) {
    int server_fd;
    struct sockaddr_in server_address;
    struct hostent *server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
        perror("ERROR opening socket"); exit(1);

    server = gethostbyname(hostname);
    if(server == NULL) {
        perror("Error, no such rost"); exit(1);
    }
    
    // limpando para nn ter problema com lixo de memoria
    bzero((char *) &server_address, sizeof(server_address));

    // configurando socket do server
    server_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr_list[0], &server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);

    // conectando ao servidor
    if(connect(server_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Connection failed"); exit(1);
    }

    return server_fd;
}