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
#include "../player.h"
#include "../config.h"
#include "./packets.h"

// Definindo o tickrate (60 vezes por segundo)
#define TICK_RATE 60.0
#define TICK_INTERVAL_SEC (1.0 / TICK_RATE) // ~0.0166 segundos

#define MAX_CONNECTIONS 30

void assert(int cond, const char * msg) {
    if(cond) {
        perror(msg);
        exit(1);
    }
}

typedef struct {
    int fd;
    bool is_connected;
    Player player;
} PlayerConnection;

int create_and_bind_passive_socket(int port_number);

int main(int argc, char *argv[]) {
    assert((argc < 2), "Port number not provided.");

    // ===================
    // DATABASE. No início, não ha player conectado.
    PlayerConnection players[MAX_CONNECTIONS];
    for(int i = 0; i < MAX_CONNECTIONS; i++)
        players[i].is_connected = 0;

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
    listen(server_sockfd, MAX_CONNECTIONS);

    // ===================
    // LOOP PRINCIPAL

    // Contador de tempo (envio de snapshot)
    struct timespec last_time, current_time;

    while(1) {
        // Configurando set de fds. (Reseta lista a cada iteração e adiciona os FDs alvo)
        FD_ZERO(&set_fds);    
            // -> Adicionando socket principal ao set de sockets
        FD_SET(server_sockfd, &set_fds);
        max_fd = server_sockfd;

            // -> Adicionando cada player ativo ao set de sockets
        for(int i = 0; i < MAX_CONNECTIONS; i++)
            if(players[i].is_connected) {
                FD_SET(players[i].fd, &set_fds);
                if(players[i].fd > max_fd)
                    max_fd = players[i].fd;
            }
                

        // Aguarda atividade de um dos sockets do set. Fica bloqueado aqui. 
        // Observação:
        // -> Como quero que o jogo rode a 60fps, vou fazer um timeout no select para que o select desbloqueie 60x por segundo.
        select(max_fd + 1, &set_fds, NULL, NULL, NULL);

        // Caso 1. Socket principal. Um novo player se conectou
        if(FD_ISSET(server_sockfd, &set_fds)) {
            // -> Aceita conexao
            client_sockfd = accept(server_sockfd, NULL, NULL);

            // Cria e guarda player. Atenção: envia informações iniciais ao player
            create_new_server_player(players, client_sockfd);
        }

        // Caso 2. A comunicação aconteceu por outro socket (que não é o passivo)
        for(int i = 0; i < MAX_CONNECTIONS; i++) {
            // Encontrou socket que se comunicou
            if(players[i].is_connected && FD_ISSET(players[i].fd, &set_fds)) {

            }
        }


    }
}


// ============ PLAYER ENTER ========================
static void create_new_server_player(PlayerConnection *players_connection_list, int client_fd) {
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        if(players_connection_list[i].is_connected == 0) {
            Vector2 size = {40, 40};
            Vector2 position = { MAP_WIDTH/2, MAP_HEIGHT/2 };
            int player_id = i;
            players_connection_list[i].player = player_create("Ronald", player_id, position);
            players_connection_list[i].fd = client_fd;
            players_connection_list[i].is_connected = true;

            send_accept_join_packet(client_fd, player_id);
            return;
        }
    }
}

static void send_accept_join_packet(int client_fd, int player_id) {
    PacketJoinAccept packet = {
        .type = JOIN_ACCEPT,
        .player_id = player_id
    };

    write(client_fd, &packet, sizeof(packet));
}

// ============ FINAL PLAYER ENTER ======================

SnapShotGame generate_snapshot_game(PlayerConnection *player_connections_list) {
    SnapShotGame snapshot;

    // Separing players    
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        snapshot.list_all_players[i] = player_connections_list[i].player;
    }
    
    return snapshot;
}

static void send_snapshot_packet(int client_fd, SnapShotGame snapshot) {
    PacketSnapshot packet = {
        .type = SNAPSHOT,
        .snapshot = snapshot
    };

    write(client_fd, &packet, sizeof(packet));
}
// ============ FINAL PLAYER ENTER ======================


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