#include "Orders.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Create a territory with armies and give it to a player
Territory* makeTerritory(string name, int armies, Player* owner) {
    Territory* territory = new Territory();
    territory->setTerritoryName(name);
    territory->setArmies(armies);
    owner->addTerritory(territory);
    return territory;
}

// Make two territories neighbours of each other
void connect(Territory* a, Territory* b) {
    a->addNeighbor(b);
    b->addNeighbor(a);
}

// Print every territory with its owner and armies
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

// Execute an order and print it with its effect
void run(Order& order) {
    order.execute();
    cout << "  " << order << endl;
}

// Demo for Part 4, order execution with the real game rules
void testOrderExecution() {
    srand(static_cast<unsigned int>(time(NULL)));

    cout << "===== Part 4: Order execution =====" << endl << endl;

    // Players, deck and a small map
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

    // 1 Every order is validated when executed, invalid ones do nothing
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

    // Valid orders of each simple kind
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

    // 2 and 3 Conquest transfers ownership and gives one card per turn
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

    // 4 Negotiate blocks attacks between the two players for the turn
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

    // 5 Blockade doubles armies and gives the territory to the Neutral player
    cout << "5. Blockade transfers ownership to the Neutral player" << endl;
    Blockade blockade(alice, ontario);
    run(blockade);
    cout << "  Neutral player owns " << Order::getNeutralPlayer()->getTerritories().size() << " territory" << endl;
    showMap(allTerritories);

    // 6 A player issues one order of every kind into its list, the engine executes them
    cout << "6. All orders issued by a player and executed by the game engine" << endl;
    OrdersList* aliceOrders = alice->getOrders();
    aliceOrders->addOrder(new Advance(alice, 2, nunavut, quebec));
    aliceOrders->addOrder(new Bomb(alice, alberta));
    aliceOrders->addOrder(new Deploy(alice, 4, manitoba));
    aliceOrders->addOrder(new Airlift(alice, 1, manitoba, quebec));
    aliceOrders->addOrder(new Negotiate(alice, charlie));
    aliceOrders->addOrder(new Blockade(alice, nunavut));
    cout << "  Alice's list as issued" << endl << *aliceOrders;

    // Deploy orders first, then the rest in list order
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

    // Free everything, territories belong to this demo, the Map in the real game
    delete alice;
    delete bob;
    delete charlie;
    for (int i = 0; i < static_cast<int>(allTerritories.size()); i++) {
        delete allTerritories[i];
    }
    delete deck;
    Order::cleanup();
}

// Entry point
int main() {
    testOrderExecution();
    return 0;
}
