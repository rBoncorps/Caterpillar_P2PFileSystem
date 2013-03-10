#ifndef CLIENTRECEIVEHANDLER_H
#define CLIENTRECEIVEHANDLER_H

#include "receivehandler.h"
#include "common.h"
#include <string>

using namespace std;

class ClientReceiveHandler : public ReceiveHandler {

public:
    ClientReceiveHandler(string name, int socketDescriptor);
    ~ClientReceiveHandler();
    void launchReception();
};

#endif // CLIENTRECEIVEHANDLER_H
