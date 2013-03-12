#include "serverreceivehandler.h"
#include "trame.h"

#include <stdexcept>

// DEBUG
#include <iostream>

ServerReceiveHandler::ServerReceiveHandler(string name, int socketDescriptor, MapIp* mapIp) : ReceiveHandler(name,socketDescriptor) {
    mapIp_ = mapIp;
}

ServerReceiveHandler::~ServerReceiveHandler() {

}

void ServerReceiveHandler::launchReception() {
    bool stopReception = false;
    Trame* receivedTrame;
    while(!stopReception) {
        try {
        receivedTrame = socketManager_.receiveTrame();
        }catch(runtime_error& e) {
            cout << "client ends up (in the naaaaaaaaaaaaaaaaaaaaaaaaaaavy" << endl;
            return;
        }
        if(receivedTrame->getType() == CON_SERV) {
            cout << "[ReceiveHandler::launchReception] Received a CON_SERV trame." << endl;
            cout << "[ReceiveHandler::launchReception] Datas : " << receivedTrame->getData() << endl;
            string name;
            string ip;
            try {
                receivedTrame->extractNameIP(name,ip);
            }catch(runtime_error& e) {
                throw runtime_error(e.what());
            }
            if(name.empty() || ip.empty()) {
                cout << "[ReceiveHandler::launchReception] Error during name or ip extraction." << endl;
                throw runtime_error("Error during name or ip extraction.");
            }
            mapIp_->insert(std::pair<string,string>(name,ip));
            cout << "[ReceiveHandler::launchReception] mapIp size : " << mapIp_->size() << endl;
            cout << "[ReceiveHandler::launchReception] Server map : " << endl;
            MapIp::iterator it;
            for(it = mapIp_->begin(); it!=mapIp_->end(); ++it) {
                cout << it->first << " : " << it->second << endl;
            }
            Trame* ackTrame = new Trame("big-daddy",ACK_CON);
            socketManager_.sendTrame(ackTrame);
        }
        if(receivedTrame->getType() == DEM_AMI) {
            cout << "[ReceiveHandler::launchReception] Received a DEM_AMI trame." << endl;
            cout << "[ReceiveHandler::launchReception] Datas : " << receivedTrame->getData() << endl;
            string name = receivedTrame->getData();
            if(name.empty()) {
                cout << "[ReceiveHandler::launchReception] The DEM_AMI trame does not contain a name to add as a friend." << endl;
                throw runtime_error("The DEM_AMI trame does not contain a name to add as friend.");
            }
            MapIp::iterator it = mapIp_->find(name);
            if(it == mapIp_->end()) {
                string errorMessage = "The server doesn't know the given name.";
                cout << "[ReceiveHandler::launchReception] " << errorMessage << endl;
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                socketManager_.sendTrame(errorTrame);
                continue;
            }
            string ip = it->second;
            SocketManager* otherClientSocketManager = new SocketManager();
            bool reachable = true;
            try {
                otherClientSocketManager->connectTo(name,ip);
            }catch(runtime_error& e) {
                reachable = false;
                string errorMessage = "The client is not connected.pouetpouet";
                cout << "[ReceiveHandler::launchReception] " << errorMessage << endl;
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                socketManager_.sendTrame(errorTrame);
            }
            bool connected = false;
            if(reachable) {
                connected = otherClientSocketManager->checkConnexion("big-daddy");
            cout << "petitpatapon" << endl;
            }
            if(connected) {
                close(otherClientSocketManager->getSocketDescriptor());
                cout << "YMCA" << endl;
                string response;
                response += name += ':';
                response += ip;
                Trame* ackTrame = new Trame("big-daddy",ACK,response.size(),1,1,response);
                socketManager_.sendTrame(ackTrame);
            }
            else {
                cout << "pifpafpouf" << endl;
                string errorMessage = "The client is not connected.";
                cout << "[ReceiveHandler::launchReception] " << errorMessage << endl;
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                socketManager_.sendTrame(errorTrame);
            }
        }
    }
}
