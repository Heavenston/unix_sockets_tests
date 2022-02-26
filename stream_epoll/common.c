#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "utils.h"
#include "client_server.h"

int recv_packet(int fd, void **buffer) {
    unsigned int packet_size;
    int result = recv(fd, &packet_size, sizeof(unsigned int), 0);
    if (result <= 0)
        return result;
    if (packet_size > MAXIMUM_PACKET_SIZE) {
        printf("Receving packet exceeds maximum size\n");
        exit(1);
    }

    *buffer = malloc(packet_size);
    result = recv(fd, *buffer, packet_size, MSG_WAITALL);
    return result;
}

int send_packet(int fd, const void *buffer, unsigned int size) {
    int result = send(fd, &size, sizeof(unsigned int), 0);
    if (result == -1)
        return result;
    result = send(fd, buffer, size, 0);
    return result;
}
