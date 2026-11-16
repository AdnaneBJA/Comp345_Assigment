#include "Orders.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"
#include "GameEngine.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Create a territory for a player
Territory* makeTerritory(string name, int armies, Player* owner) {
    Territory* territory = new Territory();
    territory->setTerritoryName(name);
    territory->setArmies(armies);
    owner->addTerritory(territory);
    return territory;
}

// Connect two territories
void connect(Territory* a, Territory* b) {
    a->addNeighbor(b);
    b->addNeighbor(a);
}

// Print map
void showMap(const vector<Territory*>& territories) {
    for (int i = 0; i < static_cast<int>(territories.size()); i++) {
        Territory* territory = territories[i];
        string owner = "nobody";
        if (territory->getPlayer() != NULL) {
            owner = territory->getPlayer()->getName();
        }
        cout << "    " << territory->getName() << " owned by " << owner
            << ", armies " << territory->getArmies() << endl;
    }
    cout << endl;
}

// Execute and print an order
void run(Order& order) {
    order.execute();
    cout << "  " << order << endl;
}

// Part 4 demo
void testOrderExecution() {
    srand(static_cast<unsigned int>(time(NULL)));

    cout << "----- Part 4: Order execution -----" << endl << endl;

    // Setup
    Player* alice = new Player("Alice");
    Player* bob = new Player("Bob");
    Player* charlie = new Player("Charlie");

    Deck* deck = new Deck();
    for (int i = 0; i < 2; i++) {
        deck->addCard(new Card(CardType::Bomb));
        deck->addCard(new Card(CardType::Airlift));
        deck->addCard(new Card(CardType::Blockade));
        deck->addCard(new Card(CardType::Diplomacy));
        deck->addCard(new Card(CardType::Reinforcement));
    }
    Order::setDeck(deck);

    Territory* quebec = makeTerritory("Quebec", 10, alice);
    Territory* ontario = makeTerritory("Ontario", 5, alice);
    Territory* manitoba = makeTerritory("Manitoba", 6, bob);
    Territory* nunavut = makeTerritory("Nunavut", 1, bob);
    Territory* alberta = makeTerritory("Alberta", 20, bob);
    Territory* britishColumbia = makeTerritory("BritishColumbia", 6, charlie);

    connect(quebec, ontario);
    connect(quebec, nunavut);
    connect(ontario, manitoba);
    connect(manitoba, nunavut);
    connect(manitoba, alberta);
    connect(alberta, britishColumbia);

    vector<Territory*> allTerritories;
    allTerritories.push_back(quebec);
    allTerritories.push_back(ontario);
    allTerritories.push_back(manitoba);
    allTerritories.push_back(nunavut);
    allTerritories.push_back(alberta);
    allTerritories.push_back(britishColumbia);

    cout << "Starting map" << endl;
    cout << "  Neighbours Quebec-Ontario, Quebec-Nunavut, Ontario-Manitoba," << endl;
    cout << "  Manitoba-Nunavut, Manitoba-Alberta, Alberta-BritishColumbia" << endl;
    showMap(allTerritories);

    // 1 Validation
    cout << "1. Invalid orders are rejected when executed" << endl;
    Deploy badDeploy(alice, 5, manitoba);
    run(badDeploy);
    Advance badAdvanceSource(alice, 3, manitoba, nunavut);
    run(badAdvanceSource);
    Advance badAdvanceAdjacent(alice, 3, quebec, alberta);
    run(badAdvanceAdjacent);
    Bomb badBombOwn(alice, ontario);
    run(badBombOwn);
    Bomb badBombFar(alice, britishColumbia);
    run(badBombFar);
    Blockade badBlockade(alice, manitoba);
    run(badBlockade);
    Airlift badAirlift(alice, 2, quebec, manitoba);
    run(badAirlift);
    Negotiate badNegotiate(alice, alice);
    run(badNegotiate);
    cout << "  Map unchanged" << endl;
    showMap(allTerritories);

    // Valid orders
    cout << "   Valid orders change the map" << endl;
    Deploy deploy(alice, 5, quebec);
    run(deploy);
    Advance moveOwn(alice, 3, quebec, ontario);
    run(moveOwn);
    Airlift airlift(alice, 2, ontario, quebec);
    run(airlift);
    Bomb bomb(alice, manitoba);
    run(bomb);
    showMap(allTerritories);

    // 2 and 3 Conquest and card
    cout << "2. Advance into an enemy territory, the winner takes it" << endl;
    cout << "3. A card is given for a conquest, only one per turn" << endl;
    cout << "  Alice has " << alice->getHand()->getCards().size() << " cards" << endl;
    Advance conquerNunavut(alice, 14, quebec, nunavut);
    run(conquerNunavut);
    cout << "  Alice has " << alice->getHand()->getCards().size() << " cards" << endl;
    manitoba->setArmies(1);
    Advance conquerManitoba(alice, 10, nunavut, manitoba);
    run(conquerManitoba);
    cout << "  Alice has " << alice->getHand()->getCards().size() << " cards, still one for this turn" << endl;
    Advance failedAttack(charlie, 2, britishColumbia, alberta);
    run(failedAttack);
    cout << "  BritishColumbia keeps its owner after a failed attack" << endl;
    showMap(allTerritories);

    // 4 Negotiate
    cout << "4. Negotiate prevents attacks between two players" << endl;
    Negotiate truce(bob, alice);
    run(truce);
    Advance blockedAttack(alice, 1, manitoba, alberta);
    run(blockedAttack);
    Bomb blockedBomb(bob, manitoba);
    run(blockedBomb);
    Order::endTurn();
    cout << "  New turn, the truce is over" << endl;
    Advance allowedAttack(alice, 1, manitoba, alberta);
    run(allowedAttack);
    cout << endl;

    // 5 Blockade
    cout << "5. Blockade transfers ownership to the Neutral player" << endl;
    Blockade blockade(alice, ontario);
    run(blockade);
    cout << "  Neutral player owns " << Order::getNeutralPlayer()->getTerritories().size() << " territory" << endl;
    showMap(allTerritories);

    // 6 Issue and execute all orders
    cout << "6. All orders issued by a player and executed by the game engine" << endl;
    OrdersList* aliceOrders = alice->getOrders();
    aliceOrders->addOrder(new Advance(alice, 2, nunavut, quebec));
    aliceOrders->addOrder(new Bomb(alice, alberta));
    aliceOrders->addOrder(new Deploy(alice, 4, manitoba));
    aliceOrders->addOrder(new Airlift(alice, 1, manitoba, quebec));
    aliceOrders->addOrder(new Negotiate(alice, charlie));
    aliceOrders->addOrder(new Blockade(alice, nunavut));
    cout << "  Alice's list as issued" << endl << *aliceOrders;

    // Deploys first
    cout << "  Execution, deploys first" << endl;
    for (int i = 0; i < aliceOrders->size(); i++) {
        if (aliceOrders->get(i)->getName() == "Deploy") {
            aliceOrders->get(i)->execute();
            cout << "  " << *aliceOrders->get(i) << endl;
            aliceOrders->remove(i);
            i--;
        }
    }
    while (aliceOrders->size() > 0) {
        aliceOrders->get(0)->execute();
        cout << "  " << *aliceOrders->get(0) << endl;
        aliceOrders->remove(0);
    }
    cout << endl << "Final map" << endl;
    showMap(allTerritories);

    // Cleanup
    delete alice;
    delete bob;
    delete charlie;
    for (int i = 0; i < static_cast<int>(allTerritories.size()); i++) {
        delete allTerritories[i];
    }
    delete deck;
}

