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
#include "./player.h"
#include "./api.h"
#include "./packets.h"
#include "../config.h"
#include "./controller/client_controller.h"

// ==================================================================
// GLOBAL GAME
ClientGame global_game_instance = {
    .num_bytes_in_buf = 0,
    .client_player_id = -1
};
// ==================================================================



int connect_to_server(char *hostname, int port_number);
void read_packets(int server_fd);

void init_setup();

int main(int argc, char *argv[])
{
    // ============== SERVIDOR ==============
    // Políticas de servidor
    if(argc < 3) {
        fprintf(stderr, "Usage %s hostname port\n", argv[0]);
        exit(1);
    }

    char *hostname = argv[1];
    int port_number = atoi(argv[2]);
    int server_fd = connect_to_server(hostname, port_number);
    int player_id = -1;

    // =====================================
    // Se chegar aqui, conectou ao server, le os pacotes no buffer 
    // (o primeiro a chegar será) o pacote de join accept.
    // ==============
    read_packets(server_fd);

    init_setup();

    // NOVO: Configurando a Câmera 2D
    Camera2D camera = {0};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;


    while (!WindowShouldClose())
    {
        // Lê o snapshot do game
        read_packets(server_fd);

        if(player_id < 0) player_id = global_game_instance.client_player_id;

        // 1. ATUALIZAÇÃO DA LÓGICA (Update)
        Vector2 delta_movement = (Vector2){0, 0};

        if (IsKeyDown(KEY_A)) 
            delta_movement.x -= 1;
        if (IsKeyDown(KEY_S))
            delta_movement.y += 1;
        if (IsKeyDown(KEY_D))
            delta_movement.x += 1;
        if (IsKeyDown(KEY_W))
            delta_movement.y -= 1;

        // Move o player
        request_move_player(server_fd, player_id, delta_movement);

        Vector2 current_pos = get_player_position(global_game_instance.list_all_players[player_id]);

        // Coloca o target no meio da tela
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f}; 
        // Atualiza o alvo da câmera para ser o centro do player
        camera.target = (Vector2){current_pos.x + (PLAYER_WIDTH / 2.0f), current_pos.y + (PLAYER_HEIGHT / 2.0f)};

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
        for(int i = 0; i < MAX_PLAYERS; i++) {
            Player p = global_game_instance.list_all_players[i];
            if(player_is_connected(&p)) {
                DrawRectangleV(get_player_position(p), (Vector2){.x = PLAYER_WIDTH, .y = PLAYER_HEIGHT}, RED);
            }
            
        }

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



// ==============================================================================
// SOCKETS CONFIGURATION
void read_packets(int server_fd) {
    int n_bytes = read(server_fd, (global_game_instance.buffer + global_game_instance.num_bytes_in_buf), BUFFER_SIZE - global_game_instance.num_bytes_in_buf);

    // Se houve erro de leitura, retorna
    if(n_bytes < 0) return;

    // Atualiza a quantidade de bytes no buffer
    global_game_instance.num_bytes_in_buf += n_bytes;

    // Obs.: O tipo do pacote é a primeira informação e tem 4 bytes.
    printf("oi\n");
    while(global_game_instance.num_bytes_in_buf >= 4) {
        type_packet type = *(int *)(global_game_instance.buffer);
        int packet_size = get_packet_size(type);

        // Se for de movimento, faz o casting
        if(type == JOIN_ACCEPT) {
            PacketJoinAccept packet;
            memcpy(&packet, global_game_instance. buffer, sizeof(PacketJoinAccept));
            process_join_packet(&packet, &global_game_instance);
            printf("LI JOIN\n\n\n");
        }
        else if(type == SNAPSHOT) {
            PacketSnapshot packet;
            memcpy(&packet, global_game_instance. buffer, sizeof(PacketSnapshot));
            process_snapshot_packet(&packet, &global_game_instance);
            printf("LI SNAPSHOT\n\n\n");

        }

        //===============================================================================
        // Deslocando os bytes restantes para o começo do buffer
        int bytes_left = global_game_instance.num_bytes_in_buf - packet_size;
        memmove(global_game_instance.buffer, (global_game_instance.buffer + packet_size), bytes_left);
        global_game_instance.num_bytes_in_buf = bytes_left;
    }
}


// Retorna o descritor de arquivo do servidor
int connect_to_server(char *hostname, int port_number) {
    int server_fd;
    struct sockaddr_in server_address;
    struct hostent *server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
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
    if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    return server_fd;
}