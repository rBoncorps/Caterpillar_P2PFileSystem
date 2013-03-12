#include "consoleguicontroller.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <fstream>
#include <cmath>

ConsoleGUIController::ConsoleGUIController(string username,string userIP,string serverName, string serverIP) : socketManager_(), serverSocketManager_() {
    username_ = username;
    userIP_ = userIP;
    serverName_ = serverName;
    serverIP_ = serverIP;
}

ConsoleGUIController::~ConsoleGUIController() {

}

bool ConsoleGUIController::connectServer() {
    serverSocketManager_.connectTo(serverName_,serverIP_);
    string msg = username_;
    msg += ':';
    msg += userIP_;
    Trame* connexionTrame = new Trame(username_,CON_SERV,msg.size(),1,1,msg);
    serverSocketManager_.sendTrame(connexionTrame);
    // Receive the server response
    Trame* ackTrame = serverSocketManager_.receiveTrame();
    if(ackTrame->getType() == ACK_CON) {
        return true;
    }
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
    else if(response->getType() == MAJ_PATH) {
        currentPath_ = response->getData();
        return "";
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
    //getting home of the current user
    string homePath;
    char buffer[128];
    FILE* home = popen("echo $HOME","r");
    if(home == NULL) {
        cout << "[ClientReceiveHandler::launchReception] error during the getHome command." << endl;
        return;
    }
    while(!feof(home)) {
        if(fgets(buffer, 128, home) != NULL)
            homePath += buffer;
    }
    pclose(home);
    homePath[homePath.size()-1] = '/';
    string downloadsFolderPath = homePath;

    //sending check_file_existance trame
    string checkFileCommand = "check_file ";
    checkFileCommand += distantFilePath;
    Trame* checkFicTrame = new Trame(username_,CMD,checkFileCommand.size(),1,1,checkFileCommand);
    socketManager_.sendTrame(checkFicTrame);
    Trame* respCheck = socketManager_.receiveTrame();
    if(respCheck->getData().at(0) == 'N') {
        cout << "\tThe file is not available on remote client." << endl;
        cout << "\tPlease try again." << endl;
        return;
    }
    else if(respCheck->getData().at(0) == 'O') {
        cout << "\n\tPlease select a download location." << endl;
        cout << "\t\tcommand : cd <path> | ls | mkdir <newDirectory>" << endl;
        cout << "\t\ttype \"select\" to select the current location for download" << endl;
        cout << "\t\tThe current download path is : " << downloadsFolderPath << endl;
        bool pathSelected = false;
        vector<string> currentCommand;
        while(!pathSelected) {
            cout << "\t\tLocal > " << downloadsFolderPath << "$ ";
            string lineEnteredcmd;
            getline(cin,lineEnteredcmd);
            stringstream sscmd(lineEnteredcmd);
            string itemcmd;
            currentCommand.clear();
            while(std::getline(sscmd, itemcmd, ' ')) {
                currentCommand.push_back(itemcmd);
            }
            if(currentCommand.empty()) {
                continue;
            }
            if(currentCommand[0] == "select") {
                pathSelected = true;
                cout << "Download directory : " << downloadsFolderPath << endl;
                continue;
            }
            if(currentCommand[0] == "cd") {
                if(currentCommand.size() < 2) {
                    cout << "\t\tcd command : invalid arg number." << endl;
                    continue;
                }
                string realPath = downloadsFolderPath;
                realPath += currentCommand[1];
                string command = "cd ";
                command += realPath;
                FILE* cdCmd = popen(command.c_str(),"r");
                char buffer[128];
                string cdReturn;
                if(cdCmd == NULL) {
                    cout << "\t\t error during the cd command." << endl;
                    return;
                }
                while(!feof(cdCmd)) {
                    if(fgets(buffer, 128, cdCmd) != NULL)
                        cdReturn += buffer;
                }
                pclose(cdCmd);
                // The cd appened succesfuly
                if(cdReturn.empty()) {
                    downloadsFolderPath = realPath;
                    downloadsFolderPath += "/";

                }
            }
            if(currentCommand[0] == "ls") {
                string command = "ls ";
                command += downloadsFolderPath;
                FILE* lsCmd = popen(command.c_str(),"r");
                char buffer[128];
                string lsReturn;
                if(lsCmd == NULL) {
                    cout << "[ClientReceiveHandler::launchReception] error during the ls command." << endl;
                    return;
                }
                while(!feof(lsCmd)) {
                    if(fgets(buffer, 128, lsCmd) != NULL)
                        lsReturn += buffer;
                }
                pclose(lsCmd);
                cout << lsReturn << endl;
            }
            if(currentCommand[0] == "mkdir") {
                string command = "mkdir ";
                command += downloadsFolderPath;
                command += currentCommand[1];
                FILE* mkdirCmd = popen(command.c_str(),"r");
                char buffer[128];
                string mkdirReturn;
                if(mkdirCmd == NULL) {
                    cout << "[ClientReceiveHandler::launchReception] error during the mkdir command." << endl;
                    return;
                }
                while(!feof(mkdirCmd)) {
                    if(fgets(buffer, 128, mkdirCmd) != NULL)
                        mkdirReturn += buffer;
                }
                pclose(mkdirCmd);
                if(mkdirReturn.empty()) {
                    cout << "[ClientReceiveHandler::launchReception] mkdir succesfull" << endl;
                }
            }
        }
    }

    string getFileCommand = "get_file ";
    getFileCommand += distantFilePath;

    Trame* demFicTrame = new Trame(username_,CMD,getFileCommand.size(),1,1,getFileCommand);
    socketManager_.sendTrame(demFicTrame);
    //cout << "[ConsoleGUIController::handleGetFileCommand] send a DEM_FIC trame" << endl;
    Trame* response = socketManager_.receiveTrame();
    if(response->getType() == ERROR) {
        throw runtime_error(response->getData());
    }
    int fileSize = 0;
    if(response->getNbTrame() >= 1) {
        string localSavePath = downloadsFolderPath;
        localSavePath += distantFilePath;
        ofstream localFile;
        localFile.open(localSavePath.c_str());
        // handle non opening
        int nbWaitedTrames = response->getNbTrame();
        int nbReceivedTrames = 0;
        Trame* currentTrame = response;
        fileSize += currentTrame->getSize();
        localFile.write(currentTrame->getSerializableTrame()->data,currentTrame->getSize());
        nbReceivedTrames++;
        bool exitWaitingLoop = false;
        if(response->getNbTrame() == 1) {
            exitWaitingLoop = true;
        }
        while(!exitWaitingLoop) {
            currentTrame = socketManager_.receiveTrame();
            localFile.write(currentTrame->getSerializableTrame()->data,currentTrame->getSize());
            fileSize += currentTrame->getSize();
            nbReceivedTrames++;
            if(nbReceivedTrames == 1) {
                cout << "Start download ..." << endl;
            }
            if(nbReceivedTrames == floor(nbWaitedTrames/4)) {
                cout << "25% received" << endl;
            }
            if(nbReceivedTrames == floor(nbWaitedTrames/2)) {
                cout << "50% received" << endl;
            }
            if(nbReceivedTrames == floor(3*nbWaitedTrames/4)) {
                cout << "75% received" << endl;
            }
            if(nbReceivedTrames == nbWaitedTrames) {
                cout << "100% received" << endl;
                exitWaitingLoop = 1;
            }
            delete currentTrame;
        }
        localFile.close();

    }
}

void ConsoleGUIController::handlePutFileCommand() {
    //getting home of the current user
    string homePath;
    char buffer[128];
    FILE* home = popen("echo $HOME","r");
    if(home == NULL) {
        cout << "[ClientReceiveHandler::launchReception] error during the getHome command." << endl;
        return;
    }
    while(!feof(home)) {
        if(fgets(buffer, 128, home) != NULL)
            homePath += buffer;
    }
    pclose(home);
    homePath[homePath.size()-1] = '/';
    string downloadsFolderPath = homePath;

    cout << "\n\tPlease select the file to upload." << endl;
    cout << "\t\tcommand : cd <path> | ls>" << endl;
    cout << "\t\ttype \"select <file_name>\" to select the file to upload" << endl;
    cout << "\t\tThe current local path is : " << downloadsFolderPath << endl;
    bool pathSelected = false;
    vector<string> currentCommand;
    string fileName;
    while(!pathSelected) {
        cout << "\t\tLocal > " << downloadsFolderPath << "$ ";
        string lineEnteredcmd;
        getline(cin,lineEnteredcmd);
        stringstream sscmd(lineEnteredcmd);
        string itemcmd;
        currentCommand.clear();
        while(std::getline(sscmd, itemcmd, ' ')) {
            currentCommand.push_back(itemcmd);
        }
        if(currentCommand.empty()) {
            continue;
        }
        if(currentCommand[0] == "select") {
            pathSelected = true;
            downloadsFolderPath += currentCommand[1];
            fileName = currentCommand[1];
            cout << "Download directory : " << downloadsFolderPath << endl;
            continue;
        }
        if(currentCommand[0] == "cd") {
            if(currentCommand.size() < 2) {
                cout << "\t\tcd command : invalid arg number." << endl;
                continue;
            }
            string realPath = downloadsFolderPath;
            realPath += currentCommand[1];
            string command = "cd ";
            command += realPath;
            FILE* cdCmd = popen(command.c_str(),"r");
            char buffer[128];
            string cdReturn;
            if(cdCmd == NULL) {
                cout << "\t\t error during the cd command." << endl;
                return;
            }
            while(!feof(cdCmd)) {
                if(fgets(buffer, 128, cdCmd) != NULL)
                    cdReturn += buffer;
            }
            pclose(cdCmd);
            // The cd appened succesfuly
            if(cdReturn.empty()) {
                downloadsFolderPath = realPath;
                downloadsFolderPath += "/";

            }
        }
        if(currentCommand[0] == "ls") {
            string command = "ls ";
            command += downloadsFolderPath;
            FILE* lsCmd = popen(command.c_str(),"r");
            char buffer[128];
            string lsReturn;
            if(lsCmd == NULL) {
                cout << "[ClientReceiveHandler::launchReception] error during the ls command." << endl;
                return;
            }
            while(!feof(lsCmd)) {
                if(fgets(buffer, 128, lsCmd) != NULL)
                    lsReturn += buffer;
            }
            pclose(lsCmd);
            cout << lsReturn << endl;
        }
    }

    string putFileCommand = "put_file ";
    putFileCommand += fileName;

    Trame* envFicTrame = new Trame(username_,CMD,putFileCommand.size(),1,1,putFileCommand);
    socketManager_.sendTrame(envFicTrame);
    Trame* response = socketManager_.receiveTrame();
    if(response->getType() == ERROR) {
        throw runtime_error(response->getData());
    }
    string filePath = downloadsFolderPath;
    cout << "file to upload : " << filePath << endl;
    FILE* file = fopen(filePath.c_str(),"r");
    if(file == NULL) {
        string errorMessage = "Cannot open the file ";
        errorMessage += downloadsFolderPath;
        cout << errorMessage << endl;
        Trame* errorTrame = new Trame(username_,ERROR,errorMessage.size(),1,1,errorMessage);
        socketManager_.sendTrame(errorTrame);
    }
    else {
        int pos = fseek(file,0,SEEK_END);
        int size = ftell(file);
        rewind(file);
        int nbTrames = (int)ceil((double)size / (double)MAX_DATA_SIZE);
        int readed = 0;
        int sended = 0;
        for(int i = 0; i < nbTrames; i++) {
            char fileBuffer[MAX_DATA_SIZE];
            int read = fread(fileBuffer,sizeof(char),MAX_DATA_SIZE,file);
            readed += read;
            string bufString;
            bufString.assign(fileBuffer,read);
            Trame* ficTrame = new Trame(username_,ENV_FIC,read,1,nbTrames,bufString);
            socketManager_.sendTrame(ficTrame);
            sended++;
            if(sended == 1) {
                cout << "Uploading " << fileName << endl;
            }
            if(sended == floor(nbTrames/4)) {
                cout << "25% sended" << endl;
            }
            if(sended == floor(nbTrames/2)) {
                cout << "50% sended" << endl;
            }
            if(sended == floor(3*nbTrames/4)) {
                cout << "75% sended" << endl;
            }
            if(sended == nbTrames) {
                cout << "100% sended" << endl;
            }
            delete ficTrame;
        }
        pclose(file);
    }
}

void ConsoleGUIController::handleCloseCommandMode() {

}

void ConsoleGUIController::handleCloseApp() {

}

string ConsoleGUIController::getCurrentPath() {
    return currentPath_;
}
