#include <string>
#include <stdexcept>
#include "receivehandler.h"

// DEBUG
#include <iostream>

ReceiveHandler::ReceiveHandler(int socketDescriptor) : socketManager_(socketDescriptor) {
}

ReceiveHandler::~ReceiveHandler() {

}

void ReceiveHandler::extractNameIP(string from, string &name, string &ip) {
    size_t splitPos = from.find(':');
    if(splitPos == string::npos) {
        cout << "[ReceiveHandler::extractNameIP] The given string does not contain ':'" << endl;
        throw runtime_error("The given string does not contain ':'");
    }
    name = from.substr(0,splitPos);
    ip = from.substr(splitPos+1,from.size());
    cout << "[ReceiveHandler::extractNameIP] extracted " << name << "="<< ip << " from " << from << endl;
}
