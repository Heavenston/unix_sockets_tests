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

#include "utils.h"
#include "client_server.h"

typedef struct {
    int fd;
    int id;
} Socket;

void server_main() {
    int result;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    unlink(SERVER_PATH);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        error("Could not bind socket");

    result = listen(fd, 1);
    if (result == -1)
        error("Could not listen for sockets");

    //
    // WAITING FOR CONNECTIONS
    // 
    printf("Waiting for connections...\n");

    Socket *socket_array[100];
    memset(socket_array, 0, 100 * sizeof(Socket*));

    int socket_id_counter = 0;
    while (1) {
        // TRY ACCEPTING A SOCKET
        {
            struct sockaddr_un connected_addr;
            socklen_t connected_addr_length = sizeof(struct sockaddr_un);
            int accepted_fd = accept(fd, (struct sockaddr*)&connected_addr, &connected_addr_length);
            if (accepted_fd != -1) {
                fcntl(accepted_fd, F_SETFL, O_NONBLOCK);
                
                printf("Socket connection received, assigning id %d\n", socket_id_counter);
                int index = 0;
                while (index < 100 && socket_array[index] != NULL)
                    index++;
                if (index >= 100) {
                    printf("Could not accept connect, buffer full");
                    shutdown(accepted_fd, SHUT_RDWR);
                }
                else {
                    socket_array[index] = malloc(sizeof(Socket));
                    socket_array[index]->fd = accepted_fd;
                    socket_array[index]->id = socket_id_counter++;
                }
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK)
                error("Error while accepting a socket");
        }

        // TRY RECEIVING DATA
        {
            for (int i = 0; i < 100; i++) {
                if (socket_array[i] == NULL)
                    continue;
                Socket *socket = socket_array[i];

                char *received_data;
                int received_bytes = recv_packet(socket->fd, (void**)&received_data);
                if (received_bytes > 0) {
                    // PROBLEM: If received data isn't a null-terminated string
                    printf("[%i] %s\n", socket->id, received_data);
                    free(received_data);
                }
                else if ((errno != EAGAIN && errno != EWOULDBLOCK) || (received_bytes == 0)) {
                    if (received_bytes == 0) {
                        printf("Client #%i disconnected\n", socket->id);
                    }
                    else {
                        printf("Error while reading data from #%i :", socket->id);
                        print_errno();
                    }
                    free(socket);
                    socket_array[i] = NULL;
                }
            }
        }

        sched_yield();
        usleep(1000);
    }

    // CLEANING UP
    result = shutdown(fd, SHUT_RDWR);
    if (result == -1)
        error("Could not shutdown socket");
}
