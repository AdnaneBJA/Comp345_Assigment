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

    while(engine.getState() != GameEngine::State::End){

		 // Read commands from console
		if (choice == 1) {
			CommandProcessor processor;
			engine.startupPhase(&processor);
		}
		// Read command from a file
		else if (choice == 2) {
			FileCommandProcessorAdapter processor("src/input.txt");
			engine.startupPhase(&processor);

			if (!processor.isFileOpen()) {
				std::cout << "Could not open Commands.txt\n";
				return 1;
			}
		}
		else {
			std::cout << "Invalid option.\n";
			return 1;
		}

		while(engine.getState() != GameEngine::State::Win){


			std::cout<<std::endl;

			if(engine.getState() == GameEngine::State::AssignReinforcement){
				engine.reinforcementPhase();
			}

			std::cout<<std::endl;


			if(engine.getState() == GameEngine::State::IssueOrders){
				engine.issueOrdersPhase();
			}

			if(engine.getState() == GameEngine::State::ExecuteOrders){
				engine.executeOrdersPhase();
			}
		}

		if(engine.getState() == GameEngine::State::Win){
			std::cout<<std::endl;
			std::cout << "game has ended, Player " << engine.getPlayers()->front()->getName() << " has won!"<< std::endl;
			CommandProcessor processor;
			engine.endingPhase(&processor);
			// free the Neutral player and the shared order lists
			Order::cleanup();
		}
		std::cout << "\nFinal state: " << engine.getStateName() << "\n";

    }


    return 0;
}
