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
            return;
        }
        if(receivedTrame->getType() == CON_SERV) {
            string name;
            string ip;
            try {
                receivedTrame->extractNameIP(name,ip);
            }catch(runtime_error& e) {
                throw runtime_error(e.what());
            }
            if(name.empty() || ip.empty()) {
                throw runtime_error("Error during name or ip extraction.");
            }
            mapIp_->insert(std::pair<string,string>(name,ip));
            MapIp::iterator it;
            cout << name << " is now connected to the service." << endl;
            cout << "Known users (may not be connected)" << endl;
            for(it = mapIp_->begin(); it!=mapIp_->end(); ++it) {
                cout << it->first << " : " << it->second << endl;
            }
            Trame* ackTrame = new Trame("big-daddy",ACK_CON);
            try {
                socketManager_.sendTrame(ackTrame);
            }catch(runtime_error& e) {
                return;
            }
        }
        if(receivedTrame->getType() == DEM_AMI) {
            string name = receivedTrame->getData();
            if(name.empty()) {
                throw runtime_error("The DEM_AMI trame does not contain a name to add as friend.");
            }
            MapIp::iterator it = mapIp_->find(name);
            if(it == mapIp_->end()) {
                string errorMessage = "The server doesn't know the given name.";
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                try {
                    socketManager_.sendTrame(errorTrame);
                }catch(runtime_error& e) {
                    return;
                }

                continue;
            }
            string ip = it->second;
            SocketManager* otherClientSocketManager = new SocketManager();
            bool reachable = true;
            try {
                otherClientSocketManager->connectTo(name,ip);
            }catch(runtime_error& e) {
                reachable = false;
                string errorMessage = "The client is not connected";
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                socketManager_.sendTrame(errorTrame);
            }
            bool connected = false;
            if(reachable) {
                connected = otherClientSocketManager->checkConnexion("big-daddy");
            }
            if(connected) {
                close(otherClientSocketManager->getSocketDescriptor());
                string response;
                response += name += ':';
                response += ip;
                Trame* ackTrame = new Trame("big-daddy",ACK,response.size(),1,1,response);
                socketManager_.sendTrame(ackTrame);
            }
            else {
                string errorMessage = "The client is not connected.";
                Trame* errorTrame = new Trame("big-daddy",ERROR,errorMessage.size(),1,1,errorMessage);
                socketManager_.sendTrame(errorTrame);
            }
        }
    }
}
