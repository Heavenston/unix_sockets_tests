#pragma once

#include <sys/un.h>

void print_errno();
void report_error(char *message);

struct sockaddr_un create_address(char *path);
