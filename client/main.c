#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"
#include "./player.h"
#include "./api.h"
#include "./packets.h"
#include "../config.h"
#include "./controller/client_controller.h"
#include "./aim.h"
#include "./weapon.h"

// ==================================================================
// GLOBAL GAME
ClientGame global_game_instance = {
    .num_bytes_in_buf = 0,
    .client_player_id = -1};
// ==================================================================

int connect_to_server(char *hostname, int port_number);
void read_packets(int server_fd, bool syscall_block);
void positions_players_interpolate(int player_id);
Vector2 process_delta_movement();
void draw_player(Player *p);

void init_setup();

// Tela de entrada de nome
void name_input_screen(char *buffer);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage %s hostname port\n", argv[0]);
        exit(1);
    }

    // Tela de entrada que solicita o nome antes do jogo começar
    init_setup();
    char buffer[MAX_INPUT_CHARS];
    name_input_screen(buffer);

    // ============== SERVIDOR ==============
    // Políticas de servidor
    char *hostname = argv[1];
    int port_number = atoi(argv[2]);

    int server_fd = connect_to_server(hostname, port_number);
    int player_id = -1;

    // =====================================
    // Se chegar aqui, esbeleceu conexão com o server. O primeiro passo
    // é enviar um pacote de request join. E aguardar o accept join.
    // =====================================
    request_join(server_fd, buffer);

    // Processa os pacotes devolvidos (request accept e, possivelmente, um snapshot)
    read_packets(server_fd, true);

    // Pega o player_id após nome ser inserido
    player_id = global_game_instance.client_player_id;

    // NOVO: Configurando a Câmera 2D
    Camera2D camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 0.75f; // zoom intermediário para ver boa parte do mapa

    // Configura a mira (module separado)
    aim_init(&global_game_instance.list_all_players[player_id].aim, PISTOL_AIM_RADIUS, WHITE);

    // Weapons: prepare one of each and a current weapon
    Weapon pistol, shotgun, sniper;
    weapon_init(&pistol, WEAPON_PISTOL);
    weapon_init(&shotgun, WEAPON_SHOTGUN);
    weapon_init(&sniper, WEAPON_SNIPER);
    Weapon *current_weapon = &pistol;


    while (!WindowShouldClose())
    {
        // Lê o snapshot do game
        read_packets(server_fd, false);

        if (player_id < 0)
            player_id = global_game_instance.client_player_id;

        // Salva a diferença de posição
        Vector2 delta_movement = process_delta_movement();

        // Move o player localmente e online.
        // Obs.: move localmente para o player local não ver delay.

        // Mas, para segurança, futuramente deve fazer uma validação se a posição que o player foi
        // faz sentido.
        Vector2 new_position_local = VerifyCollisionWithWalls(&global_game_instance.list_all_players[player_id], delta_movement);
        player_move(&global_game_instance.list_all_players[player_id], new_position_local);

        // Move online
        request_move_player(server_fd, player_id, delta_movement);

        // Configuração de câmera
        Vector2 current_pos = get_player_position(&global_game_instance.list_all_players[player_id]);
        // Coloca o target no meio da tela
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        // Atualiza o alvo da câmera para ser o centro do player
        camera.target = (Vector2){current_pos.x + (PLAYER_WIDTH / 2.0f), current_pos.y + (PLAYER_HEIGHT / 2.0f)};

        // ===================
        // Interpolação de movimento (para suavizar)
        positions_players_interpolate(player_id);

        Player *local_player = &global_game_instance.list_all_players[player_id];

        // Atualiza a mira com base na posição do player and camera
        update_player_aim(local_player, camera);
        request_aim_update(server_fd, get_player_aim(local_player));

        // Disparo: ao clicar, usa posição da mira para disparar
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            weapon_fire(current_weapon, get_player_position(local_player), get_player_aim_position(local_player));
        }

        // Atualiza projeteis
        float dt = GetFrameTime();
        weapon_update(current_weapon, dt);

        // Troca de arma (1=pistol,2=shotgun,3=sniper)
        if (IsKeyPressed(KEY_ONE)) {
            current_weapon = &pistol;
            set_radius_player_aim(local_player, PISTOL_AIM_RADIUS);
        } else if (IsKeyPressed(KEY_TWO)) {
            current_weapon = &shotgun;
            set_radius_player_aim(local_player, SHOTGUN_AIM_RADIUS);
        } else if (IsKeyPressed(KEY_THREE)) {
            current_weapon = &sniper;
            set_radius_player_aim(local_player, SNIPER_AIM_RADIUS);
        }
        // ====================

        // 2. DESENHO (Draw)
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR); // Cor fora do mapa (ex: preto)

        // NOVO: Inicia o modo de Câmera. Tudo desenhado aqui dentro pertence ao "Mundo"
        BeginMode2D(camera);

        // Desenha o fundo do mapa (ex: um chão cinza escuro)
        DrawRectangle(0, 0, MAP_WIDTH, MAP_HEIGHT, DARKGRAY);

        // Desenha uma grade (grid) para criar a percepção de movimento
        for (int i = 0; i <= MAP_WIDTH; i += 100)
        {
            DrawLine(i, 0, i, MAP_HEIGHT, GRAY); // Linhas verticais
        }
        for (int i = 0; i <= MAP_HEIGHT; i += 100)
        {
            DrawLine(0, i, MAP_WIDTH, i, GRAY); // Linhas horizontais
        }

        // Desenha os limites extremos do mapa (borda vermelha)
        DrawRectangleLines(0, 0, MAP_WIDTH, MAP_HEIGHT, RED);

        // Desenha todos os players
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            Player *p = &global_game_instance.list_all_players[i];
            if (player_is_connected(p))
            {
                draw_player(p);
            }
        }

        // Desenha projeteis da arma atual
        weapon_draw(current_weapon);

        EndMode2D(); // Termina o modo de câmera

        // NOVO: Desenhos de UI (Interface). Tudo desenhado aqui fica fixo na tela!
        DrawText("WASD para mover", 10, 10, 20, LIGHTGRAY);
        DrawFPS(10, 40);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// =======================================
