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

int main(int argc, char **argv) {
    if (argc != 2 || ((strcmp(argv[1], "server") != 0) && (strcmp(argv[1], "client") != 0))) {
        printf("Please specify server or client\n");
        return 1;
    }
    int is_server = strcmp(argv[1], "server") == 0;

    if (is_server)
        server_main();
    else
        client_main();

    return 0;
}
