#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "utils.h"

void print_errno() {
    printf("'%s'\n", strerror(errno));
}
void report_error(char *message) {
    printf("%s: ", message);
    print_errno();
    exit(1);
}

struct sockaddr_un create_address(char *path) {
    struct sockaddr_un address;

    int available_size = sizeof(struct sockaddr_un) - sizeof(sa_family_t);
    if (strlen(path) >= available_size) {
        printf("create_address error, address above maximum size (%li > %i)", strlen(path), available_size);
        exit(1);
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);

    return address;
}

