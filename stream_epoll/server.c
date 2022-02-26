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
} Socket;

void server_main() {
    int result;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    //fcntl(fd, F_SETFL, O_NONBLOCK);

    unlink(SERVER_PATH);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        report_error("Could not bind socket");

    result = listen(fd, 1);
    if (result == -1)
        report_error("Could not listen for sockets");

    // EPOLL SETUP
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        report_error("Could not create the epoll fd");

    {
        struct epoll_event event;
        event.events = EPOLLIN;
        Socket *socket = malloc(sizeof(Socket));
        socket->fd = fd;
        socket->id = 0;
        event.data.ptr = socket;
        result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if (result == -1)
            report_error("Could not add server socket file descriptor to epoll");
    }

    printf("Waiting for conncetions...\n");

    // EPOLL LOOP
    int socket_id_counter = 1;
    while (1) {
        struct epoll_event event;
        result = epoll_wait(epoll_fd, &event, 1, -1);
        if (result == -1)
            report_error("Error during epoll_wait");
        Socket *socket = (Socket*)event.data.ptr;

        if (socket->id == 0) {
            struct sockaddr_un connected_addr;
            socklen_t connected_addr_length = sizeof(struct sockaddr_un);
            int accepted_fd = accept(fd, (struct sockaddr*)&connected_addr, &connected_addr_length);
            if (accepted_fd == -1)
                report_error("Error while accepting a socket");
            
            printf("Socket connection received, assigning id %d\n", socket_id_counter);

            struct epoll_event new_event;

            Socket *new_socket = malloc(sizeof(Socket));
            new_socket->fd = accepted_fd;
            new_socket->id = socket_id_counter++;
            new_event.data.ptr = new_socket;
            new_event.events = EPOLLIN;

            result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accepted_fd, &new_event);
            if (result == -1)
                report_error("Could not add new socket to epoll");
        }
        else {
            char *received_data;
            int received_bytes = recv_packet(socket->fd, (void**)&received_data);
            if (received_bytes > 0) {
                // PROBLEM: If received data isn't a null-terminated string
                printf("[%i] %s\n", socket->id, received_data);
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
