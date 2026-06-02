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

#define WORLD_WIDTH 2000
#define WORLD_HEIGHT 2000
#define PLAYER_SPEED 300.0f

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
    playerMovement.x *= PLAYER_SPEED * GetFrameTime();
    playerMovement.y *= PLAYER_SPEED * GetFrameTime();

    if (playerPosition.x + playerMovement.x > 0 && playerPosition.x + PLAYER_WIDTH + playerMovement.x < WORLD_WIDTH)
    {
        PlayerNextPosition.x = playerPosition.x + playerMovement.x;
    }
    else
    {
        PlayerNextPosition.x = playerPosition.x;
    }

    if (playerPosition.y + playerMovement.y > 0 && playerPosition.y + PLAYER_HEIGHT + playerMovement.y < WORLD_HEIGHT)
    {
        PlayerNextPosition.y = playerPosition.y + playerMovement.y;
    }
    else
    {
        PlayerNextPosition.y = playerPosition.y;
    }

    return PlayerNextPosition;
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

    // Posição inicial no centro do MUNDO, não da tela
    Vector2 initial_position = {WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f};
    Player player = player_create("Ronald", 0, initial_position);

    // NOVO: Configurando a Câmera 2D
    Camera2D camera = {0};
    camera.target = initial_position;                                             // Ponto que a câmera vai olhar
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f}; // Coloca o target no meio da tela
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        // 1. ATUALIZAÇÃO DA LÓGICA (Update)
        Vector2 playerMovement = (Vector2){0, 0};

        if (IsKeyDown(KEY_A))
            playerMovement.x -= 1;
        if (IsKeyDown(KEY_S))
            playerMovement.y += 1;
        if (IsKeyDown(KEY_D))
            playerMovement.x += 1;
        if (IsKeyDown(KEY_W))
            playerMovement.y -= 1;

        // Move o player
        player_move(&player, VerifyCollisionWithWalls(player, playerMovement));

        // NOVO: Atualiza o alvo da câmera para ser o centro do player
        Vector2 current_pos = get_player_position(player);
        camera.target = (Vector2){current_pos.x + (PLAYER_WIDTH / 2.0f), current_pos.y + (PLAYER_HEIGHT / 2.0f)};

        // 2. DESENHO (Draw)
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR); // Cor fora do mapa (ex: preto)

        // NOVO: Inicia o modo de Câmera. Tudo desenhado aqui dentro pertence ao "Mundo"
        BeginMode2D(camera);

        // Desenha o fundo do mapa (ex: um chão cinza escuro)
        DrawRectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, DARKGRAY);

        // Desenha uma grade (grid) para criar a percepção de movimento
        for (int i = 0; i <= WORLD_WIDTH; i += 100)
        {
            DrawLine(i, 0, i, WORLD_HEIGHT, GRAY); // Linhas verticais
        }
        for (int i = 0; i <= WORLD_HEIGHT; i += 100)
        {
            DrawLine(0, i, WORLD_WIDTH, i, GRAY); // Linhas horizontais
        }

        // Desenha os limites extremos do mapa (borda vermelha)
        DrawRectangleLines(0, 0, WORLD_WIDTH, WORLD_HEIGHT, RED);

        // Desenha o Player
        DrawRectangleV(get_player_position(player), (Vector2){.x = PLAYER_WIDTH, .y = PLAYER_HEIGHT}, RED);

        EndMode2D(); // Termina o modo de câmera

        // NOVO: Desenhos de UI (Interface). Tudo desenhado aqui fica fixo na tela!
        DrawText("WASD para mover", 10, 10, 20, LIGHTGRAY);
        DrawFPS(10, 40);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void init_setup()
{
    InitWindow(800, 400, WINDOW_NAME);
    SetTargetFPS(60); // NOVO: É boa prática travar o FPS para evitar consumo excessivo de CPU
}

// Retorna o descritor de arquivo do servidor
int connect_to_server(char *hostname, int port_number)
{
    int server_fd;
    struct sockaddr_in server_address;
    struct hostent *server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        perror("ERROR opening socket");
    exit(1);

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        perror("Error, no such rost");
        exit(1);
    }

    // limpando para nn ter problema com lixo de memoria
    bzero((char *)&server_address, sizeof(server_address));

    // configurando socket do server
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], &server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);

    // conectando ao servidor
    if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    return server_fd;
}