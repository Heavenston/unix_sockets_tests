#include <sys/socket.h>
#include <stdlib.h>

void send_sized(int socket, char* buffer, int length) {
    send(socket, &length, sizeof(int), 0);
    send(socket, buffer, length, 0);
}

int recv_sized(int socket, char** buffer) {
    int length;
    recv(socket, &length, sizeof(int), 0);
    *buffer = (char*)malloc(length);
    recv(socket, *buffer, length, 0);
    return length;
}
