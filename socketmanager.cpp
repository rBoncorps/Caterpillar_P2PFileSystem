#include "socketmanager.h"
// DEBUG
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include "stdlib.h"
#include "unistd.h"

/*! \brief Creates a new SocketManager instance with the given
    socketDescriptor.
    \note If no value is given, the socketDescriptor is initialized
    with -1, wich corresponds to an invalid socket descriptor. In that
    case the client have to call connectTo/startListening to use the
    socket.
*/
SocketManager::SocketManager(int socketDescriptor) {
    socketDescriptor_ = socketDescriptor;
}

/*! \brief Destruct the current SocketManager.
*/
SocketManager::~SocketManager() {

}

/*! \brief Try to set up a connexion to the given ip.
    \warning This function must be called one time before trying
    to send a Trame, otherwise send function will send an exception.
    \exception A runtime_error is thrown if the connexion cannot be established
*/
void SocketManager::connectTo(string name, string ip) {
    hostent* clientHost;
    servent* clientService;
    sockaddr_in clientAdress;
    if((clientHost = gethostbyname(ip.c_str())) == NULL) {
        throw runtime_error("Can not find the client");
    }
    bcopy((char*)clientHost->h_addr, (char*)&clientAdress.sin_addr, clientHost->h_length);
    clientAdress.sin_family = AF_INET;
    // DEBUG : local dev
    if(name == "bob") {
        clientAdress.sin_port = htons(5002);
    }
    else if(name == "alice") {
        clientAdress.sin_port = htons(5003);
    }
    else {
        clientAdress.sin_port = htons(5001);
    }
    // /DEBUG : local dev
    socketDescriptor_ = socket(AF_INET,SOCK_STREAM,0);
    if(socketDescriptor_ < 0) {
        throw runtime_error("Can not create the connexion socket with the client.");
    }
    if((connect(socketDescriptor_, (sockaddr*)(&clientAdress), sizeof(clientAdress))) < 0) {
        throw runtime_error("Can not establish a connexion to the client.");
    }
}

/*! \brief Try to set up a listening connexion.
    \warning If the connectTo function has been called before, the previous connexion is lost.
    \exception A runtime_error is thrown if the connexion cannot be established
*/
void SocketManager::startListening(string name) {
    hostent* localHost;
    servent* localService;
    sockaddr_in localAdress;
    char machine[MAX_USERNAME_SIZE+1];
    gethostname(machine,MAX_USERNAME_SIZE);
    if((localHost=gethostbyname(machine))==NULL){
        throw runtime_error("Cannot launch the listening server");
    }
    bcopy((char*)localHost->h_addr,(char*)&localAdress.sin_addr,localHost->h_length);
    localAdress.sin_family = localHost->h_addrtype;
    localAdress.sin_addr.s_addr= INADDR_ANY;
    // DEBUG : local dev
    if(name == "bob") {
        localAdress.sin_port = htons(5002);
    }
    else if(name == "alice") {
        localAdress.sin_port = htons(5003);
    }
    else {
        localAdress.sin_port = htons(5001);
    }
    socketDescriptor_ = socket(AF_INET,SOCK_STREAM,0);
    if(socketDescriptor_ < 0) {
        throw runtime_error("Cannot create the listening socket");
    }
    if((bind(socketDescriptor_,(sockaddr*)(&localAdress),sizeof(localAdress)))<0){
        throw runtime_error("Cannot link to the listening socket.");
    }
    listen(socketDescriptor_,5);
}

/*! \brief Try to send a trame in the current socketDescriptor.
    \exception A runtime_error is thrown if an error occured during the send process.
*/
void SocketManager::sendTrame(Trame *trame) {
	usleep(50);
    SerializableTrame* serializable = trame->getSerializableTrame();
    if(write(socketDescriptor_,(char*)serializable,MAX_TRAME_SIZE) < 0) {
        throw runtime_error("Cannot send the trame in the socket.");
    }
}

/*! \brief Wait a Trame on the the socketDescriptor (setted in connectTo function)
    \return The received Trame or NULL if an error occured.
    \warning This function blocks the runtime if no Trame is sent. (It should
    be launch in a thread if a timeout is needed).
    \exception A runtime_error is thrown if an error occured during the socket reading.
*/
Trame* SocketManager::receiveTrame() const {
    char* buffer = (char*)malloc(sizeof(char)*MAX_TRAME_SIZE);
    int bufferSize = read(socketDescriptor_, buffer, MAX_TRAME_SIZE);
    if(bufferSize <= 0) {
        throw runtime_error("An error occured when during the read of the socket");
        return NULL;
    }
    SerializableTrame* serializable = (SerializableTrame*)buffer;
    string test;
    usleep(75);
    test.assign(serializable->data,serializable->taille);
    
    Trame* trame = new Trame(serializable->nameSrc,serializable->typeTrame,serializable->taille,serializable->numTrame,serializable->nbTrames,test);
    delete buffer;
    return trame;
}


/*! \brief Check if the socket's client is able to answer a request.
    \warning This function blocks the runtime if no Trame is received. (It should
    be launch in a thread if a timeout is needed).
*/
bool SocketManager::checkConnexion(string from) {
    Trame* checkTrame = new Trame(from,CHECK_CON);
    sendTrame(checkTrame);
    Trame* returnedTrame = receiveTrame();
    bool connected = (returnedTrame->getType() == ACK_CON);
    delete checkTrame;
    delete returnedTrame;
    return(connected);
}

/*! \brief SocketDescriptor getter.
*/
int SocketManager::getSocketDescriptor() const {
    return socketDescriptor_;
}
