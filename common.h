#ifndef COMMON_NET_TP1
#define COMMON_NET_TP1

struct Equipment
{
    int Id;
    int Sensors[4];
};

int getDomainByIPVersion(char *version);
void dieWithMessage(char *message);
void validateInputArgs(int argc);
void validateCommunication(int valread);
int getPort(char *port);

#endif