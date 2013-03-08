#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <vector>
#include <linux/types.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

typedef std::map<std::string,std::string> MapIp;

enum TrameType {CON_SERV,ACK_CON,DEM_AMI,DEM_CON_AMI,CMD_CON,CMD_HOME,CMD,LS_RET,CD_RET,CMD_END, DEM_FIC,ENV_FIC,ACK,FIN_CON_AMI,FIN_CON_SERV, ERROR, CHECK_CON};

#endif // COMMON_H
