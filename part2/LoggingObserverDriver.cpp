#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "CommandProcessing.h"
#include "GameEngine.h"
#include "LoggingObserver.h"
#include "Orders.h"

using namespace std;

// Folder the driver runs in, and the default command file inside it.
// CMake sets PART2_DIR to the full path of part2/.
#ifndef PART2_DIR
#define PART2_DIR "."
#endif
#define DEFAULT_COMMANDS_FILE "LoggingCommands.txt"

// Prints "yes" or "no" for a check
string yesNo(bool value) {
    return value ? "yes" : "no";
}

// Prints whether the class T is a subclass of Subject and of ILoggable
template <typename T>
void printSubclassCheck(string className) {
    cout << "  " << className << ": Subject? " << yesNo(is_base_of<Subject, T>::value)
         << ", ILoggable? " << yesNo(is_base_of<ILoggable, T>::value) << endl;
}

// Prints the current content of the log file
void printLogFile(string fileName) {
    ifstream file(fileName);
    string line;
    int number = 1;
    while (getline(file, line)) {
        cout << "  " << number << ". " << line << endl;
        number++;
    }
}

// Returns a neighbour of the territory owned (ownedByPlayer = true) or not owned
// (false) by the player, or nullptr if there is none
Territory* findNeighbor(Territory* territory, Player* player, bool ownedByPlayer) {
    for (Territory* neighbor : territory->getNeighbors()) {
        if ((neighbor->getPlayer() == player) == ownedByPlayer) {
            return neighbor;
        }
    }
    return nullptr;
}

// Returns a territory of the player that has an enemy neighbour, or the first
// territory of the player if none has one
Territory* findBorderTerritory(Player* player) {
    vector<Territory*> owned = player->getTerritories();
    for (Territory* territory : owned) {
        if (findNeighbor(territory, player, false) != nullptr) {
            return territory;
        }
    }
    return owned.empty() ? nullptr : owned[0];
}

// Points 3, 4 and 5: the players issue orders (added to their lists, and so
// logged), then the game engine executes them (their effects are logged)
void demonstrateOrders(GameEngine* engine) {
    vector<Player*> players = *engine->getPlayers();
    Player* first = players[0];
    Player* second = players[1];

    Territory* border = findBorderTerritory(first);
    Territory* enemy = findNeighbor(border, first, false);
    Territory* friendly = findNeighbor(border, first, true);

    cout << endl << "--- Issuing orders (each OrdersList::addOrder() is logged) ---" << endl;
    engine->transition(GameEngine::State::IssueOrders);

    first->getOrders()->addOrder(new Deploy(first, 6, border));
    if (enemy != nullptr) {
        first->getOrders()->addOrder(new Advance(first, 4, border, enemy));
        first->getOrders()->addOrder(new Deploy(first, 3, enemy));
    }
    if (friendly != nullptr) {
        first->getOrders()->addOrder(new Advance(first, 1, border, friendly));
    }
    second->getOrders()->addOrder(new Negotiate(second, first));
    second->getOrders()->addOrder(new Negotiate(second, second));

    // a card played by a player also adds an order to the player's list
    vector<Card*> cards = second->getHand()->getCards();
    if (!cards.empty()) {
        cout << second->getName() << " plays a card: ";
        cards[0]->play(second, engine->getDeck());
    }

    cout << first->getName() << "'s orders:" << endl << *first->getOrders();
    cout << second->getName() << "'s orders:" << endl << *second->getOrders();

    cout << endl << "--- Executing orders (each Order::execute() is logged) ---" << endl;
    engine->transition(GameEngine::State::ExecuteOrders);
    engine->executeOrdersPhase();
}

// Runs the whole Part 5 demonstration with the given command processor and log observer
void testLoggingObserver(CommandProcessor* processor, LogObserver* logger) {
    logger->clearLog();
    cout << *logger << " (" << filesystem::absolute(logger->getFileName()).string() << ")" << endl;

    cout << endl << "--- 1) Subject and ILoggable subclasses ---" << endl;
    printSubclassCheck<Command>("Command");
    printSubclassCheck<CommandProcessor>("CommandProcessor");
    printSubclassCheck<FileCommandProcessorAdapter>("FileCommandProcessorAdapter");
    printSubclassCheck<Order>("Order");
    printSubclassCheck<Deploy>("Deploy");
    printSubclassCheck<OrdersList>("OrdersList");
    printSubclassCheck<GameEngine>("GameEngine");

    GameEngine* engine = new GameEngine();

    // the same subjects seen through the two base classes, as the observer sees them
    Subject* engineAsSubject = engine;
    ILoggable* engineAsLoggable = engine;
    Subject* processorAsSubject = processor;
    ILoggable* processorAsLoggable = processor;
    cout << "  GameEngine as ILoggable: " << engineAsLoggable->stringToLog() << endl;
    cout << "  CommandProcessor as ILoggable: " << processorAsLoggable->stringToLog() << endl;

    // the log observer watches the command processor and the game engine; they
    // pass it on to every command they create and to every player's orders list
    engineAsSubject->attach(logger);
    processorAsSubject->attach(logger);
    cout << "  GameEngine: " << *engineAsSubject << endl;
    cout << "  CommandProcessor: " << *processorAsSubject << endl;

    cout << endl << "--- 2) Startup phase: saveCommand(), saveEffect() and transition() are logged ---" << endl;
    engine->startupPhase(processor);

    if (engine->getState() == GameEngine::State::AssignReinforcement && engine->getPlayers()->size() >= 2) {
        demonstrateOrders(engine);
    }
    else {
        cout << endl << "The game was not started, so no orders are issued." << endl;
    }

    cout << endl << *processor << endl;
    cout << *engine << endl;

    cout << endl << "--- Content of " << logger->getFileName() << " ---" << endl;
    printLogFile(logger->getFileName());

    delete engine;
    Order::cleanup();
}

// Creates the console or the file command processor from the command line option,
// then runs the demonstration
int main(int argc, char* argv[]) {
    cout << "PART 5: GAME LOG OBSERVER" << endl;

    string option = "-file";
    string fileName = "";
    if (argc >= 2) {
        option = argv[1];
    }
    if (argc >= 3) {
        // resolve the path now, before the working directory changes
        fileName = filesystem::absolute(argv[2]).string();
    }

    filesystem::current_path(PART2_DIR);
    if (fileName == "") {
        fileName = DEFAULT_COMMANDS_FILE;
    }

    CommandProcessor* processor = nullptr;
    if (option == "-console") {
        cout << "Reading commands from the console (maps are in maps/, e.g. loadmap maps/canada.map)." << endl;
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

    // the observer is deleted last, after every subject that points to it
    auto* logger = new LogObserver("gamelog.txt");
    testLoggingObserver(processor, logger);

    delete processor;
    delete logger;
    return 0;
}
