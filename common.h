#ifndef COMMON_NET_TP1
#define COMMON_NET_TP1
#define BUFFER_SIZE_BYTES 500

int getDomainByIPVersion(char *version);
void dieWithMessage(char *message);
void validateInputArgs(int argc);
void validateCommunication(int valread);
int getPort(char *port);

#endif