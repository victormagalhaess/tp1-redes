#include <sys/socket.h>
#include <string.h>
#include "common.h"

#define V4 "v4"
#define V6 "v6"

int getDomainByIPVersion(char *version)
{

    if (strcmp(version, V4))
    {
        return AF_INET;
    }
    else if (strcmp(version, V6))
    {
        return AF_INET6;
    }
    return 0;
}