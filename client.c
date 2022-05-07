// Client side C/C++ program to demonstrate Socket
// programming
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    int sock = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in6 serv_addrv6;

    char *hello = "Hello from client";
    char buffer[1024] = {0};

    int domain = getDomainByIPVersion(strdup(argv[1]));
    int port = atoi(argv[2]);

    if ((sock = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = domain;
    serv_addr.sin_port = htons(port);

    serv_addrv6.sin6_family = domain;
    serv_addrv6.sin6_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(domain, "127.0.0.1", &serv_addr.sin_addr) <= 0 && domain == AF_INET)
    {
        printf(
            "\nInvalid IPV4 address/ Address not supported \n");
        return -1;
    }

    if (inet_pton(domain, "::1", &serv_addrv6.sin6_addr) <= 0 && domain == AF_INET6)
    {
        printf(
            "\nInvalid IPV6 address/ Address not supported \n");
        return -1;
    }

    struct sockaddr *conAddress = (struct sockaddr *)&serv_addr;
    int conSize = sizeof(serv_addr);
    if (domain == AF_INET6)
    {
        conAddress = (struct sockaddr *)&serv_addrv6;
        conSize = sizeof(serv_addrv6);
    }

    if (connect(sock, conAddress, conSize) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    int valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    return 0;
}
