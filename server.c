// Server side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_PENDING 5
#define BUFFER_SIZE_BYTES 500

int main(int argc, char const *argv[])
{
    validateInputArgs(argc);

    int server_fd, new_socket;
    int opt = 1;

    struct sockaddr_in address;
    struct sockaddr_in6 addressv6;

    char buffer[BUFFER_SIZE_BYTES] = {0};
    int domain = getDomainByIPVersion(strdup(argv[1]));
    int port = getPort(strdup(argv[2]));

    if ((server_fd = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dieWithMessage("socket failed");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        dieWithMessage("setsockopt failed");
    }

    address.sin_family = domain;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    addressv6.sin6_family = domain;
    addressv6.sin6_addr = in6addr_any;
    addressv6.sin6_port = htons(port);

    struct sockaddr *conAddress = (struct sockaddr *)&address;
    int conSize = sizeof(address);
    if (domain == AF_INET6)
    {
        conAddress = (struct sockaddr *)&addressv6;
        conSize = sizeof(addressv6);
    }

    if (bind(server_fd, conAddress, conSize) < 0)
    {
        dieWithMessage("bind failed");
    }

    if (listen(server_fd, MAX_PENDING) < 0)
    {
        dieWithMessage("listen failed");
    }
    if ((new_socket = accept(server_fd, conAddress, (socklen_t *)&conSize)) < 0)
    {
        dieWithMessage("accept failed");
    }
    for (;;)
    {
        int valread = read(new_socket, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(valread);
        printf("%s\n", buffer);
        //        send(new_socket, hello, strlen(hello), 0);
    }

    return 0;
}
