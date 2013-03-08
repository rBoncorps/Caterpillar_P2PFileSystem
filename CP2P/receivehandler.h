#ifndef RECEIVEHANDLER_H
#define RECEIVEHANDLER_H

#include "socketmanager.h"
#include "common.h"

using namespace std;

/*! \brief A class to handle a receive trame loop.

    The class must be initialized with a socketDescriptor created
    by an accept call. Otherwise the loop won't
    be able to extract received trames.
*/
class ReceiveHandler {

public:
    ReceiveHandler(int socketDescriptor);
    ~ReceiveHandler();
    virtual void launchReception() = 0;

protected:
    void extractNameIP(string from, string& name, string& ip);
    SocketManager socketManager_;


};

#endif // RECEIVEHANDLER_H
