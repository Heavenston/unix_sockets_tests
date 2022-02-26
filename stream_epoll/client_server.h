#pragma once

#define SERVER_PATH "/tmp/seqpkt_server.socket"
#define MAXIMUM_PACKET_SIZE 1000000

typedef struct {
    int id;
} handshake_packet_t;

void client_main();
void server_main();

int recv_packet(int fd, void **buffer);
int send_packet(int fd, const void *buffer, unsigned int size);
