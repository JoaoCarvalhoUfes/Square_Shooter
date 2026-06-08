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
#include "../shared/player.h"
#include "../shared/packets.h"
#include "api.h"
#include "controller/client_controller.h"
#include "../config.h"
#include "../shared/utils.h"
#include "../shared/aim.h"
#include "../shared/weapon.h"

// ==================================================================
// GLOBAL GAME
ClientGame global_game_instance = {
    .num_bytes_in_buf = 0,
    .client_player_id = -1};
Weapon all_players_weapons[MAX_PLAYERS][3];
float player_damage_timer[MAX_PLAYERS] = {0};
// ==================================================================

int connect_to_server(char *hostname, int port_number);
void read_packets(int server_fd, bool syscall_block);
void positions_players_interpolate(int player_id);
Vector2 process_delta_movement();
void draw_player(Player *p);
void draw_game_world(Camera2D camera, float death_timer);

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

    init_setup();

    // Calls the name input screen, which updates the buffer with the name chosen by the user
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

    for (int i = 0; i < MAX_PLAYERS; i++) {
        weapon_init(&all_players_weapons[i][0], WEAPON_PISTOL);
        weapon_init(&all_players_weapons[i][1], WEAPON_SHOTGUN);
        weapon_init(&all_players_weapons[i][2], WEAPON_SNIPER);
    }
    
    Weapon *current_weapon = &all_players_weapons[player_id][0];

    while (!WindowShouldClose())
    {
        // Lê o snapshot do game
        read_packets(server_fd, false);

        if (player_id < 0)
            player_id = global_game_instance.client_player_id;

        // Salva a diferença de posição
        Vector2 raw_movement = process_delta_movement();

        Vector2 pixel_offset = normalize_movement(raw_movement);
        pixel_offset.x *= PLAYER_SPEED * GetFrameTime();
        pixel_offset.y *= PLAYER_SPEED * GetFrameTime();

        Vector2 new_position_local = verify_collision_with_walls(&global_game_instance.list_all_players[player_id], pixel_offset);
        Player *local_player = &global_game_instance.list_all_players[player_id];
        
        // Move online if alive
        if (local_player->life > 0) {
            player_move(&global_game_instance.list_all_players[player_id], new_position_local);
            request_move_player(server_fd, player_id, raw_movement);
        }

        // Configuração de câmera
        Vector2 current_pos = get_player_position(&global_game_instance.list_all_players[player_id]);
        // Coloca o target no meio da tela
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        // Atualiza o alvo da câmera para ser o centro do player
        camera.target = (Vector2){current_pos.x + (PLAYER_WIDTH / 2.0f), current_pos.y + (PLAYER_HEIGHT / 2.0f)};

        // ===================
        // Interpolação de movimento (para suavizar)
        positions_players_interpolate(player_id);

        // Atualiza a mira com base na posição do player and camera
        update_player_aim(local_player, camera);
        request_aim_update(server_fd, get_player_aim(local_player));

        // Sincroniza a posição visual da mira dos outros jogadores com suas posições interpoladas
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i != player_id && player_is_connected(&global_game_instance.list_all_players[i])) {
                Player *p = &global_game_instance.list_all_players[i];
                Vector2 center = {p->position.x + PLAYER_WIDTH/2.0f, p->position.y + PLAYER_HEIGHT/2.0f};
                p->aim.pos.x = center.x + p->aim.dir.x * p->aim.distance;
                p->aim.pos.y = center.y + p->aim.dir.y * p->aim.distance;
            }
        }

        // Disparo: ao clicar, usa posição da mira para disparar
        if (local_player->life > 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 origin = get_player_position(local_player);
            Vector2 player_center = (Vector2){origin.x + (PLAYER_WIDTH / 2.0f), origin.y + (PLAYER_HEIGHT / 2.0f)};
            weapon_fire(current_weapon, player_center, local_player->aim.dir);
            request_shoot(server_fd, player_id);
        }

        // Atualiza projeteis e timers
        float dt = GetFrameTime();
        static float local_respawn_timer = 0.0f;
        
        if (local_player->life <= 0 && local_respawn_timer <= 0) {
            local_respawn_timer = 3.0f;
        } else if (local_respawn_timer > 0) {
            local_respawn_timer -= dt;
            if (local_respawn_timer < 0) local_respawn_timer = 0;
        }

        static int last_frame_life[MAX_PLAYERS] = {0};
        
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (player_is_connected(&global_game_instance.list_all_players[i])) {
                weapon_update(&all_players_weapons[i][0], dt);
                weapon_update(&all_players_weapons[i][1], dt);
                weapon_update(&all_players_weapons[i][2], dt);
                
                // --- CLIENT SIDE COLLISION TO DESTROY PROJECTILES VISUALLY ---
                for (int w = 0; w < 3; w++) {
                    Weapon *weapon = &all_players_weapons[i][w];
                    if (weapon->type == WEAPON_SNIPER) continue; // Sniper is a laser, doesn't disappear on hit
                    for (int p_idx = 0; p_idx < MAX_PROJECTILES; p_idx++) {
                        Projectile *proj = &weapon->projectiles[p_idx];
                        if (!proj->active) continue;
                        
                        for (int j = 0; j < MAX_PLAYERS; j++) {
                            if (i == j) continue;
                            if (!player_is_connected(&global_game_instance.list_all_players[j])) continue;
                            
                            Vector2 target_pos = get_player_position(&global_game_instance.list_all_players[j]);
                            Rectangle target_rec = {target_pos.x, target_pos.y, PLAYER_WIDTH, PLAYER_HEIGHT};
                            
                            if (CheckCollisionCircleRec(proj->pos, 8.0f, target_rec)) {
                                proj->active = false;
                                break;
                            }
                        }
                    }
                }
                // -------------------------------------------------------------
                
                int current_life = global_game_instance.list_all_players[i].life;
                if (current_life < last_frame_life[i] && last_frame_life[i] > 0) {
                    player_damage_timer[i] = 0.2f; // pisca vermelho por 200ms
                }
                last_frame_life[i] = current_life;
                
                if (player_damage_timer[i] > 0) player_damage_timer[i] -= dt;
            } else {
                last_frame_life[i] = 0;
                player_damage_timer[i] = 0;
            }
        }

        // Troca de arma (1=pistol,2=shotgun,3=sniper)
        if (IsKeyPressed(KEY_ONE)) {
            current_weapon = &all_players_weapons[player_id][0];
            set_player_aim(local_player, WEAPON_PISTOL);
            set_player_weapon(local_player, WEAPON_PISTOL);
            request_weapon_update(server_fd, WEAPON_PISTOL);
        } else if (IsKeyPressed(KEY_TWO)) {
            current_weapon = &all_players_weapons[player_id][1];
            set_player_aim(local_player, WEAPON_SHOTGUN);
            set_player_weapon(local_player, WEAPON_SHOTGUN);
            request_weapon_update(server_fd, WEAPON_SHOTGUN);
        } else if (IsKeyPressed(KEY_THREE)) {
            current_weapon = &all_players_weapons[player_id][2];
            set_player_aim(local_player, WEAPON_SNIPER);
            set_player_weapon(local_player, WEAPON_SNIPER);
            request_weapon_update(server_fd, WEAPON_SNIPER);
        }
        // ====================

        // 2. DESENHO (Draw)
        draw_game_world(camera, local_player->life <= 0 ? local_respawn_timer : 0.0f);
    }

    CloseWindow();
    return 0;
}

