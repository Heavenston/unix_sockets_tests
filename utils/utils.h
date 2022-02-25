#include <sys/un.h>

void print_errno();
void error(char *message);

struct sockaddr_un create_address(char *path);
