
#include "common.h"
#include "server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

/* All the socket creation logic is segregated in the following function.
It receives the ipVersion (expected to be v4 or v6) and the port, builds the socket
and returns the int that represents the socket connection */

int buildServerSocket(char *ipVersion, char *portString)
{

    int serverFd, sock;
    int opt = 1;

    struct sockaddr_in address;
    struct sockaddr_in6 addressv6;

    int domain = getDomainByIPVersion(ipVersion);
    int port = getPort(portString);

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

/* The functions in this section are pretty much auxiliar. They are meant to
do auxiliar tasks as: end the socket connection and kill the server, copy strings,
format the commands received by the client, format the output strings to be send
to the client, etc */

void closeSocketAndDie(int socket)
{
    close(socket);
    dieWithMessage("Server closing...");
}

void copyCommand(char *fullCommand, char *mainCommand, char *buffer)
{
    strcpy(mainCommand, buffer);
    strcpy(fullCommand, buffer);
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

int parseInput(char *fullCommand, int *sensorsToProcess, int *allValidSensors)
{
    // this double call of strtok was made to remove the "COMMAND sensor" from the command to be executed
    // the command "read" is a bit different and does not count with the "sensor" keyword, so we do not need to
    // remove the second token of the command
    char *partOfCommand = strtok(fullCommand, " ");
    if (strcmp(partOfCommand, "read") != 0)
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
            sensorsToProcess[sensor] = 1;
        }
        else
        {
            *allValidSensors = 0;
        }
    }

    return atoi(partOfCommand) - 1;
}

void formatElement(int element, char *elementFeedback)
{
    char elementId[4];
    sprintf(elementId, "0%d ", element + 1);
    strcat(elementFeedback, elementId);
}

void formatOutput(char *sensorOperationFeedback, char *installationFeedback, char *limitFeedback, char *alreadyAppliedFeedback, int newState, int equipment)
{
    char *status = newState ? "added " : "removed ";
    char *alreadyAppliedStatus = newState ? "already exists in " : "does not exists in ";
    char equipmentId[4] = "";
    char fullInstalationFeedback[100] = "";
    char fullAlreadyAppliedFeedback[100] = "";
    formatElement(equipment, equipmentId);

    if (strcmp(installationFeedback, "") != 0)
    {
        strcpy(fullInstalationFeedback, "sensor ");
        strcat(installationFeedback, status);
        strcat(fullInstalationFeedback, installationFeedback);
    }

    if (strcmp(alreadyAppliedFeedback, "") != 0)
    {
        strcpy(fullAlreadyAppliedFeedback, "sensor ");
        strcat(alreadyAppliedFeedback, alreadyAppliedStatus);
        strcat(alreadyAppliedFeedback, equipmentId);
        strcat(fullAlreadyAppliedFeedback, alreadyAppliedFeedback);
    }

    sprintf(sensorOperationFeedback, "%s%s%s", fullInstalationFeedback, limitFeedback, fullAlreadyAppliedFeedback);
}

