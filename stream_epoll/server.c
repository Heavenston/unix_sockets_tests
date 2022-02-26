#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "utils.h"
#include "client_server.h"

typedef struct {
    int fd;
    int id;
    char username[20];
} Socket;

void server_main() {
    int result;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    unlink(SERVER_PATH);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        report_error("Could not bind socket");

    result = listen(fd, 1);
    if (result == -1)
        report_error("Could not listen for sockets");

    {
        int value = 1;
        socklen_t length = sizeof(int);
        result = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &value, length);
        if (result == -1)
            report_error("Could not read option");
    }

    // Values
    int socket_id_counter = 1;
    Socket *sockets[100];
    memset(sockets, 0, sizeof(sockets));

    // EPOLL SETUP
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        report_error("Could not create the epoll fd");

    Socket *server_socket = malloc(sizeof(Socket));
    server_socket->fd = fd;
    server_socket->id = 0;
    strcpy(server_socket->username, "server");
    sockets[0] = server_socket;

    {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.u32 = 0;
        result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if (result == -1)
            report_error("Could not add server socket file descriptor to epoll");
    }

    printf("Waiting for conncetions...\n");

    // EPOLL LOOP

    while (1) {
        struct epoll_event event;
        result = epoll_wait(epoll_fd, &event, 1, -1);
        if (result == -1)
            report_error("Error during epoll_wait");
        Socket *socket = sockets[event.data.u32];

        if (socket->id == 0) {
            struct sockaddr_un connected_addr;
            socklen_t connected_addr_length = sizeof(struct sockaddr_un);
            int accepted_fd = accept(fd, (struct sockaddr*)&connected_addr, &connected_addr_length);
            if (accepted_fd == -1)
                report_error("Error while accepting a socket");
            
            printf("Socket connection received, assigning id %d\n", socket_id_counter);

            int index = 1;
            while (index < 100 && sockets[index] != NULL)
                index++;

            Socket *new_socket = malloc(sizeof(Socket));
            new_socket->fd = accepted_fd;
            new_socket->id = socket_id_counter++;
            sprintf(new_socket->username, "%i", new_socket->id);
            sockets[index] = new_socket;

            struct epoll_event new_event;
            new_event.data.u32 = index;
            new_event.events = EPOLLIN;
            result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accepted_fd, &new_event);
            if (result == -1)
                report_error("Could not add new socket to epoll");

            handshake_packet_t handshake_packet = { new_socket->id };
            result = send_packet(accepted_fd, &handshake_packet, sizeof(handshake_packet_t));
            if (result == -1)
                report_error("Could not send handshake packet");
        }
        else {
            char *received_data;
            int received_bytes = recv_packet(socket->fd, (void**)&received_data);
            if (received_bytes > 0) {
                // PROBLEM: If received data isn't a null-terminated string
                printf("[%s] %s\n", socket->username, received_data);

                server_message_t *message = malloc(sizeof(server_message_t) + received_bytes);
                message->sender_id = socket->id;
                strcpy(message->username, socket->username);
                strcpy(message->message, received_data);
                for (int i = 1; i < 100; i++) {
                    if (sockets[i] == NULL || i == event.data.u32)
                        continue;
                    Socket *socket_b = sockets[i];

                    result = send_packet(socket_b->fd, message, sizeof(server_message_t) + received_bytes);
                    if (result == -1)
                        report_error("Could not send packet");
                }

                free(message);
                free(received_data);
            }
            else {
                if (received_bytes == 0) {
                    printf("Client #%i disconnected\n", socket->id);
                }
                else {
                    printf("Error while reading data from #%i: ", socket->id);
                    print_errno();
                }

                result = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket->fd, NULL);
                if (result == -1)
                    report_error("Could not delete socket from epoll");

                free(socket);
                sockets[event.data.u32] = NULL;
            }
        }
    }

    // CLEANING UP
    result = close(epoll_fd);
    if (result == -1)
        report_error("Could not close the epoll fd");

    result = shutdown(fd, SHUT_RDWR);
    if (result == -1)
        report_error("Could not shutdown socket");
}
