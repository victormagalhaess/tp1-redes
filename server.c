
#include "common.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_PENDING 5
#define MAX_SENSORS 15
#define BUFFER_SIZE_BYTES 500

enum Commands
{
    LIST = 0,
    ADD,
    REMOVE,
    READ,
    KILL,
    INVALID = -1,
};

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
    else if (strcmp(command, "kill") == 0)
    {
        return KILL;
    }

    return INVALID;
}

void formatElement(int element, char *elementFeedback)
{
    char elementId[4];
    sprintf(elementId, "0%d ", element + 1);
    strcat(elementFeedback, elementId);
}

void listSensors(struct Equipment *equipments, char *fullCommand, char *sensorsInEquipment)
{
    // this call of strtok was made to remove the "COMMAND sensor" from the command to be executed
    char *partOfCommand = strtok(fullCommand, " ");
    for (int i = 0; i < 3; i++)
    {
        partOfCommand = strtok(NULL, " ");
    }

    int equipmentToCheck = atoi(partOfCommand) - 1;
    for (int i = 0; i < 4; i++)
    {
        if (equipments[equipmentToCheck].Sensors[i])
        {
            formatElement(i, sensorsInEquipment);
        }
    }
}

void changeSensor(struct Equipment *equipments, int equipmentToChange, int newState, int sensor, char *installationFeedback, char *alreadyAppliedFeedback)
{
    if (equipments[equipmentToChange].Sensors[sensor] != newState)
    {
        equipments[equipmentToChange].Sensors[sensor] = newState;
        formatElement(sensor, installationFeedback);
    }
    else
    {
        formatElement(sensor, alreadyAppliedFeedback);
    }
}

void formatOutput(char *sensorOperationFeedback, char *installationFeedback, char *limitFeedback, char *alreadyAppliedFeedback, int newState, int equipment)
{
    char *status = newState ? "added " : "deleted ";
    char *alreadyAppliedStatus = newState ? "already exists in " : "does not exists in ";
    char equipmentId[4] = "";
    formatElement(equipment, equipmentId);

    if (strcmp(installationFeedback, "") != 0)
    {
        strcat(installationFeedback, status);
    }

    if (strcmp(alreadyAppliedFeedback, "") != 0)
    {
        strcat(alreadyAppliedFeedback, alreadyAppliedStatus);
        strcat(alreadyAppliedFeedback, equipmentId);
    }

    sprintf(sensorOperationFeedback, "%s%s%s", installationFeedback, limitFeedback, alreadyAppliedFeedback);
}

int changeSensors(struct Equipment *equipments, char *fullCommand, int newState, char *sensorOperationFeedback, int installedSensors)
{
    int sensorsToChange[4] = {0};

    char installationFeedback[50] = "";
    char limitFeedback[15] = "";
    char alreadyAppliedFeedback[50] = "";

    // this double call of strtok was made to remove the "COMMAND sensor" from the command to be executed
    char *partOfCommand = strtok(fullCommand, " ");
    partOfCommand = strtok(NULL, " ");

    while (partOfCommand != NULL)
    {
        partOfCommand = strtok(NULL, " ");
        if (strcmp(partOfCommand, "in") == 0)
        {
            partOfCommand = strtok(NULL, " ");
            break;
        }
        int sensor = atoi(partOfCommand) - 1;
        if (sensor >= 0 && sensor <= 3)
        {
            sensorsToChange[sensor] = 1;
        }
        printf(" %s\n", partOfCommand);
    }

    int equipmentToChange = atoi(partOfCommand) - 1;

    if (equipmentToChange >= 0 && equipmentToChange <= 3)
    {
        for (int i = 0; i < 4; i++)
        {
            if (sensorsToChange[i])
            {
                if (newState)
                {
                    if (installedSensors < MAX_SENSORS)
                    {
                        installedSensors += 1;
                        changeSensor(equipments, equipmentToChange, newState, i, installationFeedback, alreadyAppliedFeedback);
                    }
                    else
                    {
                        strcpy(limitFeedback, "limit exceeded");
                    }
                }
                else
                {
                    installedSensors -= 1;
                    changeSensor(equipments, equipmentToChange, newState, i, installationFeedback, alreadyAppliedFeedback);
                }
            }
        }
    }
    formatOutput(sensorOperationFeedback, installationFeedback, limitFeedback, alreadyAppliedFeedback, newState, equipmentToChange);
    return installedSensors;
}

int addSensors(struct Equipment *equipments, char *fullCommand, char *sensorAddFeedback, int installedSensors)
{
    int sensorStateOn = 1;
    return changeSensors(equipments, fullCommand, sensorStateOn, sensorAddFeedback, installedSensors);
}

int removeSensors(struct Equipment *equipments, char *fullCommand, char *sensorRemoveFeedback, int installedSensors)
{
    int sensorStateOff = 0;
    return changeSensors(equipments, fullCommand, sensorStateOff, sensorRemoveFeedback, installedSensors);
}

void readFromSensors() {}

void die(int socket)
{
    close(socket);
    exit(-1);
}

int main(int argc, char const *argv[])
{
    int sock = buildServerSocket(argc, argv);
    int installedSensors = 0;
    char buffer[BUFFER_SIZE_BYTES] = {0};
    char message[BUFFER_SIZE_BYTES] = "";
    char mainCommand[BUFFER_SIZE_BYTES] = "";
    char fullCommand[BUFFER_SIZE_BYTES] = "";

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
        memset(mainCommand, 0, sizeof(mainCommand));

        int valread = read(sock, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(valread);
        printf("%s\n", buffer);
        strcpy(mainCommand, buffer);
        strcpy(fullCommand, buffer);
        int commandType = parseCommand(mainCommand);

        switch (commandType)
        {
        case LIST:
            listSensors(equipments, fullCommand, message);
            break;
        case ADD:
            installedSensors = addSensors(equipments, fullCommand, message, installedSensors);
            break;
        case REMOVE:
            installedSensors = removeSensors(equipments, fullCommand, message, installedSensors);
            break;
        case READ:
            readFromSensors();
            break;
        case KILL:
            die(sock);
            break;
        default:
            close(sock);
            break;
        }

        int valsent = send(sock, message, strlen(message), 0);
        validateCommunication(valsent);
    }
    return 0;
}
