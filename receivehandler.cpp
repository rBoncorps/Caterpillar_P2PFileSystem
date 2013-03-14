#include <string>
#include <stdexcept>
#include "receivehandler.h"

// DEBUG
#include <iostream>

ReceiveHandler::ReceiveHandler(string name, int socketDescriptor) : socketManager_(socketDescriptor) {
    name_ = name;
}

ReceiveHandler::~ReceiveHandler() {

}