/*This section is meant to operate on the commands. It covers the commands that operates over the sensors and
equipments and a few auxiliar functions. They go as Follow:
ChangeSensors: A abstraction of the operation of "Changing a sensor state" it is actually not a command, but is used by
the add and remove commands.
AddSensors: Calls the ChangeSensors function with the instalation objective, takes in account the limit of sensors and increases it
RemoveSensors: Calls the ChangeSensors function with the removal objective, does not really care about the limit, but decreases it
ListSensors: List the sensors installed in the required equipment
ReadSensors: Read the required sensors in the required equipment, builds a fake reading using 3 random integers to build a fake decimal*/

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

    if (strcmp(sensorsInEquipment, "") == 0)
    {
        strcpy(sensorsInEquipment, "none");
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

int changeSensors(struct Equipment *equipments, char *fullCommand, int newState, char *sensorOperationFeedback, int installedSensors)
{
    char installationFeedback[50] = "";
    char limitFeedback[15] = "";
    char alreadyAppliedFeedback[50] = "";

    int allValidSensors = 1;
    int sensorsToChange[4] = {0};
    int equipmentToChange = parseInput(fullCommand, sensorsToChange, &allValidSensors);
    int valueToChange = newState ? 1 : -1;

    if (equipmentToChange >= 0 && equipmentToChange <= 3)
    {
        if (allValidSensors)
        {
            for (int i = 0; i < 4; i++)
            {
                if (sensorsToChange[i])
                {

                    if (installedSensors < MAX_SENSORS || !newState)
                    {
                        installedSensors += valueToChange;
                        changeSensor(equipments, equipmentToChange, newState, i, installationFeedback, alreadyAppliedFeedback);
                    }
                    else if (newState)
                    {
                        strcpy(limitFeedback, "limit exceeded");
                    }
                }
            }
            formatOutput(sensorOperationFeedback, installationFeedback, limitFeedback, alreadyAppliedFeedback, newState, equipmentToChange);
        }
        else
        {
            strcpy(sensorOperationFeedback, "invalid sensor");
        }
    }
    else
    {
        strcpy(sensorOperationFeedback, "invalid equipment");
    }
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

void buildSensorReading(char *sensorReadings)
{
    char reading[6] = "";
    sprintf(reading, "%d.%d%d ", rand() % 9, rand() % 9, rand() % 9); // nice little trick to fake a random decimal number
    strcat(sensorReadings, reading);
}

void readFromSensors(struct Equipment *equipments, char *fullCommand, char *sensorReadFeedback)
{
    int allValidSensors = 1;
    int sensorsToRead[4] = {0};
    int equipmentToRead = parseInput(fullCommand, sensorsToRead, &allValidSensors);
    char invalidSensors[50] = "";
    char invalidSensorsFeedback[100] = "";
    if (equipmentToRead >= 0 && equipmentToRead <= 3)
    {
        if (allValidSensors)
        {
            for (int i = 0; i < 4; i++)
            {
                if (sensorsToRead[i] && equipments[equipmentToRead].Sensors[i])
                    buildSensorReading(sensorReadFeedback);
                else if (sensorsToRead[i])
                    formatElement(i, invalidSensors);
            }
            if (strcmp(invalidSensors, "") != 0)
            {
                if (strcmp(sensorReadFeedback, "") != 0)
                {
                    strcat(invalidSensorsFeedback, "and ");
                }
                strcat(invalidSensorsFeedback, invalidSensors);
                strcat(invalidSensorsFeedback, "not installed");
                strcat(sensorReadFeedback, invalidSensorsFeedback);
            }
        }
        else
        {
            strcpy(sensorReadFeedback, "invalid sensor");
        }
    }
    else
    {
        strcpy(sensorReadFeedback, "invalid equipment");
    }
}

/*The last section covers the main function that is responsible to start the server, initialize variables, and start the
reading loop, where the server enters in a state that it waits the client to send a message and execute the valid
commands that it receives.*/

int main(int argc, char const *argv[])
{
    char *ipVersion = strdup(argv[1]);
    char *port = strdup(argv[2]);
    validateInputArgs(argc);
    srand(time(0));
    int sock = buildServerSocket(ipVersion, port);
    int connectionIsAlive = 1;
    int installedSensors = 0;

    char buffer[BUFFER_SIZE_BYTES] = "";
    char message[BUFFER_SIZE_BYTES] = "";
    char mainCommand[BUFFER_SIZE_BYTES] = "";
    char fullCommand[BUFFER_SIZE_BYTES] = "";

    struct Equipment equipments[4] = {
        [0] = {.Sensors = {0}},
        [1] = {.Sensors = {0}},
        [2] = {.Sensors = {0}},
        [3] = {.Sensors = {0}},
    };

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));
        memset(mainCommand, 0, sizeof(mainCommand));

        int totalBytesRead = read(sock, buffer, BUFFER_SIZE_BYTES);
        validateCommunication(totalBytesRead);
        printf("%s\n", buffer);
        copyCommand(fullCommand, mainCommand, buffer);
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
            readFromSensors(equipments, fullCommand, message);
            break;
        case KILL:
            closeSocketAndDie(sock);
            break;
        default:
            close(sock);
            connectionIsAlive = 0;
            break;
        }
        if (connectionIsAlive)
        {
            int valsent = send(sock, message, strlen(message), 0);
            validateCommunication(valsent);
        }
        else
        {
            sock = buildServerSocket(ipVersion, port);
            connectionIsAlive = 1;
        }
    }
    return 0;
}
