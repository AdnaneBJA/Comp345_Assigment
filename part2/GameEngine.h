#ifndef GAMEENGINE_H
#define GAMEENGINE_H
#include <iostream>
#include <string>
#include <vector>
#include "Map.h"
#include "Player.h"
#include "Cards.h"
#include "LoggingObserver.h"

// Forward Declaration
class CommandProcessor;

// GameEngine class implementation
// Every state transition notifies the observers so the new state is logged
class GameEngine : public Subject, public ILoggable {
public:
    // All states that are possible
    enum class State {
        Start,
        MapLoaded,
        MapValidated,
        PlayersAdded,
        AssignReinforcement,
        IssueOrders,
        ExecuteOrders,
        Win,
        End
    };

    // methods
    GameEngine();
    GameEngine(const GameEngine& other);
    GameEngine& operator=(const GameEngine& other);
    ~GameEngine();

    State getState() const;
    std::string getStateName() const;
    bool isFinished() const;
    // move the engine to a new state (used with the CommandProcessor)
    void transition(State newState);

    // Log line: the current (new) state of the engine
    std::string stringToLog();

    void startupPhase(CommandProcessor* processor);
    int getReinforcementPool(Player* player) const;
    void setReinforcementPool(Player* player, int newAmount);
    bool loadMap(const std::string& filename);
    bool validateMap();
    bool addPlayer(const std::string& name);
    bool gameStart();

    int calculateReinforcement(Player* player);
    void reinforcementPhase();
    void issueOrdersPhase();
    void executeOrdersPhase();
    bool checkWinCondition();
    void checkDefeatedPlayer();
    void endingPhase(CommandProcessor* processor);

    Map* getMap() const;
    std::vector<Player*>* getPlayers() const;
    Deck* getDeck() const;
    // print the current state
    friend std::ostream& operator<<(std::ostream& out, const GameEngine& engine);

private:
    State* currentState;
    static std::string stateName(State state);
    Map* gameMap;
    std::vector<Player*>* players;
    Deck* deck;
    MapLoader* mapLoader;
    std::vector<int>* reinforcementPools;
    // store the next state in result
};

#endif
