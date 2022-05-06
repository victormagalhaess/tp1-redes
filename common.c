#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
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