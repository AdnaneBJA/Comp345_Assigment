#include "GameEngine.h"
#include "CommandProcessing.h"
#include <iostream>

int main() {
    GameEngine engine;
    int choice;
    // Display command options between console and file
    std::cout << "Testing Game Startup\n";
    std::cout << "1. Console commands\n";
    std::cout << "2. File commands\n";
    std::cout << "Choose an option: ";
    std::cin >> choice;
    // Read commands from console
    if (choice == 1) {
        CommandProcessor processor;
        engine.startupPhase(&processor);
    }
    // Read command from a file
    else if (choice == 2) {
        FileCommandProcessorAdapter processor("EngineCommands.txt");

        if (!processor.isFileOpen()) {
            std::cout << "Could not open EngineCommands.txt\n";
            return 1;
        }
        engine.startupPhase(&processor);
    }
    else {
        std::cout << "Invalid option.\n";
        return 1;
    }
    
    if (engine.getState() == GameEngine::State::AssignReinforcement) {
    std::cout << "\nFinal reinforcement pools:\n";
    for (Player* player : *engine.getPlayers()) {
        std::cout << player->getName() << ": " << engine.getReinforcementPool(player) << " armies\n";
        }
    }
    std::cout << "\nFinal state: "
              << engine.getStateName() << "\n";

    return 0;
}
