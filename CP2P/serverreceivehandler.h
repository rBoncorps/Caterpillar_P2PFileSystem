#ifndef SERVERRECEIVEHANDLER_H
#define SERVERRECEIVEHANDLER_H

#include "receivehandler.h"
#include "common.h"
#include <string>

using namespace std;

class ServerReceiveHandler : public ReceiveHandler {

public:
    ServerReceiveHandler(string name, int socketDescriptor, MapIp* mapIp);
    ~ServerReceiveHandler();

    void launchReception();

private:
    MapIp* mapIp_;
};

#endif // SERVERRECEIVEHANDLER_H
