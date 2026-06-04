#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>

// includes que precisam melhorar a organização
#include "../config.h"
#include "./player.h"
#include "./packets.h"
#include "./controller/movement_controller.h"

// Definindo o tickrate (60 vezes por segundo)
#define TIME_PER_FRAME 16667
#define MAX_EVENTS (MAX_PLAYERS + 1)

typedef struct
{
    int fd;
    Player player;

    // buffer config
    char buffer[BUFFER_SIZE];
    int num_bytes_in_buf;
} PlayerConnection;

// GLOBAL DATABASE ===========================
PlayerConnection players_connections[MAX_PLAYERS];
int epoll_fd;
// END GLOBAL DATABASE ===========================

// THREADS ===========================
void *snapshot_thread_function(void *arg);
void *create_new_server_player_thread_function(void *arg);

// END THREADS ===========================

int create_and_bind_passive_socket(int port_number);
void read_packets(PlayerConnection *player_connection, bool syscall_block);
static void create_new_server_player(int client_fd);
SnapShot generate_snapshot();

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("Port number not provided.");
        exit(1);
    }

    // Bloqueando SIGPIPE (Para tratar se algum jogador desconectar)
    signal(SIGPIPE, SIG_IGN);

    // ===================
    // DATABASE. No início, não ha player conectado.
    for (int i = 0; i < MAX_PLAYERS; i++)
        disconnect_player(&players_connections[i].player);

    // Criando epoll para escutar quando algum cliente se comunicar
    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1) {
        perror("Erro ao criar epoll");
        exit(1);
    }
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
    // Adicionando socket passivo no epoll
    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.ptr = &server_sockfd 
    };
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sockfd, &ev) == -1) {
        perror("Erro ao adicionar o socket passivo ao epoll.\n");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];

    // ===================
    // Iniciando threads
    pthread_t snapshot_thread;
    pthread_t new_player_thread;
    pthread_create(&snapshot_thread, NULL, snapshot_thread_function, NULL);
    // ==================

    while (1)
    {
        
        // Aguarda atividade de um dos sockets do epoll. Fica bloqueado aqui por, no maximo, o tempo do timeout
        // Observação:        
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for(int i = 0; i < num_events; i++) {
            // Caso 1. Socket principal. Um novo player se conectou
            // Ja adiciona ao epoll após criar
            if(events[i].data.ptr == &server_sockfd) {
                client_sockfd = accept(server_sockfd, NULL, NULL);
                pthread_create(&new_player_thread, NULL, create_new_server_player_thread_function, &client_sockfd);
            }
            // Caso 2. A comunicação aconteceu por outro socket (que não é o passivo)
            else {
                PlayerConnection *conn = (PlayerConnection *) events[i].data.ptr;
                read_packets(conn, false);
            }
        }
    }


    // Fechando conexao
    close(epoll_fd);
    close(server_sockfd);
}

// ============ PLAYER ENTER ========================
static void create_new_server_player(int client_fd)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (player_is_connected(&players_connections[i].player) == false)
        {
            Vector2 position = {MAP_WIDTH / 2, MAP_HEIGHT / 2};
            int player_id = i;
            players_connections[i].player = player_create("Ronald", player_id, position);
            players_connections[i].fd = client_fd;
            players_connections[i].num_bytes_in_buf = 0;

            // Lendo pacote de join request. Fica bloqueado aqui.
            read_packets(&players_connections[i], true);

            // Ao desbloquear, faz a conexão do player no jogo.
            // Faz a conexão do player por último para evitar inconsistencias
            connect_player(&players_connections[i].player);

            // Adiciona a conexa do novo player ao manipulador (epoll) de fd's
            struct epoll_event client_ev = {
                .data.ptr = &players_connections[i],
                .events = EPOLLIN
            };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);

            // Enviando pacote de join accept
            PacketJoinAccept packet = create_join_accept_packet(player_id);
            send_packet(client_fd, JOIN_ACCEPT, &packet);

            return;
        }
    }
}

// ============ FINAL PLAYER ENTER ======================

SnapShot generate_snapshot()
{
    SnapShot snapshot;

    // (zera todos os slots de jogadores) (valgrind não reclamar de lixo)
    memset(&snapshot, 0, sizeof(SnapShot));

    // Separing players
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        snapshot.list_all_players[i] = players_connections[i].player;
    }

    return snapshot;
}

