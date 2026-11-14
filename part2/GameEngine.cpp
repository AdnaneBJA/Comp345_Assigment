#include <utility> 
#include <algorithm>
#include <random> 
#include <sstream>
#include "CommandProcessing.h"
#include "GameEngine.h"

// Default constructor
GameEngine::GameEngine() {
    currentState = new State(State::Start);
    gameMap = nullptr;
    players = new std::vector<Player*>();
    deck = new Deck();
    mapLoader = new MapLoader();
    reinforcementPools = new std::vector<int>();
}

// Copy constructor
GameEngine::GameEngine(const GameEngine& other) {
    currentState = new State(*other.currentState);

    gameMap = nullptr;
    if (other.gameMap != nullptr) {
        gameMap = new Map(*other.gameMap);
    }

    players = new std::vector<Player*>();

    for (Player* player : *other.players) {
        players->push_back(new Player(*player));
    }

    deck = new Deck(*other.deck);
    mapLoader = new MapLoader();
    reinforcementPools = new std::vector<int>(*other.reinforcementPools);
}

// Assignment operator
GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        GameEngine copy(other);

        std::swap(currentState, copy.currentState);
        std::swap(gameMap, copy.gameMap);
        std::swap(players, copy.players);
        std::swap(deck, copy.deck);
        std::swap(mapLoader, copy.mapLoader);
        std::swap(reinforcementPools, copy.reinforcementPools);
    }

    return *this;
}

// Destructor
GameEngine::~GameEngine() {
    delete currentState;

    for (Player* player : *players) {
        delete player;
    }

    delete players;
    delete gameMap;
    delete deck;
    delete mapLoader;
    delete reinforcementPools;
}

// Return current state as enum value
GameEngine::State GameEngine::getState() const {
    return *currentState;
}

// Convert each state into displayable text
std::string GameEngine::stateName(State state) {
    switch (state) {
    case State::Start: return "start";
    case State::MapLoaded: return "map loaded";
    case State::MapValidated: return "map validated";
    case State::PlayersAdded: return "players added";
    case State::AssignReinforcement: return "assign reinforcement";
    case State::IssueOrders: return "issue orders";
    case State::ExecuteOrders: return "execute orders";
    case State::Win: return "win";
    case State::End: return "end";
    }
    return "unknown";
}
// Get readable name of state
std::string GameEngine::getStateName() const {
    return stateName(*currentState);
}

// Implements the labeled edges in the Assignment 1 diagram
// No actual map loading or creations yet
/* bool GameEngine::nextState(const std::string& command, State& result) const {
    switch (*currentState) {
    case State::Start:
        if (command == "loadmap") { result = State::MapLoaded; return true; }
        break;
    case State::MapLoaded:
        if (command == "loadmap") { result = State::MapLoaded; return true; }
        if (command == "validatemap") { result = State::MapValidated; return true; }
        break;
    case State::MapValidated:
        if (command == "addplayer") { result = State::PlayersAdded; return true; }
        break;
    case State::PlayersAdded:
        if (command == "addplayer") { result = State::PlayersAdded; return true; }
        if (command == "assigncountries") { result = State::AssignReinforcement; return true; }
        break;
    case State::AssignReinforcement:
        if (command == "issueorder") { result = State::IssueOrders; return true; }
        break;
    case State::IssueOrders:
        if (command == "issueorder") { result = State::IssueOrders; return true; }
        if (command == "endissueorders") { result = State::ExecuteOrders; return true; }
        break;
    case State::ExecuteOrders:
        if (command == "execorder") { result = State::ExecuteOrders; return true; }
        if (command == "endexecorders") { result = State::AssignReinforcement; return true; }
        if (command == "win") { result = State::Win; return true; }
        break;
    case State::Win:
        if (command == "play") { result = State::Start; return true; }
        if (command == "end") { result = State::End; return true; }
        break;
    case State::End:
        break;
    }
    return false;
} */

// Accepts valid commands to update the state and reject invalid commands then print the result, does not change the current state
/* bool GameEngine::processCommand(const std::string& command) {
    State destination = *currentState;
    if (!nextState(command, destination)) {
        std::cout << "Invalid command '" << command
                  << "' in state '" << getStateName() << "'. State unchanged.\n";
        return false;
    }

    const std::string previous = getStateName();
    *currentState = destination;
    std::cout << "Command accepted: " << command << "\n"
              << "Transition: " << previous << " -> " << getStateName() << "\n";
    return true;
} */