// 7 Orders issued by the players of a real game and executed by the GameEngine
void testOrdersWithGameEngine() {
    cout << "7. Orders issued by players and executed by the real GameEngine" << endl;

    // New turn so nothing from the earlier demo carries over
    Order::endTurn();

    GameEngine engine;
    if (!engine.loadMap("maps/canada.map") || !engine.validateMap()) {
        cout << "  Could not load maps/canada.map, section skipped" << endl;
        return;
    }
    engine.addPlayer("Alice");
    engine.addPlayer("Bob");
    if (!engine.gameStart()) {
        cout << "  Could not start the game, section skipped" << endl;
        return;
    }

    vector<Player*> players = *engine.getPlayers();
    Player* first = players[0];
    Player* second = players[1];

    // A territory of the first player next to an enemy, and another territory of its own
    Territory* border = NULL;
    Territory* enemy = NULL;
    vector<Territory*> owned = first->getTerritories();
    for (int i = 0; i < static_cast<int>(owned.size()) && border == NULL; i++) {
        vector<Territory*> neighbors = owned[i]->getNeighbors();
        for (int j = 0; j < static_cast<int>(neighbors.size()); j++) {
            if (neighbors[j]->getPlayer() != first) {
                border = owned[i];
                enemy = neighbors[j];
                break;
            }
        }
    }
    if (border == NULL || owned.size() < 2) {
        cout << "  No border territory found, section skipped" << endl;
        return;
    }
    Territory* other = owned[0];
    if (other == border) {
        other = owned[1];
    }

    // Issued in mixed order, the engine runs the deploy first
    first->getOrders()->addOrder(new Advance(first, 5, border, enemy));
    first->getOrders()->addOrder(new Deploy(first, 10, border));
    first->getOrders()->addOrder(new Bomb(first, enemy));
    first->getOrders()->addOrder(new Airlift(first, 2, border, other));
    first->getOrders()->addOrder(new Blockade(first, other));
    second->getOrders()->addOrder(new Negotiate(second, first));

    cout << endl << "  " << first->getName() << "'s orders" << endl << *first->getOrders();
    cout << "  " << second->getName() << "'s orders" << endl << *second->getOrders() << endl;

    engine.transition(GameEngine::State::ExecuteOrders);
    engine.executeOrdersPhase();

    cout << endl << "  After execution" << endl;
    cout << "    " << border->getName() << " owned by " << border->getPlayer()->getName()
        << ", armies " << border->getArmies() << endl;
    cout << "    " << enemy->getName() << " owned by " << enemy->getPlayer()->getName()
        << ", armies " << enemy->getArmies() << endl;
    cout << "    " << other->getName() << " owned by " << other->getPlayer()->getName()
        << ", armies " << other->getArmies() << endl;
}

// Entry point
int main() {
    testOrderExecution();
    cout << endl;
    testOrdersWithGameEngine();
    Order::cleanup();
    return 0;
}