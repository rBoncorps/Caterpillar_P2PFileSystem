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
        cout << "[ClientReceiveHandler::launchReception] received a CHECK_CON from " << receivedTrame->getFrom() << endl;
        Trame* ackTrame = new Trame(name_,ACK_CON);
        socketManager_.sendTrame(ackTrame);
    }
    else if(receivedTrame->getType() == CMD_CON) {
        cout << "[ClientReceiveHandler::launchReception] received a CMD_CON from " << receivedTrame->getFrom() << endl;
        string currentPath;
        char buffer[128];
        FILE* home = popen("echo $HOME","r");
        if(home == NULL) {
            cout << "[ClientReceiveHandler::launchReception] error during the getHome command." << endl;
            return;
        }
        while(!feof(home)) {
            if(fgets(buffer, 128, home) != NULL)
                currentPath += buffer;
        }
        pclose(home);
        currentPath[currentPath.size()-1] = '/';
        cout << "[ClientReceiveHandler::launchReception] CurrentPath : " << currentPath << endl;
        Trame* homePathTrame = new Trame(name_,CMD_HOME,currentPath.size(),1,1,currentPath);
        socketManager_.sendTrame(homePathTrame);
        bool exitCmdMode = false;
        while(!exitCmdMode) {
            Trame* receivedCMDTrame;
            try {
                receivedCMDTrame = socketManager_.receiveTrame();
            }catch(runtime_error& e) {
                cout << "Remote ends up, closing the connexion" << endl;
                return;
            }
            if(receivedCMDTrame->getType() == CMD) {
                cout << "[ClientReceiveHandler::launchReception] Received a command" << endl;
                vector<string> currentCMD;
                stringstream ss(receivedCMDTrame->getData());
                string item;
                while(std::getline(ss, item, ' ')) {
                    currentCMD.push_back(item);
                }
                if(currentCMD.empty()) {
                    cout << "[ClientReceiveHandler::launchReception] Received an invalid command, aborting handeling." << endl;
                    continue;
                }
                if(currentCMD[0] == "cd") {
                    if(currentCMD.size() < 2) {
                        cout << "[ClientReceiveHandler::launchReception] cd command : invalid arg number." << endl;
                        continue;
                    }
                    string realPath = currentPath;
                    realPath += currentCMD[1];
                    cout << "[ClientReceiveHandler::launchReception] realPath : " << realPath << endl;
                    string command = "cd ";
                    command += realPath;
                    cout << "[ClientReceiveHandler::launchReception] command : " << command << endl;
                    FILE* cdCmd = popen(command.c_str(),"r");
                    char buffer[128];
                    string cdReturn;
                    if(cdCmd == NULL) {
                        cout << "[ClientReceiveHandler::launchReception] error during the cd command." << endl;
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
                        cout << "[ClientReceiveHandler::launchReception] cd appends succesfuly" << endl;
                    }
                    cout << "[ClientReceiveHandler::launchReception] new currentPath : " << currentPath << endl;
                    Trame* cdTrame = new Trame(name_,CD_RET,cdReturn.size(),1,1,cdReturn);
                    socketManager_.sendTrame(cdTrame);
                }
                else if(currentCMD[0] == "ls") {
                    string command = "ls ";
                    command += currentPath;
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
                    cout << "[ClientReceiveHandler::launchReception] ls return : " << lsReturn << endl;
                    Trame* lsTrame = new Trame(name_,LS_RET,lsReturn.size(),1,1,lsReturn);
                    socketManager_.sendTrame(lsTrame);
                }
                else if(currentCMD[0] == "check_file") {
                    if(currentCMD.size() < 2) {
                        cout << "[ClientReceiveHandler::launchReception] check_file command : invalid arg number." << endl;
                        continue;
                    }
                    cout << "[ClientReceiveHandler::launchReception] want to check the existance of file  " << currentPath << currentCMD[1] << endl;
                    string filePath = currentPath;
                    filePath += currentCMD[1];
                    FILE* file = fopen(filePath.c_str(),"r");
                    string msg;
                    Trame* checkFileRetTrame;
                    if(file == NULL) {
                        cout << "the file doesn't exist" << endl;
                        msg = "N";
                        checkFileRetTrame = new Trame(name_,CMD,msg.size(),1,1,msg);
                    }
                    else {
                        pclose(file);
                        cout << "the file exists" << endl;
                        msg = "O";
                        checkFileRetTrame = new Trame(name_,CMD,msg.size(),1,1,msg);
                    }
                    socketManager_.sendTrame(checkFileRetTrame);
                }
                else if(currentCMD[0] == "get_file") {
                    if(currentCMD.size() < 2) {
                        cout << "[ClientReceiveHandler::launchReception] get_file command : invalid arg number." << endl;
                        continue;
                    }
                    cout << "[ClientReceiveHandler::launchReception] want to get the file " << currentPath << currentCMD[1] << endl;
                    string filePath = currentPath;
                    filePath += currentCMD[1];
                    FILE* file = fopen(filePath.c_str(),"r");
                    if(file == NULL) {
                        string errorMessage = "Cannot open the file ";
                        errorMessage += currentCMD[1];
                        Trame* errorTrame = new Trame(name_,ERROR,errorMessage.size(),1,1,errorMessage);
                        socketManager_.sendTrame(errorTrame);
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
                            socketManager_.sendTrame(ficTrame);
                            delete ficTrame;
                        }
                        pclose(file);
                    }
                }
            }
        }
    }
}
