#ifndef CONSOLEGUICONTROLLER_H
#define CONSOLEGUICONTROLLER_H

#include "socketmanager.h"
#include "trame.h"
#include <map>
#include <string>

using namespace std;

typedef map<string,string> MapFriend;

class ConsoleGUIController {
public:
    ConsoleGUIController(string username,string serverName, string serverIP);
    ~ConsoleGUIController();

    void handleAddFriend(string friendName);
    void handleCommandMode(string friendName);
    string handleCdCommand(string folder);
    string handleLsCommand();
    void handleGetFileCommand(string distantFilePath);
    void handlePutFileCommand(string localFilePath);
    void handleCloseCommandMode();
    void handleCloseApp();


private:
    string username_;
    string currentPath_;
    SocketManager socketManager_;
    SocketManager serverSocketManager_;
    MapFriend mapFriend_;
};

#endif // CONSOLEGUICONTROLLER_H
