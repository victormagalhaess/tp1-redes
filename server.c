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

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        return -1;
    }
    int server_fd, new_socket;

    int opt = 1;

    struct sockaddr_in address;
    struct sockaddr_in6 addressv6;

    char buffer[1024] = {0};
    char *hello = "Hello from server";
    int domain = getDomainByIPVersion(strdup(argv[1]));
    int port = atoi(argv[2]);

    // Creating socket file descriptor
    if ((server_fd = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
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

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, conAddress, conSize) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_PENDING) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, conAddress, (socklen_t *)&conSize)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    int valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    return 0;
}
