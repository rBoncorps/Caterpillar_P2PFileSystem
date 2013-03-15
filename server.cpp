#pragma GCC diagnostic ignored "-fpermissive"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <pthread.h>

#include "serverreceivehandler.h"
#include "socketmanager.h"
#include "trame.h"
#include "common.h"

// DEBUG
#include <iostream>

using namespace std;

string name = "big-daddy";

// Global variable definition
MapIp mapIP;

void* receptionTrame(void* sock) {
    int* tmp = (int*)sock;
    int socketDescriptor = *tmp;
    ServerReceiveHandler* receiveHandler = new ServerReceiveHandler(name, socketDescriptor,&mapIP);
    receiveHandler->launchReception();
}

int main() {
	cout << "CP2P server is running" << endl;
	cout << "segmentation fault (such a joke !)" << endl;
    pthread_t nouveau_client;

    int socket_descriptor,
         nouv_socket_descriptor,
         longueur_adresse_courante;
    SocketManager* mainSocketManager = new SocketManager();
    mainSocketManager->startListening("big-daddy");
    socket_descriptor = mainSocketManager->getSocketDescriptor();

    sockaddr_in adresse_client_courant;
    for(;;){
        longueur_adresse_courante = sizeof(adresse_client_courant);
        if((nouv_socket_descriptor= accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))<0){
            cout << "Unable to accept a new client connexion." << endl;
            return 1;
        }
        if(pthread_create(&nouveau_client, NULL, receptionTrame, (void*)&nouv_socket_descriptor)) {
            cout << "Unable to create an other thread to handle connexion." << endl;
            return 1;
        }
    }
}
