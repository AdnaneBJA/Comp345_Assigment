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
        FileCommandProcessorAdapter processor("src/input.txt");

        if (!processor.isFileOpen()) {
            std::cout << "Could not open Commands.txt\n";
            return 1;
        }
        engine.startupPhase(&processor);

		if (engine.getState() == GameEngine::State::AssignReinforcement) {
		std::cout << "\nInitial reinforcement pools:\n";

		for (Player* player : engine.getPlayers()) {
			std::cout << player->getName() << ": " << engine.getReinforcementPool(player) << " armies\n";
			}
		}

		engine.reinforcementPhase();

		std::cout<<std::endl;

		std::cout << "\nFinal reinforcement pools:\n";
		for (Player* player : engine.getPlayers()) {
			std::cout << player->getName() << ": " << engine.getReinforcementPool(player) << " armies\n";
			}

		if(engine.getState() == GameEngine::State::IssueOrders){
			engine.issueOrdersPhase();
		}


    }
    else {
        std::cout << "Invalid option.\n";
        return 1;
    }




    std::cout << "\nFinal state: " << engine.getStateName() << "\n";

    return 0;
}