// Returns true once the engine reaches the End state.
bool GameEngine::isFinished() const {
    return *currentState == State::End;
}

// Moves the engine to the given state and prints the transition.
void GameEngine::transition(State newState) {
    const std::string previous = getStateName();
    *currentState = newState;
    std::cout << "Transition: " << previous << " -> " << getStateName() << "\n";
}

// Loadmap from a file and update the state
bool GameEngine::loadMap(const std::string& filename) {
    Map* newMap = mapLoader->load_map(filename);
    if (newMap == nullptr) {
        std::cout << "Failed to load map: " << filename << "\n";
        return false;
    }
    delete gameMap;
    gameMap = newMap;
    transition(State::MapLoaded);
    std::cout << "Map loaded successfully: " << filename << "\n";
    return true;
}

// Validate map that is loaded before adding players
bool GameEngine::validateMap() {
    if (getState() != State::MapLoaded) {
        std::cout << "Cannot validate map in the current state.\n";
        return false;
    }
    if (gameMap == nullptr) {
        std::cout << "No map has been loaded.\n";
        return false;
    }
    if (!gameMap->validate()) {
        std::cout << "Map validation failed.\n";
        return false;
    }

    transition(State::MapValidated);
    std::cout << "Map validated successfully.\n";
    return true;
}

// Add a player, includes a player limit and no duplicate names
bool GameEngine::addPlayer(const std::string& name) {
    if (getState() != State::MapValidated &&
        getState() != State::PlayersAdded) {
        std::cout << "Cannot add players in the current state.\n";
        return false;
    }
    if (name.empty()) {
        std::cout << "Player name cannot be empty.\n";
        return false;
    }
    if (players->size() >= 6) {
        std::cout << "Maximum of 6 players allowed.\n";
        return false;
    }
    for (Player* player : *players) {
        if (player->getName() == name) {
            std::cout << "Player name already exists.\n";
            return false;
        }
    }

    Player* newPlayer = new Player(name);
    players->push_back(newPlayer);
    transition(State::PlayersAdded);
    std::cout << "Player added: " << name << "\n";
    return true;
}

// Starts the game by assigning territories, cards, and reinforcement armies
bool GameEngine::gameStart() {
    if (getState() != State::PlayersAdded) {
        std::cout << "Cannot start the game in the current state.\n";
        return false;
    }
    if (players->size() < 2 || players->size() > 6) {
        std::cout << "The game requires between 2 and 6 players.\n";
        return false;
    }
    if (gameMap == nullptr) {
        std::cout << "No map has been loaded.\n";
        return false;
    }
// Randomize player order
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(players->begin(), players->end(), generator);
    std::cout << "Randomized player order:\n";

    for (Player* player : *players) {
        std::cout << player->getName() << "\n";
    }
// Distribute shuffled territories equally
    std::vector<Territory*> territories = gameMap->getTerritories();
    std::shuffle(territories.begin(), territories.end(), generator);

    for (size_t i = 0; i < territories.size(); i++) {
        Player* player = (*players)[i % players->size()];
        Territory* territory = territories[i];
        player->addTerritory(territory);
        territory->setPlayer(player);
    }
    std::cout << "Territories distributed successfully.\n";
    for (Player* player : *players) {
        std::cout << player->getName()
                  << " owns "
                  << player->getTerritories().size()
                  << " territories.\n";
    }
// Give players 2 cards each
    for (Player* player : *players) {
    Card* card1 = deck->draw(player->getHand());
    Card* card2 = deck->draw(player->getHand());
    if (card1 == nullptr || card2 == nullptr) {
        std::cout << "Failed to give 2 cards to " << player->getName() << "\n";
        return false;
    }
    std::cout << "2 cards given to " << player->getName() << "\n";
}

// Give players 50 reinforcement armies each
    reinforcementPools->assign(players->size(), 50);
    for (size_t i = 0; i < players->size(); i++) {
        std::cout << (*players)[i]->getName() 
                  << " received "
                  << (*reinforcementPools)[i]
                  << " reinforcement armies.\n";
    }

    transition(State::AssignReinforcement);
    std::cout << "Game started successfully.\n";
    return true; 
}

