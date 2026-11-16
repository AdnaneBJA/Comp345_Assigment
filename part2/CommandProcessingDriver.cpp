//
// Part 1 driver. Shows that:
//   1) commands can be read from the console with the CommandProcessor
//   2) commands can be read from a text file with the FileCommandProcessorAdapter
//   3) invalid commands are rejected and valid commands change the game state
//
// Usage:  CommandProcessingDriver -console
//         CommandProcessingDriver -file <filename>
//         CommandProcessingDriver                   (menu: console or file)
//

#include <iostream>
#include <sstream>
#include <string>

#include "CommandProcessing.h"
#include "GameEngine.h"

using namespace std;

// File used when no option is given. CMake sets it to the full path of
// part2/commands.txt so the driver finds it from any folder.
#ifndef COMMANDS_FILE
#define COMMANDS_FILE "Commands.txt"
#endif

// Reads commands until quit or the end of the input. Invalid commands are
// rejected, valid ones change the state of the game engine.
void testCommandProcessor(CommandProcessor* processor) {
    GameEngine engine;

    while (!engine.isFinished()) {
        cout << endl << "[" << engine.getStateName() << "] > ";
        Command* command = processor->getCommand();
        if (command == nullptr) {
            cout << endl << "No more commands." << endl;
            break;
        }

        if (processor->validate(command, &engine)) {
            engine.transition(processor->nextState(command));
            command->saveEffect("Accepted, new state: " + engine.getStateName());
        }
        cout << *command << endl;

        // the play phase is done in Part 3, so the driver goes straight to the
        // win state to be able to test replay and quit
        if (engine.getState() == GameEngine::State::AssignReinforcement) {
            cout << "(play phase skipped, done in Part 3)" << endl;
            engine.transition(GameEngine::State::Win);
        }
    }

    cout << endl << *processor << endl;
}

// Asks the user to choose between the console and a file (used when no
// command line option is given)
void askForOption(string& option, string& fileName) {
    string choice;
    while (true) {
        cout << endl << "Where should the commands come from?" << endl;
        cout << "  1) Console" << endl;
        cout << "  2) File" << endl;
        cout << "Choice: ";
        if (!getline(cin, choice)) {
            option = "";
            return;
        }
        if (choice == "1") {
            option = "-console";
            return;
        }
        if (choice == "2") {
            option = "-file";
            cout << "File name (press Enter for " << COMMANDS_FILE << "): ";
            string name;
            getline(cin, name);
            if (name != "") {
                fileName = name;
            }
            return;
        }
        cout << "Invalid choice, enter 1 or 2." << endl;
    }
}

// Creates the console or the file command processor. The option comes from
// the command line (-console or -file <filename>), or from a menu if none is given.
int main(int argc, char* argv[]) {
    cout << "PART 1: COMMAND PROCESSOR" << endl;
    cout << "Commands: loadmap <mapfile>, validatemap, addplayer <playername>, gamestart, replay, quit" << endl;

    string option;
    string fileName = COMMANDS_FILE;
    if (argc >= 2) {
        option = argv[1];
        if (argc >= 3) {
            fileName = argv[2];
        }
    }
    else {
        askForOption(option, fileName);
    }

    CommandProcessor* processor = nullptr;
    if (option == "-console") {
        cout << "Reading commands from the console." << endl;
        processor = new CommandProcessor();
    }
    else if (option == "-file") {
        FileCommandProcessorAdapter* adapter = new FileCommandProcessorAdapter(fileName);
        if (!adapter->isFileOpen()) {
            cout << "Could not open " << fileName << endl;
            delete adapter;
            return 1;
        }
        cout << "Reading commands from the file " << fileName << endl;
        processor = adapter;
    }
    else {
        cout << "Usage: -console or -file <filename>" << endl;
        return 1;
    }

    testCommandProcessor(processor);

    delete processor;
    return 0;
}