// ============ FINAL PLAYER ENTER ======================

// ============ LEITURA DE PACOTE =======================
void read_packets(PlayerConnection *player_connection, bool syscall_block)
{   
    // Obs.: Offset para não sobrescrever o buffer local.
    // Lê até (no máximo) o que falta para completar o buffer
    int n_bytes = -1;
    if(syscall_block == true) {
        n_bytes = read(player_connection->fd, (player_connection->buffer + player_connection->num_bytes_in_buf), BUFFER_SIZE - player_connection->num_bytes_in_buf);
    } else {
        n_bytes = recv(player_connection->fd, (player_connection->buffer + player_connection->num_bytes_in_buf), BUFFER_SIZE - player_connection->num_bytes_in_buf, MSG_DONTWAIT);
    }

    // Se houve erro de leitura, retorna
    if (n_bytes < 0)
        return;

    // Atualiza a quantidade de bytes no buffer
    player_connection->num_bytes_in_buf += n_bytes;

    // Obs.: O tipo do pacote é a primeira informação e tem 4 bytes.
    while (player_connection->num_bytes_in_buf >= 4)
    {
        type_packet type = *(int *)(player_connection->buffer);
        int packet_size = get_packet_size(type);

        // Se o pacote não está todo no buffer, não lê
        if (player_connection->num_bytes_in_buf < packet_size)
            return;

        // Se for de movimento, faz o casting
        if (type == MOVEMENT)
        {
            PacketMove packet = *(PacketMove *)(player_connection->buffer);
            process_movement_packet(&player_connection->player, packet);
        }
        // Se for de join request, pega o nome e salva
        else if(type == JOIN_REQUEST) { 
            PacketJoinRequest packet = *(PacketJoinRequest *)(player_connection->buffer);
            strcpy(player_connection->player.name, packet.name);
        }
        //===============================================================================
        // Deslocando os bytes restantes para o começo do buffer
        int bytes_left = player_connection->num_bytes_in_buf - packet_size;
        memmove(player_connection->buffer, (player_connection->buffer + packet_size), bytes_left);
        player_connection->num_bytes_in_buf = bytes_left;
    }
}

int create_and_bind_passive_socket(int port_number)
{
    int server_sockfd, n;
    struct sockaddr_in server_addr;

    // reset
    bzero((char *)&server_addr, sizeof(server_addr));

    // Criando socket passivo do servidor
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        perror("Error opening socket.");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // bind do fd com o addr
    n = bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    return server_sockfd;
}

///===========================================================================================
// thread
void *snapshot_thread_function(void *arg)
{
    // só para não relamar (warning)

    struct timeval start_time, end_time;
    long elapsed_time, sleep_time;

    while (true)
    {
        // 1. Marca o início do ciclo
        gettimeofday(&start_time, NULL);
        SnapShot snapshot;
        memset(&snapshot, 0, sizeof(SnapShot));
        snapshot = generate_snapshot();
        PacketSnapshot packet = create_snapshot_packet(&snapshot);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            errno = 0;
            if (player_is_connected(&players_connections[i].player))
            {
                send_packet(players_connections[i].fd, SNAPSHOT, &packet);

                if (errno == EPIPE)
                {   
                    // Desconectando
                    disconnect_player(&players_connections[i].player);
                    // Removendo do epoll
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, players_connections[i].fd, NULL);
                    // Fechando fd
                    close(players_connections[i].fd);
                }
            }
        }
        // ---------------------------

        // 2. Marca o fim do ciclo e calcula quanto tempo o trabalho demorou
        gettimeofday(&end_time, NULL);
        elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                       (end_time.tv_usec - start_time.tv_usec);

        // 3. Calcula quanto tempo sobra para completar os 16.6ms
        sleep_time = TIME_PER_FRAME - elapsed_time;

        // 4. Se sobrou tempo, a thread dorme o restante.
        if (sleep_time > 0)
        {
            usleep(sleep_time);
        }
    }

    return NULL;
}


void *create_new_server_player_thread_function(void *arg) {
    int client_fd = *(int *)arg;
    create_new_server_player(client_fd);

    return NULL;
}
// END THREADS ===========================