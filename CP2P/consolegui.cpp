#include "consolegui.h"

#include <iostream>

/*! \brief Creates a new ConsoleGUI with the given controller.
	\warning This function doesn't start the main loop of the GUI
*/
ConsoleGUI::ConsoleGUI(ConsoleGUIController* controller) {
	controller_ = controller;
}

ConsoleGUI::~ConsoleGUI() {

}

void ConsoleGUI::launchGUI() {
	while(currentCommand_ != exit) {
		cout << "Enter a command or type help"  endl;
		cin >> currentCommand_;

