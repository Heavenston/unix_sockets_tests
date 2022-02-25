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
#include "sized_communication.h"

void receiver(int socket) {
    char* recvBuffer = NULL;
    int read = recv_sized(socket, &recvBuffer);

    printf("Read bytes: %i\n", read);
    printf("Read message: %s\n", recvBuffer);

    free(recvBuffer);
    for (int i = 0; i < 10; i++) {
        send(socket, &i, sizeof(int), 0);
        // 250ms sleep
        usleep(250000);
    }

    int i = -1;
    send(socket, &i, sizeof(int), 0);
}

int main() {
    int sockets[2] = {-1, -1};
    socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets);

    int buffer_size = -1;
    socklen_t buffer_length = sizeof(buffer_size);
    int s = getsockopt(sockets[0], AF_UNIX, SO_SNDBUF, &buffer_size, &buffer_length);
    printf("%d - %d\n", buffer_size, buffer_length);

    if (fork() == 0) {
        receiver(sockets[1]);
        return 0;
    }
    int socket = sockets[0];

    char message[150];
    printf("> ");
    scanf("%[^\n]", message);
    send_sized(socket, message, strlen(message) + 1);

    while (1) {
        int value = -1;
        recv(socket, &value, sizeof(int), 0);
        if (value == -1)
            break;
        printf("%i\n", value);
    }

	return 0;
}

