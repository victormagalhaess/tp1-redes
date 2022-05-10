
#include "common.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_PENDING 5
#define BUFFER_SIZE_BYTES 500

int buildServerSocket(int argc, char const *argv[])
{
    validateInputArgs(argc);

    int serverFd, sock;
    int opt = 1;

    struct sockaddr_in address;
    struct sockaddr_in6 addressv6;

    int domain = getDomainByIPVersion(strdup(argv[1]));
    int port = getPort(strdup(argv[2]));

    if ((serverFd = socket(domain, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        dieWithMessage("socket failed");
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
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

    if (bind(serverFd, conAddress, conSize) < 0)
    {
        dieWithMessage("bind failed");
    }

    if (listen(serverFd, MAX_PENDING) < 0)
    {
        dieWithMessage("listen failed");
    }
    if ((sock = accept(serverFd, conAddress, (socklen_t *)&conSize)) < 0)
    {
        dieWithMessage("accept failed");
    }

    return sock;
}

int main(int argc, char const *argv[])
{
    int sock = buildServerSocket(argc, argv);
    char buffer[BUFFER_SIZE_BYTES] = {0};

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(valread);
        printf("%s\n", buffer);
        int valsent = send(sock, buffer, strlen(buffer), 0);
        validateCommunication(valsent);
        printf("Sent %d bytes successfuly\n", valsent);
    }

    return 0;
}
