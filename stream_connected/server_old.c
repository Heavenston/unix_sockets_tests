/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "utils.h"
#include "client_server.h"

void server_main() {
    int result;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    unlink(SERVER_PATH);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        error("Could not bind socket");

    result = listen(fd, 1);
    if (result == -1)
        error("Could not listen for sockets");

    //
    // WAITING FOR A CONNECTION
    // 
    printf("Waiting for a connection...\n");

    struct sockaddr_un connected_addr;
    socklen_t connected_addr_length = sizeof(struct sockaddr_un);
    int accepted_fd = accept(fd, (struct sockaddr*)&connected_addr, &connected_addr_length);
    if (accepted_fd == -1)
        error("Could not accept a socket");

    printf("Socket connection received\n");

    char *received_data;
    int received_bytes = recv_packet(accepted_fd, (void**)&received_data);
    while (received_bytes != 0) {
        // PROBLEM: If received data isn't a null-terminated string
        printf("< %s\n", received_data);

        free(received_data);
        received_bytes = recv_packet(accepted_fd, (void**)&received_data);
    }

    // END OF CONNECTION

    if (received_bytes == 0) {
        printf("Remote socket closed connection\n");
    }

    // CLEANING UP
    result = shutdown(fd, SHUT_RDWR);
    if (result == -1)
        error("Could not shutdown socket");
}
*/
