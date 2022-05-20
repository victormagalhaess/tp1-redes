#ifndef SERVER_NET_TP1
#define SERVER_NET_TP1

#define MAX_PENDING 5
#define MAX_SENSORS 15

enum Commands
{
    LIST = 0,
    ADD,
    REMOVE,
    READ,
    KILL,
    INVALID = -1,
};

struct Equipment
{
    int Sensors[4];
};

#endif