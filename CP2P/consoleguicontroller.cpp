#include "consoleguicontroller.h"

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <fstream>

ConsoleGUIController::ConsoleGUIController(string username,string serverName, string serverIP) : socketManager_(), serverSocketManager_() {
    username_ = username;
    serverSocketManager_.connectTo(serverName,serverIP);
}

ConsoleGUIController::~ConsoleGUIController() {

}

void ConsoleGUIController::handleAddFriend(string friendName) {
    Trame* demAmiTrame = new Trame(username_,DEM_AMI,friendName.size(),1,1,friendName);
    serverSocketManager_.sendTrame(demAmiTrame);
    Trame* serverResponse = serverSocketManager_.receiveTrame();
    if(serverResponse->getType() == ERROR) {
        throw runtime_error(serverResponse->getData());
    }
    else if(serverResponse->getType() == ACK) {
        string name;
        string ip;
        serverResponse->extractNameIP(name,ip);
        cout << "[ConsoleGUIController::handleAddFriend] recevied ACK for DEM_AMI. " << name << " has the adress " << ip << endl;
        mapFriend_.insert(pair<string,string>(name,ip));
    }
}

void ConsoleGUIController::handleCommandMode(string friendName) {
    MapFriend::iterator it = mapFriend_.find(friendName);
    if(it == mapFriend_.end()) {
        throw runtime_error("The given name is not in your friend map.");
    }
    socketManager_.connectTo(it->first,it->second);
    Trame* trame = new Trame(username_,CMD_CON);
    socketManager_.sendTrame(trame);
    Trame* distantHomePath = socketManager_.receiveTrame();
    if(distantHomePath->getType() == ERROR) {
        throw runtime_error(distantHomePath->getData());
    }
    currentPath_ = distantHomePath->getData();
    cout << "[ConsoleGUIController::handleCommandMode] current path on distant client : " << currentPath_ << endl;
}

string ConsoleGUIController::handleCdCommand(string folder) {
    string cdCommand = "cd ";
    cdCommand += folder;
    Trame* cmdTrame = new Trame(username_,CMD,cdCommand.size(),1,1,cdCommand);
    socketManager_.sendTrame(cmdTrame);
    Trame* response = socketManager_.receiveTrame();
    if(response->getType() == ERROR) {
        throw runtime_error(response->getData());
    }
    return response->getData();
}

string ConsoleGUIController::handleLsCommand() {
    string lsCommand = "ls ";
    Trame* cmdTrame = new Trame(username_,CMD,lsCommand.size(),1,1,lsCommand);
    socketManager_.sendTrame(cmdTrame);
    Trame* response = socketManager_.receiveTrame();
    if(response->getType() == ERROR) {
        throw runtime_error(response->getData());
    }
    return response->getData();

}

void ConsoleGUIController::handleGetFileCommand(string distantFilePath) {
    string getFileCommand = "get_file ";
    getFileCommand += distantFilePath;
    Trame* demFicTrame = new Trame(username_,CMD,getFileCommand.size(),1,1,getFileCommand);
    socketManager_.sendTrame(demFicTrame);
    cout << "[ConsoleGUIController::handleGetFileCommand] send a DEM_FIC trame" << endl;
    Trame* response = socketManager_.receiveTrame();
    if(response->getType() == ERROR) {
        throw runtime_error(response->getData());
    }
    string downloadsFolderPath = "Downloads/";
    int fileSize = 0;
    if(response->getNbTrame() > 1) {
        string localSavePath = downloadsFolderPath;
        localSavePath += distantFilePath;
        /*FILE* file = fopen(localSavePath.c_str(),"w");
        if(file == NULL) {
            throw runtime_error("Cannot create the local file for downloading.");
        }*/
        ofstream localFile;
        localFile.open(localSavePath.c_str());
        // handle non opening
        int nbWaitedTrames = response->getNbTrame();
        int nbReceivedTrames = 0;
        Trame* currentTrame = response;
        cout << "received : " << endl;
        cout << currentTrame->getSize() << endl;
        for(int x = 0; x < currentTrame->getSize(); x++) {
            cout << currentTrame->getSerializableTrame()->data[x];
        }
        //cout << response->getData().data() << endl;
        fileSize += currentTrame->getSize();
        localFile.write(currentTrame->getSerializableTrame()->data,currentTrame->getSize());
        // DEBUG
        //localFile.close();
        nbReceivedTrames++;
        bool exitWaitingLoop = false;
        while(!exitWaitingLoop) {
            currentTrame = socketManager_.receiveTrame();
            cout << "received : " << endl;
            for(int x = 0; x < currentTrame->getSize(); x++) {
                cout << currentTrame->getSerializableTrame()->data[x];
            }
            //cout << currentTrame->getData().data() << endl;
            localFile.write(currentTrame->getSerializableTrame()->data,currentTrame->getSize());
            fileSize += currentTrame->getSize();
            nbReceivedTrames++;
            if(nbReceivedTrames == nbWaitedTrames) {
                exitWaitingLoop = 1;
            }
            delete currentTrame;
        }
        localFile.close();

    }
}

void ConsoleGUIController::handlePutFileCommand(string localFilePath) {

}

void ConsoleGUIController::handleCloseCommandMode() {

}

void ConsoleGUIController::handleCloseApp() {

}
