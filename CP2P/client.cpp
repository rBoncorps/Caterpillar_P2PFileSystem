#pragma GCC diagnostic ignored "-fpermissive"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <string>
#include <iostream>

#include "socketmanager.h"
#include "trame.h"

using namespace std;

// Global variables for debugging
string name = "bob"; string ip = "127.0.0.1";
//string name = "alice"; string ip = "127.0.0.1";
string host = "127.0.0.1";

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
		return -1
	}

	ConsoleGUI* consoleGUI = new ConsoleGUI();
}
