#ifndef TRAME_UTILS_H
#define TRAME_UTILS_H

#include "trame.h"
#include <stdio.h>
#include <strings.h>
#include <linux/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

int connectTo(char* name, char* ip);
int sendTrame(Trame* trame, int socketDescriptor);
Trame* receiveTrame(int socketDescriptor);
int checkConnection(int socketDescriptor);

#endif