// Print the current state of the engine
std::ostream& operator<<(std::ostream& out, const GameEngine& engine) {
    out << "GameEngine state: " << engine.getStateName();
    return out;
}

// Startup phase which reads and executes commands until game starts
void GameEngine::startupPhase(CommandProcessor* processor) {
    std::cout << "Game Startup Phase\n";
    while (getState() != State::AssignReinforcement) {
        std::cout << "\nEnter command: ";
        Command* command = processor->getCommand();
        if (command == nullptr) {
            break;
        }
        // Validate command
        if (!processor->validate(command, this)) {
            std::cout << command->getEffect() << "\n";
            continue;
        }
        // Get command name
        std::stringstream ss(command->getCommand());
        std::string name;
        std::string argument;
        ss >> name >> argument;
        // Execute the matching command
        bool success = false;
        if (name == "loadmap") {
            success = loadMap(argument);
        }
        else if (name == "validatemap") {
            success = validateMap();
        }
        else if (name == "addplayer") {
            success = addPlayer(argument);
        }
        else if (name == "gamestart") {
            success = gameStart();
        }
        if (success) {
            command->saveEffect("Command executed successfully.");
        }
        else {
            command->saveEffect("Command execution failed.");
        }

        std::cout << command->getEffect() << "\n";
    }
}

///////////

// Reinforcement phase

// Armies a player earns this turn
int GameEngine::calculateReinforcement(Player* player){
    // integer division already rounds down
    int armies = static_cast<int>(player->getTerritories().size()) / 3;

    // continent bonus: the player must own every territory of the continent
    for (Continent* continent : gameMap->getContinents()) {
        std::vector<Territory*> territories = continent->getTerritories();
        if (territories.empty()) continue;

        bool ownsAll = true;
        for (Territory* t : territories) {
            if (t->getPlayer() != player) {
                ownsAll = false;
                break;
            }
        }
        if (ownsAll) {
            armies += continent->getControlBonus();
            std::cout << "player " << player->getName() << " has total control of continent " << continent->getName()
            		<< " with bonus " << continent->getControlBonus() << std::endl;
        }
    }

    // minimum of 3 per turn
    if (armies < 3) armies = 3;
    return armies;
}

// Gives every player their armies for the turn, then moves to the next state
void GameEngine::reinforcementPhase() {
    std::cout << "Reinforcement phase" << std::endl;
    for (size_t i = 0; i < players->size(); i++) {
        Player* p = (*players)[i];
        int earned = calculateReinforcement(p);
        (*reinforcementPools)[i] += earned;
        std::cout << p->getName() << " earns " << earned
                  << " armies (pool: " << (*reinforcementPools)[i] << ")\n";
    }
    transition(State::IssueOrders);
}

//////////////////////
// Issue Order phase

void GameEngine::issueOrdersPhase(){

	std::cout << "Issue Orders Phase" << std::endl;
	std::cout << "type: \"commit\" to end turn" << std::endl;
	std::vector<Player*>::iterator it;
	std::string playerInput;
	int currentIndex=0;

	for(it = players->begin(); it != players->end(); it++){
		std::cout << "turn of player: " << (*it)->getName() << std::endl;
		std::cout << "enter order: ";
		std::cin >> playerInput;
		std::cout <<  "\ninfo entered: " << playerInput << std::endl;




		while(playerInput != "commit" && (*reinforcementPools)[currentIndex] > 0){

			if((*reinforcementPools)[currentIndex] > 0 && playerInput != "deploy"){
				std::cout << "please deploy all available armies before entering another order" << std::endl;
				std::cout << "enter order: ";
				std::cin >> playerInput;
			}
			else{
				(*it)->issueOrder(playerInput);
				std::cout << "enter order: " << std::endl;
				std::cin >> playerInput;
			}



		}

		currentIndex++;
	}

}


//////////////////////

// Getter methods
Map* GameEngine::getMap() const {
    return gameMap;
}

std::vector<Player*> GameEngine::getPlayers() const {
    return *players;
}

Deck* GameEngine::getDeck() const {
    return deck;
}

int GameEngine::getReinforcementPool(Player* player) const {
    for (size_t i = 0; i < players->size(); i++) {
        if ((*players)[i] == player) {
            return (*reinforcementPools)[i];
        }
    }

    return -1;
}
