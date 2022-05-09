// Client side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUFFER_SIZE_BYTES 500

int main(int argc, char const *argv[])
{
    validateInputArgs(argc);

    int sock = 0;
    int domain;

    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv_addrv6;

    char *hello = "Hello from client";
    char buffer[BUFFER_SIZE_BYTES] = {0};

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) > 0)
    {
        domain = AF_INET;
    }

    if (inet_pton(AF_INET6, argv[1], &serv_addrv6.sin6_addr) > 0)
    {
        domain = AF_INET6;
    }

    int port = atoi(argv[2]);

    if ((sock = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dieWithMessage("Socket creation error\n");
    }

    serv_addr.sin_family = domain;
    serv_addr.sin_port = htons(port);

    serv_addrv6.sin6_family = domain;
    serv_addrv6.sin6_port = htons(port);

    struct sockaddr *conAddress = (struct sockaddr *)&serv_addr;
    int conSize = sizeof(serv_addr);
    if (domain == AF_INET6)
    {
        conAddress = (struct sockaddr *)&serv_addrv6;
        conSize = sizeof(serv_addrv6);
    }

    if (connect(sock, conAddress, conSize) < 0)
    {
        dieWithMessage("Connection Failed \n");
    }

    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    int valread = read(sock, buffer, BUFFER_SIZE_BYTES);
    printf("%s\n", buffer);
    return 0;
}
