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

#define FIRST_PATH "/tmp/temporary_dgram_1.socket"
#define SECOND_PATH "/tmp/temporary_dgram_2.socket"

int main() {
    int result;
    unlink(FIRST_PATH);
    unlink(SECOND_PATH);

    int socket_one = socket(AF_UNIX, SOCK_DGRAM, 0);
    int socket_two = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un addr_one = create_address(FIRST_PATH);
    struct sockaddr_un addr_two = create_address(SECOND_PATH);

    result = bind(socket_two, (struct sockaddr*)&addr_two, sizeof(struct sockaddr_un));
    if (result == -1)
        error("Could not bind two");

    printf("Sending...\n");
    sendto(socket_one, "Hello", 6, 0, (struct sockaddr*)&addr_two, sizeof(struct sockaddr_un));
    printf("Receiving...\n");

    char buffer[150];
    int size = recv(socket_two, &buffer, 150, 0);
    printf("Received: %s\n", buffer);

    return 0;
}
