#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define V4 "v4"
#define V6 "v6"
#define MIN_PORT_VALUE 1025
#define MAX_PORT_VALUE 65535

int getDomainByIPVersion(char *version)
{
    if (strcmp(version, V4) == 0)
    {
        return AF_INET;
    }
    else if (strcmp(version, V6) == 0)
    {
        return AF_INET6;
    }
    dieWithMessage("Invalid ip version. You must specify a valid ip version (v4 or v6)");
    return -1; // never reached
}

void dieWithMessage(char *message)
{
    printf("%s\n", message);
    exit(-1);
}

void validateInputArgs(int argc)
{
    if (argc != 3)
    {
        dieWithMessage("Invalid args number");
    }
}

void validateCommunication(int status)
{
    if (status < 1)
    {
        dieWithMessage("Error during communication");
    }
}

int getPort(char *portString)
{
    int port = atoi(portString);
    if (port >= MIN_PORT_VALUE && port <= MAX_PORT_VALUE)
    {
        return port;
    }
    dieWithMessage("Port in invalid range. You must use a non-root valid Unix port betwheen 1025 and 65535");
    return -1; // never reached
}