// TELA DE ENTRADA DE NOME
// =======================================
void name_input_screen(char *buffer)
{
    char name[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount = 0;
    int framesCounter = 0;
    bool name_submitted = false;

    while (!WindowShouldClose() && !name_submitted)
    {
        framesCounter++;

        // Lê input do teclado
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125] (printable characters)
            if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
            {
                name[letterCount] = (char)key;
                name[letterCount + 1] = '\0'; // Add null terminator
                letterCount++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        // Handle backspace functionality
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            letterCount--;
            if (letterCount < 0)
                letterCount = 0;
            name[letterCount] = '\0';
        }

        // Handle ENTER to confirm name
        if (IsKeyPressed(KEY_ENTER) && letterCount > 0)
        {
            // atualiza o buffer com o nome setado
            strcpy(buffer, name);
            name_submitted = true;
        }

        // 2. DESENHO (Draw)
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Desenha o pedido de nome do jogador - Tela monótona
        DrawText("ENTER YOUR NAME:", 240, 140, 20, LIGHTGRAY);

        // Draw the input box outline
        DrawRectangleLines(240, 180, 320, 50, LIGHTGRAY);

        // Draw the text typed by the user
        DrawText(name, 250, 192, 24, WHITE);

        // Draw a blinking underscore cursor if under character limit
        if (letterCount < MAX_INPUT_CHARS)
        {
            // Blink every 30 frames (0.5 seconds at 60 FPS)
            if (((framesCounter / 30) % 2) == 0)
            {
                DrawText("_", 250 + MeasureText(name, 24), 192, 24, WHITE);
            }
        }
        else
        {
            DrawText("Press BACKSPACE to delete", 240, 250, 20, RED);
        }

        DrawText("Press ENTER to confirm", 240, 300, 20, YELLOW);
        DrawFPS(10, 10);

        EndDrawing();
    }
}

