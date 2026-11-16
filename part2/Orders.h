#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>
#include <string>
#include <vector>

#include "LoggingObserver.h"

// Forward declarations
class Player;
class Territory;
class Deck;


// Order abstract parent of every order
class Order : public Subject, public ILoggable {
public:
    Order();
    Order(Player* issuer);
    Order(const Order& other);
    virtual ~Order();
    Order& operator=(const Order& other);

    virtual bool validate() const = 0;
    virtual void execute() = 0;
    virtual Order* clone() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string describe() const = 0;

    bool isExecuted() const;
    std::string getEffect() const;
    Player* getIssuer() const;

    // Log line
    std::string stringToLog();

    friend std::ostream& operator<<(std::ostream& out, const Order& order);

    // Shared game state
    static void setDeck(Deck* deck);
    static Player* getNeutralPlayer();
    static void endTurn();
    static void cleanup();

protected:
    void setEffect(std::string effectText);
    void setInvalid(std::string reason);

    // Why invalid, empty if valid
    virtual std::string invalidReason() const = 0;

    // Helpers
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
    Player* issuer;

    static Deck* deck;
    static Player* neutralPlayer;
    static std::vector<Player*>* truceFirst;
    static std::vector<Player*>* truceSecond;
    static std::vector<Player*>* conqueredThisTurn;
    static void createLists();
};

// Deploy armies onto own territory
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
    Territory* targetTerritory;
};

// Advance move to or attack a neighbour
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
    Territory* sourceTerritory;
    Territory* targetTerritory;
};

// Bomb halve armies on an adjacent enemy territory
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
    Territory* targetTerritory;
};

// Blockade double armies, territory goes Neutral
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
    Territory* targetTerritory;
};

// Airlift move armies between own territories
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
    Territory* sourceTerritory;
    Territory* targetTerritory;
};

// Negotiate no attacks between two players this turn
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
    Player* targetPlayer;
};

// OrdersList a player's orders
class OrdersList : public Subject, public ILoggable {
public:
    OrdersList();
    OrdersList(const OrdersList& other);
    ~OrdersList();
    OrdersList& operator=(const OrdersList& other);

    void addOrder(Order* order);
    bool remove(int index);
    bool move(int from, int to);
    int size() const;
    Order* get(int index) const;

    // Log line
    std::string stringToLog();

    friend std::ostream& operator<<(std::ostream& out, const OrdersList& list);

private:
    void deleteAllOrders();
    bool isValidIndex(int index) const;

    std::vector<Order*>* orders;
};

#endif