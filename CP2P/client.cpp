#pragma GCC diagnostic ignored "-fpermissive"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <string>
#include <iostream>

#include "consolegui.h"
#include "consoleguicontroller.h"
#include "socketmanager.h"
#include "clientreceivehandler.h"
#include "trame.h"

using namespace std;

// Global variables for debugging
//string name = "bob"; string ip = "127.0.0.1";
string name = "alice"; string ip = "127.0.0.1";
string host = "127.0.0.1";

void* singleHandler(void* sock) {
    int* tmp = (int*)sock;
    int socketDescriptorPouet = *tmp;
    ClientReceiveHandler* receiveHandler = new ClientReceiveHandler(name, socketDescriptorPouet);
    receiveHandler->launchReception();
}

void* handleTrame() {
    SocketManager* receiveSocketManager = new SocketManager();
    receiveSocketManager->startListening(name);
    pthread_t handleTrame_th;
    sockaddr_in adresse_client_courant;
    int longueur_adresse_courante;
    int newSocketDescriptor;
    for(;;) {
        longueur_adresse_courante = sizeof(adresse_client_courant);
        if((newSocketDescriptor = accept(receiveSocketManager->getSocketDescriptor(),(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))<0) {
            cout << "unable to accept a new client " << endl;
            return 1;
        }
        if(pthread_create(&handleTrame_th,NULL,singleHandler,(void*)&newSocketDescriptor)) {
            cout << "unable to create a new thread" << endl;
            return 1;
        }
    }
    return;
}

int main(int argc, char** argv) {
    cout << "Welcome to CP2P file system !" << endl;
    // DEBUG
    cout << "Hello " << name << endl;
    SocketManager* mainSocketManager = new SocketManager();
    mainSocketManager->connectTo("big-daddy",host);

    // Create the connexion trame
    string connexionMsg = name;
    connexionMsg += ':';
    connexionMsg += ip;
    Trame* connexionTrame = new Trame(name,CON_SERV,connexionMsg.size(),1,1,connexionMsg);
    sleep(1);
    mainSocketManager->sendTrame(connexionTrame);

    // Receive the server response
    Trame* ackTrame = mainSocketManager->receiveTrame();
    if(ackTrame->getType() == ACK_CON) {
        cout  << "You are now connected to CP2P service." << endl;
    }

	// Launch the thread handeling incoming messages
	pthread_t handleMessage;
	if(pthread_create(&handleMessage, NULL,handleTrame,NULL)) {
		cout << "Cannot launch the listening local server." << endl;
        return -1;
	}

    ConsoleGUIController* controller = new ConsoleGUIController(name,"big-daddy",host);
    ConsoleGUI* consoleGUI = new ConsoleGUI(controller);
    consoleGUI->launchGUI();
}
