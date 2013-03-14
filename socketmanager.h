#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <string>

#include "common.h"
#include "trame.h"

using namespace std;

/*! \brief A class to handle basic operation on a socket.

    The basic use of this class is to first call connectTo or startListening
    function and then send/receive trames. A SocketManager object is able to
    manage only one socket. The related socket is close when the destructor is
    called.
*/
class SocketManager {
public:
    SocketManager(int socketDescriptor_ = -1);
    ~SocketManager();
    void connectTo(string name, string ip);
    void startListening(string name);
    void sendTrame(Trame* trame);
    Trame* receiveTrame() const;
    bool checkConnexion(string from);
    int getSocketDescriptor() const;

private:
    int socketDescriptor_;
};

#endif // SOCKETMANAGER_H
