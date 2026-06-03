#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <time.h>
#include <stdbool.h>

// includes que precisam melhorar a organização
#include "../config.h"
#include "./player.h"
#include "./packets.h"


// controller imports
#include "./controller/movement_controller.h"

// Definindo o tickrate (60 vezes por segundo)
#define TICK_RATE 60.0
#define TICK_INTERVAL_SEC (1.0 / TICK_RATE) // ~0.0166 segundos
#define TICK_PERIOD_USEC 16667

void assert(int cond, const char * msg) {
    if(cond) {
        perror(msg);
        exit(1);
    }
}

typedef struct {
    int fd;
    Player player;

    // buffer config
    char buffer[BUFFER_SIZE];
    int num_bytes_in_buf;
} PlayerConnection;

int create_and_bind_passive_socket(int port_number);
void read_packets(PlayerConnection *player_connection);
static void create_new_server_player(PlayerConnection *players_connection_list, int client_fd);
SnapShot generate_snapshot(PlayerConnection *player_connections_list);

int main(int argc, char *argv[]) {
    assert((argc < 2), "Port number not provided.");

    // ===================
    // DATABASE. No início, não ha player conectado.
    PlayerConnection players_connections[MAX_PLAYERS];
    for(int i = 0; i < MAX_PLAYERS; i++)
        disconnect_player(&players_connections[i].player);

    // Criando set para escutar quando algum cliente se comunicar
    fd_set set_fds; // set de file descriptors
    int max_fd; // maior file descriptor
    // ===================


    // ===================
    // SERVER AND CLIENT CONFIG
    int port_number = atoi(argv[1]);
    int server_sockfd = create_and_bind_passive_socket(port_number);

    // client config
    int client_sockfd;

    // Definindo como um socket passivo
    listen(server_sockfd, MAX_PLAYERS);

    // ===================
    // LOOP PRINCIPAL

    while(1) {
        // Configurando set de fds. (Reseta lista a cada iteração e adiciona os FDs alvo)
        FD_ZERO(&set_fds);    
            // -> Adicionando socket principal ao set de sockets
        FD_SET(server_sockfd, &set_fds);
        max_fd = server_sockfd;

            // -> Adicionando cada player ativo ao set de sockets
        for(int i = 0; i < MAX_PLAYERS; i++)
            if(player_is_connected(&players_connections[i].player)) {
                FD_SET(players_connections[i].fd, &set_fds);
                if(players_connections[i].fd > max_fd)
                    max_fd = players_connections[i].fd;
            }
                

        // Aguarda atividade de um dos sockets do set. Fica bloqueado aqui. 
        // Observação:
        // -> Como quero que o jogo rode a 60fps, vou fazer um timeout no select para que o select desbloqueie 60x por segundo.
        struct timeval timeout;
        timeout.tv_sec = 0;  // 0 segundos
        timeout.tv_usec = TICK_PERIOD_USEC; // 0 microssegundos

        select(max_fd + 1, &set_fds, NULL, NULL, &timeout);

        // Caso 1. Socket principal. Um novo player se conectou
        if(FD_ISSET(server_sockfd, &set_fds)) {
            // -> Aceita conexao
            client_sockfd = accept(server_sockfd, NULL, NULL);

            // Cria e guarda player. Atenção: envia informações iniciais ao player
            create_new_server_player(players_connections, client_sockfd);
        }

        // Caso 2. A comunicação aconteceu por outro socket (que não é o passivo)
        for(int i = 0; i < MAX_PLAYERS; i++) {
            // Encontrou socket que se comunicou
            if(player_is_connected(&players_connections[i].player) && FD_ISSET(players_connections[i].fd, &set_fds)) {
                read_packets(&players_connections[i]);
            }
        }

        // Envia um novo snapshot aos players
        SnapShot snapshot = generate_snapshot(players_connections);
        PacketSnapshot packet = create_snapshot_packet(&snapshot);
        for(int i = 0; i < MAX_PLAYERS; i++) {
            if(player_is_connected(&players_connections[i].player)) {                
                send_packet(players_connections[i].fd, SNAPSHOT, &packet);
            }
        }

    }
}


// ============ PLAYER ENTER ========================
static void create_new_server_player(PlayerConnection *players_connection_list, int client_fd) {
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if(player_is_connected(&players_connection_list[i].player) == false) {
            Vector2 position = { MAP_WIDTH/2, MAP_HEIGHT/2 };
            int player_id = i;
            players_connection_list[i].player = player_create("Ronald", player_id, position);
            players_connection_list[i].fd = client_fd;
            players_connection_list[i].num_bytes_in_buf = 0;

            connect_player(&players_connection_list[i].player);

            // Enviando pacote de join accept
            PacketJoinAccept packet = create_join_accept_packet(player_id);
            fprintf(stdout, "enviando o pacote de join\n");
            fflush(stdout);
            send_packet(client_fd, JOIN_ACCEPT, &packet);

            return;
        }
    }
}

// ============ FINAL PLAYER ENTER ======================

SnapShot generate_snapshot(PlayerConnection *player_connections_list) {
    SnapShot snapshot;

    // Separing players    
    for(int i = 0; i < MAX_PLAYERS; i++) {
        snapshot.list_all_players[i] = player_connections_list[i].player;
    }
    
    return snapshot;
}

// ============ FINAL PLAYER ENTER ======================

// ============ LEITURA DE PACOTE =======================
void read_packets(PlayerConnection *player_connection) {
    // Obs.: Offset para não sobrescrever o buffer local.
    // Lê até (no máximo) o que falta para completar o buffer
    int n_bytes = read(player_connection->fd, (player_connection->buffer + player_connection->num_bytes_in_buf), BUFFER_SIZE - player_connection->num_bytes_in_buf);

    // Se houve erro de leitura, retorna
    if(n_bytes < 0) return;

    // Atualiza a quantidade de bytes no buffer
    player_connection->num_bytes_in_buf += n_bytes;

    // Obs.: O tipo do pacote é a primeira informação e tem 4 bytes.
    while(player_connection->num_bytes_in_buf >= 4) {
        type_packet type = *(int *)(player_connection->buffer);
        int packet_size = get_packet_size(type);

        // Se for de movimento, faz o casting
        if(type == MOVEMENT) {
            //PacketMove packet = *(PacketMove *)(player_connection->buffer);
            PacketMove packet;
            memcpy(&packet, player_connection->buffer, sizeof(PacketMove));
            process_movement_packet(&player_connection->player, packet);
        }


        //===============================================================================
        // Deslocando os bytes restantes para o começo do buffer
        int bytes_left = player_connection->num_bytes_in_buf - packet_size;
        memmove(player_connection->buffer, (player_connection->buffer + packet_size), bytes_left);
        player_connection->num_bytes_in_buf = bytes_left;
    }
}

int create_and_bind_passive_socket(int port_number) {
    int server_sockfd, n;
    struct sockaddr_in server_addr;
    
    // reset
    bzero((char *) &server_addr, sizeof(server_addr));

    // Criando socket passivo do servidor
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert((server_sockfd < 0), "Error opening socket.");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // bind do fd com o addr
    n = bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    assert((n < 0), "Binding failed");

    return server_sockfd;
}