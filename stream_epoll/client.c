#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "utils.h"
#include "client_server.h"

void show_prompt() {
    printf("> ");
    fflush(stdout);
}

void client_main() {
    int result;
    
    // Creation and connection of socket
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        report_error("Could not connect to server");

    handshake_packet_t *handshake;
    result = recv_packet(fd, (void**)&handshake);
    if (result == -1)
        report_error("Could not received handshake");
    
    // EPOLL SETUP
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        report_error("Could not create the epoll fd");

    { // Adding stdin
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = STDIN_FILENO;
        result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
        if (result == -1)
            report_error("Could not add stdin file descriptor to epoll");
    }
    { // Adding socket
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = fd;
        result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
        if (result == -1)
            report_error("Could not add stdin file descriptor to epoll");
    }
    // Start of app

    printf("Connected to server with id #%i!\n", handshake->id);
    printf("Type messages to be sent to the server\n");
    printf("Type 'exit' to quit the program\n");
    show_prompt();

    while (1) {
        struct epoll_event event;
        result = epoll_wait(epoll_fd, &event, 1, -1);
        if (result == -1)
            report_error("Error while listening for events");

        if (event.data.fd == STDIN_FILENO) {
            char buffer[150];
            memset(buffer, '\0', 150);
            scanf("%[^\n]", buffer);
            fgetc(stdin);
            show_prompt();

            if (strcmp("exit", buffer) == 0)
                break;

            int length = strlen(buffer);
            result = send_packet(fd, buffer, length+1);
            if (result == -1)
                report_error("Could not send message");
        }
        else if (event.data.fd == fd) {
            printf("\x1b[\1G"); // Move cursor to start of line
            printf("\x1b[\2K"); // Clear whole line
            server_message_t *server_message;
            result = recv_packet(fd, (void**)&server_message);
            if (result == -1)
                report_error("Could not receive message");
            else if (result == 0) {
                printf("Server disconnected\n");
                break;
            }

            printf("[%s] %s\n", server_message->username, server_message->message);
            show_prompt();
        }
    }
    printf("Shutting down...\n");

    result = close(epoll_fd);
    if (result == -1)
        report_error("Could not close the epoll fd");

    result = shutdown(fd, SHUT_RDWR);
    if (result == -1)
        report_error("Could not shutdown socket");
}
