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

void client_main() {
    int result = 150;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = create_address(SERVER_PATH);
    result = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
        error("Could not connect to server");
    printf("Connected to server!\n");
    printf("Type message to end\n");
    printf("Type 'exit' to quit the program\n");
    
    send_packet(fd, "HELLLLOOOO", 11);

    char buffer[150];
    while (1) {
        printf("> ");
        fflush(stdout);
        scanf("%[^\n]", buffer);
        fgetc(stdin);

        if (strcmp("exit", buffer) == 0)
            break;

        int length = strlen(buffer);
        send_packet(fd, buffer, length+1);
    }
    printf("Shutting down...\n");

    result = shutdown(fd, SHUT_RDWR);
    if (result == -1)
        error("Could not shutdown socket");
}
