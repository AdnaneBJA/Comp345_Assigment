#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>
#include <string>
#include <vector>

#include "LoggingObserver.h"

// Forward declarations, Player.h includes this file so we cannot include it back
class Player;
class Territory;
class Deck;


// Order abstract parent of every order type
// An order is issued by a player and validated only when it is executed
// Executing an order notifies the observers so its effect is logged
class Order : public Subject, public ILoggable {
public:
    Order();
    Order(Player* issuer);
    Order(const Order& other);
    virtual ~Order();  // deleting an Order* also runs the subclass destructor
    Order& operator=(const Order& other);

    virtual bool validate() const = 0;
    virtual void execute() = 0;
    virtual Order* clone() const = 0;  // copy of the right subclass, used to copy an OrdersList
    virtual std::string getName() const = 0;
    virtual std::string describe() const = 0;

    bool isExecuted() const;
    std::string getEffect() const;
    Player* getIssuer() const;

    // Log line: the order and its effect
    std::string stringToLog();

    friend std::ostream& operator<<(std::ostream& out, const Order& order);

    // Game wide state shared by all orders, managed by the game engine
    static void setDeck(Deck* deck);       // deck used to give a card after a conquest
    static Player* getNeutralPlayer();     // created the first time it is needed
    static void endTurn();                 // clears truces and conquests of the turn
    static void cleanup();                 // frees the neutral player and the shared lists

protected:
    void setEffect(std::string effectText);
    void setInvalid(std::string reason);

    // Why the order is invalid, empty if it is valid
    virtual std::string invalidReason() const = 0;

    // Helpers shared by the subclasses
    static bool isAdjacent(Territory* a, Territory* b);
    static bool hasTruce(Player* a, Player* b);
    static void addTruce(Player* a, Player* b);
    static void transferTerritory(Territory* territory, Player* newOwner);
    static std::string giveConquestCard(Player* player);
    static std::string nameOf(Territory* territory);
    static std::string nameOf(Player* player);

private:
    bool* executed;
    std::string* effect;
    Player* issuer;  // not owned, the player who issued the order

    static Deck* deck;                              // not owned
    static Player* neutralPlayer;                   // owned, freed by cleanup
    static std::vector<Player*>* truceFirst;        // truce pairs for this turn
    static std::vector<Player*>* truceSecond;
    static std::vector<Player*>* conqueredThisTurn; // players who already got their card
    static void createLists();
};

// Deploy armies from the reinforcement pool onto one of the player's territories.
class Deploy : public Order {
public:
    Deploy();
    Deploy(Player* issuer, int armies, Territory* targetTerritory);
    Deploy(const Deploy& other);
    ~Deploy();
    Deploy& operator=(const Deploy& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Deploy& order);

protected:
    std::string invalidReason() const;

private:
    int* armies;
    Territory* targetTerritory;  // not owned, belongs to the Map
};

// Advance move armies to a neighbouring territory, attack if enemy-owned
class Advance : public Order {
public:
    Advance();
    Advance(Player* issuer, int armies, Territory* sourceTerritory, Territory* targetTerritory);
    Advance(const Advance& other);
    ~Advance();
    Advance& operator=(const Advance& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Advance& order);

protected:
    std::string invalidReason() const;

private:
    int* armies;
    Territory* sourceTerritory;  // not owned
    Territory* targetTerritory;  // not owned
};

// Bomb remove half the armies on an enemy territory next to the player, bomb card.
class Bomb : public Order {
public:
    Bomb();
    Bomb(Player* issuer, Territory* targetTerritory);
    Bomb(const Bomb& other);
    ~Bomb();
    Bomb& operator=(const Bomb& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Bomb& order);

protected:
    std::string invalidReason() const;

private:
    Territory* targetTerritory;  // not owned
};

// Blockade double armies on own territory, then it goes to the Neutral player, blockade card.
class Blockade : public Order {
public:
    Blockade();
    Blockade(Player* issuer, Territory* targetTerritory);
    Blockade(const Blockade& other);
    ~Blockade();
    Blockade& operator=(const Blockade& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Blockade& order);

protected:
    std::string invalidReason() const;

private:
    Territory* targetTerritory;  // not owned
};

// Airlift move armies between any two own territories, airlift card.
class Airlift : public Order {
public:
    Airlift();
    Airlift(Player* issuer, int armies, Territory* sourceTerritory, Territory* targetTerritory);
    Airlift(const Airlift& other);
    ~Airlift();
    Airlift& operator=(const Airlift& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Airlift& order);

protected:
    std::string invalidReason() const;

private:
    int* armies;
    Territory* sourceTerritory;  // not owned
    Territory* targetTerritory;  // not owned
};

// Negotiate no attacks between two players this turn, diplomacy card.
class Negotiate : public Order {
public:
    Negotiate();
    Negotiate(Player* issuer, Player* targetPlayer);
    Negotiate(const Negotiate& other);
    ~Negotiate();
    Negotiate& operator=(const Negotiate& other);

    bool validate() const;
    void execute();
    Order* clone() const;
    std::string getName() const;
    std::string describe() const;

    friend std::ostream& operator<<(std::ostream& out, const Negotiate& order);

protected:
    std::string invalidReason() const;

private:
    Player* targetPlayer;  // not owned
};

// OrdersList a player's orders for the turn. Owns its orders and deletes them.
// Adding an order notifies the observers so the new order is logged.
class OrdersList : public Subject, public ILoggable {
public:
    OrdersList();
    OrdersList(const OrdersList& other);
    ~OrdersList();
    OrdersList& operator=(const OrdersList& other);

    void addOrder(Order* order);
    bool remove(int index);         // false if index doesn't exist
    bool move(int from, int to);    // false if either index doesn't exist
    int size() const;
    Order* get(int index) const;    // NULL if index doesn't exist

    // Log line: the last order added
    std::string stringToLog();

    friend std::ostream& operator<<(std::ostream& out, const OrdersList& list);

private:
    void deleteAllOrders();
    bool isValidIndex(int index) const;

    std::vector<Order*>* orders;
};

#endif
