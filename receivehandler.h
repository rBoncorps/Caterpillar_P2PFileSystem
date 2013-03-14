#ifndef RECEIVEHANDLER_H
#define RECEIVEHANDLER_H

#include "socketmanager.h"
#include "common.h"
#include <string>

using namespace std;

/*! \brief A class to handle a receive trame loop.

    The class must be initialized with a socketDescriptor created
    by an accept call. Otherwise the loop won't
    be able to extract received trames.
*/
class ReceiveHandler {

public:
    ReceiveHandler(string name,int socketDescriptor);
    ~ReceiveHandler();
    virtual void launchReception() = 0;

protected:
    string name_;
    SocketManager socketManager_;


};

#endif // RECEIVEHANDLER_H
