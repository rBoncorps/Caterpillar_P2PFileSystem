#include "clientreceivehandler.h"
// DEBUG
#include <iostream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <stdexcept>

ClientReceiveHandler::ClientReceiveHandler(string name, int socketDescriptor) : ReceiveHandler(name, socketDescriptor) {

}

ClientReceiveHandler::~ClientReceiveHandler() {

}

void ClientReceiveHandler::launchReception() {
    Trame* receivedTrame;
    try {
        receivedTrame = socketManager_.receiveTrame();
    }catch(runtime_error& e) {
        cout << "Remote ends up, closing the connexion" << endl;
        close(socketManager_.getSocketDescriptor());
        return;
    }
    if(receivedTrame->getType() == CHECK_CON) {
        Trame* ackTrame = new Trame(name_,ACK_CON);
        try {
            socketManager_.sendTrame(ackTrame);
            delete ackTrame;
        }catch(runtime_error& e) {
            return;
        }
    }
    else if(receivedTrame->getType() == CMD_CON) {
        string currentPath;
        char buffer[128];
        FILE* home = popen("echo $HOME","r");
        if(home == NULL) {
            return;
        }
        while(!feof(home)) {
            if(fgets(buffer, 128, home) != NULL)
                currentPath += buffer;
        }
        pclose(home);
        currentPath[currentPath.size()-1] = '/';
        Trame* homePathTrame = new Trame(name_,CMD_HOME,currentPath.size(),1,1,currentPath);
        try {
            socketManager_.sendTrame(homePathTrame);
            delete homePathTrame;
        }catch(runtime_error& e) {
            return;
        }

        bool exitCmdMode = false;
        while(!exitCmdMode) {
            Trame* receivedCMDTrame;
            try {
                receivedCMDTrame = socketManager_.receiveTrame();
            }catch(runtime_error& e) {
                return;
            }
            if(receivedCMDTrame->getType() == CMD) {
                vector<string> currentCMD;
                stringstream ss(receivedCMDTrame->getData());
                string item;
                while(std::getline(ss, item, ' ')) {
                    currentCMD.push_back(item);
                }
                if(currentCMD.empty()) {
                    continue;
                }
                if(currentCMD[0] == "cd") {
                    if(currentCMD.size() < 2) {
                        continue;
                    }
                    Trame* cdTrame;
                    string realPath = currentPath;
                    realPath += currentCMD[1];
                    string command = "cd ";
                    command += realPath;
                    FILE* cdCmd = popen(command.c_str(),"r");
                    char buffer[128];
                    string cdReturn;
                    if(cdCmd == NULL) {
                        return;
                    }
                    while(!feof(cdCmd)) {
                        if(fgets(buffer, 128, cdCmd) != NULL)
                            cdReturn += buffer;
                    }
                    pclose(cdCmd);

                    // The cd appened succesfuly
                    if(cdReturn.empty()) {
                        currentPath = realPath;
                        currentPath += "/";
                        cdTrame = new Trame(name_,MAJ_PATH,currentPath.size(),1,1,currentPath);
                    }
                    else {
                        cdTrame = new Trame(name_,CD_RET,cdReturn.size(),1,1,cdReturn);
                    }
                    try {
                        socketManager_.sendTrame(cdTrame);
                        delete cdTrame;
                    }catch(runtime_error& e) {
                        return;
                    }
                }
                else if(currentCMD[0] == "ls") {
                    string command = "ls ";
                    command += currentPath;
                    FILE* lsCmd = popen(command.c_str(),"r");
                    char buffer[128];
                    string lsReturn;
                    if(lsCmd == NULL) {
                        return;
                    }
                    while(!feof(lsCmd)) {
                        if(fgets(buffer, 128, lsCmd) != NULL)
                            lsReturn += buffer;
                    }
                    pclose(lsCmd);
                    Trame* lsTrame = new Trame(name_,LS_RET,lsReturn.size(),1,1,lsReturn);
                    try {
                        socketManager_.sendTrame(lsTrame);
                        delete lsTrame;
                    }catch(runtime_error& e) {
                        return;
                    }
                }
                else if(currentCMD[0] == "check_file") {
                    if(currentCMD.size() < 2) {
                        continue;
                    }
                    string filePath = currentPath;
                    filePath += currentCMD[1];
                    FILE* file = fopen(filePath.c_str(),"r");
                    string msg;
                    Trame* checkFileRetTrame;
                    if(file == NULL) {
                        msg = "N";
                        checkFileRetTrame = new Trame(name_,CMD,msg.size(),1,1,msg);
                    }
                    else {
                        pclose(file);
                        msg = "O";
                        checkFileRetTrame = new Trame(name_,CMD,msg.size(),1,1,msg);
                    }
                    try {
                        socketManager_.sendTrame(checkFileRetTrame);
                        delete checkFileRetTrame;
                    }catch(runtime_error& e) {
                        return;
                    }
                }
                else if(currentCMD[0] == "get_file") {
                    if(currentCMD.size() < 2) {
                        continue;
                    }
                    string filePath = currentPath;
                    filePath += currentCMD[1];
                    FILE* file = fopen(filePath.c_str(),"r");
                    if(file == NULL) {
                        string errorMessage = "Cannot open the file ";
                        errorMessage += currentCMD[1];
                        Trame* errorTrame = new Trame(name_,ERROR,errorMessage.size(),1,1,errorMessage);
                        try {
                            socketManager_.sendTrame(errorTrame);
                            delete errorTrame;
                        }catch(runtime_error& e) {
                            return;
                        }
                    }
                    else {
                        int pos = fseek(file,0,SEEK_END);
                        int size = ftell(file);
                        rewind(file);
                        int nbTrames = (int)ceil((double)size / (double)MAX_DATA_SIZE);

                        int readed = 0;
                        for(int i = 0; i < nbTrames; i++) {
                            char fileBuffer[MAX_DATA_SIZE];
                            int read = fread(fileBuffer,sizeof(char),MAX_DATA_SIZE,file);
                            readed += read;
                            string bufString;
                            bufString.assign(fileBuffer,read);
                            Trame* ficTrame = new Trame(name_,ENV_FIC,read,1,nbTrames,bufString);
                            try {
                                socketManager_.sendTrame(ficTrame);
                                delete ficTrame;
                            }catch(runtime_error& e) {
                                return;
                            }
                        }
                        pclose(file);
                    }
                }
                else if(currentCMD[0] == "put_file") {
                    if(currentCMD.size() < 2) {
                        continue;
                    }
                    string filePath = currentPath;
                    filePath += currentCMD[1];
                    FILE* file = fopen(filePath.c_str(),"w");
                    if(file == NULL) {
                        // handle error
                    }
                    else {
                        Trame* ackTrame = new Trame(name_,ACK);
                        try {
                            socketManager_.sendTrame(ackTrame);
                            delete ackTrame;
                        }catch(runtime_error& e) {
                            return;
                        }
                        Trame* response;
                        try {
                            response = socketManager_.receiveTrame();
                        }catch(runtime_error& e) {
                            return;
                        }
                        if(response->getType() == ERROR) {
                        }
                        else {
                            int fileSize = 0;
                            if(response->getNbTrame() >= 1) {
                                string localSavePath = currentPath;
                                localSavePath += currentCMD[1];
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
                                delete currentTrame;
                                while(!exitWaitingLoop) {
                                    try {
                                        currentTrame = socketManager_.receiveTrame();
                                    }catch(runtime_error& e) {
                                        return;
                                    }
                                    localFile.write(currentTrame->getSerializableTrame()->data,currentTrame->getSize());
                                    fileSize += currentTrame->getSize();
                                    nbReceivedTrames++;
                                    if(nbReceivedTrames == 1) {
                                        cout << "Getting " << currentCMD[1] << " at " <<  currentPath << endl;
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
                    }
                }
            }
            delete receivedCMDTrame;
        }
    }
}