void draw_game_world(Camera2D camera, float death_timer)
{
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
        if (player_is_connected(p) && p->life > 0)
        {
            draw_player(p);
            
            weapon_draw(&all_players_weapons[i][0]);
            weapon_draw(&all_players_weapons[i][1]);
            weapon_draw(&all_players_weapons[i][2]);
        }
    }

    EndMode2D(); // Termina o modo de câmera

    DrawText("WASD para mover", 10, 10, 20, LIGHTGRAY);
    DrawFPS(10, 40);

    // Death Screen UI
    if (death_timer > 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
        const char *deathText = "VOCE MORREU!";
        const char *respawnText = TextFormat("Respawn em: %.1f", death_timer);
        
        int w1 = MeasureText(deathText, 40);
        int w2 = MeasureText(respawnText, 30);
        
        DrawText(deathText, GetScreenWidth()/2 - w1/2, GetScreenHeight()/2 - 40, 40, RED);
        DrawText(respawnText, GetScreenWidth()/2 - w2/2, GetScreenHeight()/2 + 10, 30, WHITE);
    }

    EndDrawing();
}

void name_input_screen(char *buffer)
{
    char name[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount = 0;
    int framesCounter = 0;
    bool name_submitted = false;

    while (!WindowShouldClose() && !name_submitted)
    {
        framesCounter++;

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
    SetTargetFPS(60); 
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
    WeaponType player_weapon = get_player_weapon(p);
    aim_draw(get_player_aim(p), get_weapon_shape_by_type(player_weapon));
    DrawText(player_name, textX, textY, FONT_SIZE, FONT_COLOR);
    
    Color playerColor = BLACK;
    if (player_damage_timer[p->id] > 0) {
        playerColor = RED;
    }
    
    DrawRectangleV(player_position, (Vector2){.x = PLAYER_WIDTH, .y = PLAYER_HEIGHT}, playerColor);
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
        else if (type == SHOOT)
        {
            PacketShoot packet;
            memcpy(&packet, global_game_instance.buffer, sizeof(PacketShoot));
            if (packet.player_id != global_game_instance.client_player_id) {
                Player *p = &global_game_instance.list_all_players[packet.player_id];
                WeaponType w_type = get_player_weapon(p);
                Weapon *w = NULL;
                if (w_type == WEAPON_PISTOL) w = &all_players_weapons[packet.player_id][0];
                else if (w_type == WEAPON_SHOTGUN) w = &all_players_weapons[packet.player_id][1];
                else if (w_type == WEAPON_SNIPER) w = &all_players_weapons[packet.player_id][2];

                if (w) {
                    Vector2 origin = get_player_position(p);
                    Vector2 p_center = (Vector2){origin.x + (PLAYER_WIDTH / 2.0f), origin.y + (PLAYER_HEIGHT / 2.0f)};
                    weapon_fire(w, p_center, p->aim.dir);
                }
            }
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