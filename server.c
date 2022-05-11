
#include "common.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_PENDING 5
#define BUFFER_SIZE_BYTES 500
#define LIST 0
#define ADD 1
#define REMOVE 2
#define READ 3
#define INVALID -1

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

int parseCommand(char *fullCommand)
{
    char *command = strtok(fullCommand, " ");
    if (strcmp(command, "list") == 0)
    {
        return LIST;
    }
    else if (strcmp(command, "add") == 0)
    {
        return ADD;
    }
    else if (strcmp(command, "remove") == 0)
    {
        return REMOVE;
    }
    else if (strcmp(command, "read") == 0)
    {
        return READ;
    }

    return INVALID;
}

void listSensors(int sensors[], char *sensorsInEquipment)
{
    for (int i = 0; i < 4; i++)
    {
        if (sensors[i])
        {
            char sensorID[4];
            sprintf(sensorID, "0%d ", i + 1);
            printf("%s\n", sensorID);
            strcat(sensorsInEquipment, sensorID);
        }
    }
}

void addSensors() {}

void removeSensors() {}

void readFromSensors() {}

int main(int argc, char const *argv[])
{
    int sock = buildServerSocket(argc, argv);
    char buffer[BUFFER_SIZE_BYTES] = {0};
    char message[BUFFER_SIZE_BYTES] = "";
    char auxBuffer[BUFFER_SIZE_BYTES] = "";

    struct Equipment equipments[4] = {
        [0] = {.Id = 0, .Sensors = {0}},
        [1] = {.Id = 1, .Sensors = {0}},
        [2] = {.Id = 2, .Sensors = {0}},
        [3] = {.Id = 3, .Sensors = {0}},
    };

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
        memset(auxBuffer, 0, sizeof(auxBuffer));

        int valread = read(sock, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(valread);
        printf("%s\n", buffer);
        strcpy(auxBuffer, buffer);
        int commandType = parseCommand(auxBuffer);

        switch (commandType)
        {
        case LIST:
            listSensors(equipments[0].Sensors, message);
            break;
        case ADD:
            addSensors();
            break;
        case REMOVE:
            removeSensors();
            break;
        case READ:
            readFromSensors();
            break;
        default:
            break;
        }

        int valsent = send(sock, message, strlen(message), 0);
        validateCommunication(valsent);
        printf("Sent %d bytes successfuly\n", valsent);
    }
    return 0;
}
