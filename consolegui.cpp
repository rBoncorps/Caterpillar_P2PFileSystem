#include "consolegui.h"

#include <sstream>
#include <iostream>
#include <stdexcept>

/*! \brief Creates a new ConsoleGUI with the given controller.
	\warning This function doesn't start the main loop of the GUI
*/
ConsoleGUI::ConsoleGUI(ConsoleGUIController* controller) {
	controller_ = controller;
}

ConsoleGUI::~ConsoleGUI() {

}

void ConsoleGUI::launchGUI() {

    while(true) {
        cout << "Enter a command or type help"  << endl;
        /* Extract from the given line the command, and all the
         enventual parameters and put it into the currentCommand
         vector.
        */
        string lineEntered;
        getline(cin,lineEntered);
        stringstream ss(lineEntered);
        string item;
        currentCommand_.clear();
        while(std::getline(ss, item, ' ')) {
            currentCommand_.push_back(item);
        }
        if(currentCommand_.empty()) {
            // Handle the case of an empty input
            continue;
        }
        if(currentCommand_[0] == "help") {
            cout << "use \"add_friend <username>\" command to add an other username as a friend." << endl;
            cout << "use \"cmd <username>\" command to start a command mode with the username (must be a friend)." << endl;
            cout << "\t in command mode you can use ls | cd to update the path you're looking at on the remote." << endl;
            cout << "\t use \"get_file\" or \"put_file\" commands to get a remote file or put a local one on the remote." << endl;
        }
        if(currentCommand_[0] == "add_friend") {
            if(currentCommand_.size() < 2) {
                for(int i = 0; i < currentCommand_.size(); i++) {
                    cout << currentCommand_[i] << endl;
                }
                cout << "add_friend : invalid argument, use add_friend <contactName>" << endl;
            }
            else {
                try {
                    controller_->handleAddFriend(currentCommand_[1]);
                }catch(runtime_error& e) {
                    cout << e.what() << endl;
                    continue;
                }
            }
        }
        if(currentCommand_[0] == "exit") {
            cout << "Closing application ..." << endl;
            controller_->handleCloseApp();
            break;
        }
        if(currentCommand_[0] == "cmd") {
            if(currentCommand_.size() < 2) {
                cout << "cmd : invalid argument, use cmd <contactName>" << endl;
            }
            else {
                string currentCmdContact = currentCommand_[1];
                bool exitCmdMode = false;
                try {
                    controller_->handleCommandMode(currentCmdContact);
                }catch(runtime_error& e) {
                    cout << e.what() << endl;
                    continue;
                }
                cout << "You entered command mode. Allowed command : cd <folder> | ls | get_file <file>" << endl;
                while(!exitCmdMode) {
                    cout << "\tDistant:" << currentCmdContact << " > " << controller_->getCurrentPath() << "$ ";
                    string lineEnteredcmd;
                    getline(cin,lineEnteredcmd);
                    stringstream sscmd(lineEnteredcmd);
                    string itemcmd;
                    currentCommand_.clear();
                    while(std::getline(sscmd, itemcmd, ' ')) {
                        currentCommand_.push_back(itemcmd);
                    }
                    if(currentCommand_.empty()) {
                        continue;
                    }
                    if(currentCommand_[0] == "exit") {
                        exitCmdMode = true;
                        cout << "You are leaving command mode." << endl;
                        controller_->handleCloseCommandMode();
                        continue;
                    }
                    if(currentCommand_[0] == "cd") {
                        if(currentCommand_.size() < 2) {
                            cout << "cd : invalid argument, use cd <folder>" << endl;
                            continue;
                        }
                        string cdReturn;
                        try {
                            cdReturn = controller_->handleCdCommand(currentCommand_[1]);
                        }catch(runtime_error& e) {
                            cout << e.what() << endl;
                        }
                        if(!cdReturn.empty()) {
                            cout << cdReturn << endl;
                        }
                    }
                    if(currentCommand_[0] == "ls") {
                        string lsReturn;
                        try {
                            lsReturn = controller_->handleLsCommand();
                        }catch(runtime_error& e) {
                            cout << e.what() << endl;
                        }
                        if(!lsReturn.empty()) {
                            cout << lsReturn << endl;
                        }
                    }
                    if(currentCommand_[0] == "get_file") {
                        if(currentCommand_.size() < 2) {
                            cout << "get_file : invalid argument, use get_file <fileName>" << endl;
                            continue;
                        }
                        try {
                            controller_->handleGetFileCommand(currentCommand_[1]);
                        }catch(runtime_error& e) {
                            cout << e.what() << endl;
                            continue;
                        }
                    }
                    if(currentCommand_[0] == "put_file") {
                        controller_->handlePutFileCommand();
                    }
                }
            }
        }
    }
}