void init_setup()
{
    InitWindow(800, 400, WINDOW_NAME);
    SetTargetFPS(60); // NOVO: É boa prática travar o FPS para evitar consumo excessivo de CPU
}
// =======================================
// player
void draw_player(Player *p)
{
    Vector2 player_position = get_player_position(p);
    char *player_name = get_player_name(p);

    // Calcula a posição do Texto
    // Medimos a largura do texto para centralizar perfeitamente acima do quadrado
    int textWidth = MeasureText(player_name, FONT_SIZE);

    // X centralizado: centro do player menos a metade da largura do texto
    int textX = player_position.x + (PLAYER_WIDTH / 2) - (textWidth / 2);

    // Y acima do player: Y do player menos a altura da fonte e uma folga (ex: 10 pixels)
    int textY = player_position.y - FONT_SIZE - 10;

    // Desenha a mira (em coordenadas de mundo) - default como pistola (1)
    aim_draw(get_player_aim(p), 1);
    DrawText(player_name, textX, textY, FONT_SIZE, FONT_COLOR);
    DrawRectangleV(player_position, (Vector2){.x = PLAYER_WIDTH, .y = PLAYER_HEIGHT}, RED);
}

// =======================================
// movement interpolate
void positions_players_interpolate(int player_id)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        Player *p = &global_game_instance.list_all_players[i];
        if (player_is_connected(p))
        {
            if (i == player_id)
                continue;
            else
            {
                p->position = Vector2Lerp(p->position, p->target_position, MOVE_INTERPOLATION_FACTOR);
            }
        }
    }
}

Vector2 process_delta_movement()
{
    Vector2 delta_movement = (Vector2){0, 0};

    if (IsKeyDown(KEY_A))
        delta_movement.x -= 1;
    if (IsKeyDown(KEY_S))
        delta_movement.y += 1;
    if (IsKeyDown(KEY_D))
        delta_movement.x += 1;
    if (IsKeyDown(KEY_W))
        delta_movement.y -= 1;

    return delta_movement;
}

// ==============================================================================
// SOCKETS CONFIGURATION
void read_packets(int server_fd, bool syscall_block)
{
    int n_bytes = -1;
    if (syscall_block)
    {
        n_bytes = read(server_fd, (global_game_instance.buffer + global_game_instance.num_bytes_in_buf), BUFFER_SIZE - global_game_instance.num_bytes_in_buf);
    }
    else
    {
        n_bytes = recv(server_fd,
                       global_game_instance.buffer + global_game_instance.num_bytes_in_buf,
                       BUFFER_SIZE - global_game_instance.num_bytes_in_buf,
                       MSG_DONTWAIT);
    }

    // Se houve erro de leitura, retorna
    if (n_bytes < 0)
        return;

    // Atualiza a quantidade de bytes no buffer
    global_game_instance.num_bytes_in_buf += n_bytes;

    // Obs.: O tipo do pacote é a primeira informação e tem 4 bytes.
    while (global_game_instance.num_bytes_in_buf >= 4)
    {
        type_packet type = *(int *)(global_game_instance.buffer);
        int packet_size = get_packet_size(type);

        // Leitura de pacote parcialmente recebido
        if (global_game_instance.num_bytes_in_buf < packet_size)
            return;

        // Se for de movimento, faz o casting
        if (type == JOIN_ACCEPT)
        {
            PacketJoinAccept packet;
            memcpy(&packet, global_game_instance.buffer, sizeof(PacketJoinAccept));
            process_join_accept_packet(&packet, &global_game_instance);
        }
        else if (type == SNAPSHOT)
        {
            PacketSnapshot packet;
            memcpy(&packet, global_game_instance.buffer, sizeof(PacketSnapshot));
            process_snapshot_packet(&packet, &global_game_instance);
        }

        //===============================================================================
        // Deslocando os bytes restantes para o começo do buffer
        int bytes_left = global_game_instance.num_bytes_in_buf - packet_size;
        memmove(global_game_instance.buffer, (global_game_instance.buffer + packet_size), bytes_left);
        global_game_instance.num_bytes_in_buf = bytes_left;
    }
}

// ====================================

// Retorna o descritor de arquivo do servidor
int connect_to_server(char *hostname, int port_number)
{
    int server_fd;
    struct sockaddr_in server_address;
    struct hostent *server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        perror("Error, no such host");
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