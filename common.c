#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define V4 "v4"
#define V6 "v6"

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
    return 0;
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
    if (status < 0)
    {
        dieWithMessage("Error during communication");
    }
}

int getPort(char *port)
{
    return atoi(port);
}