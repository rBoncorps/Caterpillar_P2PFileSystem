#ifndef CONSOLEGUI_H
#define CONSOLEGUI_H

#include "consoleguicontroller.h"

#include <string>

using namespace std;

/*! \brief A class allowing user interactions with the core.

	Provides a console interface (in the terminal where the
	program has been launched.
*/
class ConsoleGUI {
	
public:
	ConsoleGUI(ConsoleGUIController* controller);
	~ConsoleGUI();
	void launchGUI();

private:
	ConsoleGUIController* controller_;
	string currentCommand_;

};

#endif
