#include "Orders.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"

#include <cstdlib>

using namespace std;


// Order

// Shared state, starts empty
Deck* Order::deck = NULL;
Player* Order::neutralPlayer = NULL;
vector<Player*>* Order::truceFirst = NULL;
vector<Player*>* Order::truceSecond = NULL;
vector<Player*>* Order::conqueredThisTurn = NULL;

// Default constructor not executed yet, no issuer
Order::Order() {
    this->executed = new bool(false);
    this->effect = new string("");
    this->issuer = NULL;
}

// Constructor with the player who issued the order
Order::Order(Player* issuer) {
    this->executed = new bool(false);
    this->effect = new string("");
    this->issuer = issuer;
}

// Copy constructor new values, copied from other, deep copy
// The issuer is shared, the order does not own the player
Order::Order(const Order& other) {
    this->executed = new bool(*other.executed);
    this->effect = new string(*other.effect);
    this->issuer = other.issuer;
}

// Destructor free members, set to NULL for no dangling pointers
// The issuer is not deleted, the game owns the players
Order::~Order() {
    delete executed;
    executed = NULL;
    delete effect;
    effect = NULL;
    issuer = NULL;
}

// Assignment operator copy the values into our own memory
Order& Order::operator=(const Order& other) {
    if (this != &other) {
        *this->executed = *other.executed;
        *this->effect = *other.effect;
        this->issuer = other.issuer;
    }
    return *this;
}

// Getter executed
bool Order::isExecuted() const {
    return *executed;
}

// Getter effect
string Order::getEffect() const {
    return *effect;
}

// Getter issuer
Player* Order::getIssuer() const {
    return issuer;
}

// Mark as executed + save effect
void Order::setEffect(string effectText) {
    *this->executed = true;
    *this->effect = effectText;
}

// Save why the order was rejected, it stays not executed
void Order::setInvalid(string reason) {
    *this->executed = false;
    *this->effect = "INVALID, " + reason;
}

// Print order, plus effect if it ran or was rejected
ostream& operator<<(ostream& out, const Order& order) {
    out << order.getName() << " order: " << order.describe();
    if (order.getEffect() != "") {
        out << " | Effect: " << order.getEffect();
    }
    return out;
}

// Set the deck used for conquest cards
void Order::setDeck(Deck* newDeck) {
    deck = newDeck;
}

// Neutral player, created the first time a blockade needs it
Player* Order::getNeutralPlayer() {
    if (neutralPlayer == NULL) {
        neutralPlayer = new Player("Neutral");
    }
    return neutralPlayer;
}

// Create the shared lists if they do not exist yet
void Order::createLists() {
    if (truceFirst == NULL) {
        truceFirst = new vector<Player*>();
        truceSecond = new vector<Player*>();
        conqueredThisTurn = new vector<Player*>();
    }
}

// End of turn, truces end and players can earn a new card
void Order::endTurn() {
    createLists();
    truceFirst->clear();
    truceSecond->clear();
    conqueredThisTurn->clear();
}

// Free the neutral player and the shared lists, call once at the end of the game
void Order::cleanup() {
    delete neutralPlayer;
    neutralPlayer = NULL;
    delete truceFirst;
    truceFirst = NULL;
    delete truceSecond;
    truceSecond = NULL;
    delete conqueredThisTurn;
    conqueredThisTurn = NULL;
    deck = NULL;
}

// True if b is a neighbour of a
bool Order::isAdjacent(Territory* a, Territory* b) {
    vector<Territory*> neighbors = a->getNeighbors();
    for (int i = 0; i < static_cast<int>(neighbors.size()); i++) {
        if (neighbors[i] == b) {
            return true;
        }
    }
    return false;
}

// True if a and b negotiated this turn, in either direction
bool Order::hasTruce(Player* a, Player* b) {
    createLists();
    for (int i = 0; i < static_cast<int>(truceFirst->size()); i++) {
        Player* first = truceFirst->at(i);
        Player* second = truceSecond->at(i);
        if ((first == a && second == b) || (first == b && second == a)) {
            return true;
        }
    }
    return false;
}

// Record a truce between a and b for this turn
void Order::addTruce(Player* a, Player* b) {
    createLists();
    truceFirst->push_back(a);
    truceSecond->push_back(b);
}

// Change the owner of a territory in both players' lists
void Order::transferTerritory(Territory* territory, Player* newOwner) {
    Player* oldOwner = territory->getPlayer();
    if (oldOwner != NULL) {
        oldOwner->removeTerritory(territory);
    }
    newOwner->addTerritory(territory);
}

// Give one card to a player who conquered, at most one per turn
string Order::giveConquestCard(Player* player) {
    createLists();
    for (int i = 0; i < static_cast<int>(conqueredThisTurn->size()); i++) {
        if (conqueredThisTurn->at(i) == player) {
            return " No card, " + player->getName() + " already got one this turn.";
        }
    }
    conqueredThisTurn->push_back(player);
    if (deck == NULL) {
        return " No card, no deck was set.";
    }
    Card* card = deck->draw(player->getHand());
    if (card == NULL) {
        return " No card, the deck is empty.";
    }
    return " " + player->getName() + " receives a card.";
}

// Territory name or a placeholder if missing
string Order::nameOf(Territory* territory) {
    if (territory == NULL) {
        return "unknown territory";
    }
    return territory->getName();
}

// Player name or a placeholder if missing
string Order::nameOf(Player* player) {
    if (player == NULL) {
        return "unknown player";
    }
    return player->getName();
}

//Deploy

// Default constructor invalid order
Deploy::Deploy() {
    this->armies = new int(0);
    this->targetTerritory = NULL;
}

// Constructor
Deploy::Deploy(Player* issuer, int armies, Territory* targetTerritory) : Order(issuer) {
    this->armies = new int(armies);
    this->targetTerritory = targetTerritory;
}

// Copy constructor, the territory is shared not copied
Deploy::Deploy(const Deploy& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->targetTerritory = other.targetTerritory;
}

// Destructor free members, territory belongs to the Map so it is not deleted
Deploy::~Deploy() {
    delete armies;
    armies = NULL;
    targetTerritory = NULL;
}

// Assignment operator
Deploy& Deploy::operator=(const Deploy& other) {
    if (this != &other) {
        Order::operator=(other);
        *this->armies = *other.armies;
        this->targetTerritory = other.targetTerritory;
    }
    return *this;
}

// Rules, target must belong to the issuer
string Deploy::invalidReason() const {
    if (getIssuer() == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (*armies <= 0) {
        return "must deploy at least 1 army.";
    }
    if (targetTerritory->getPlayer() != getIssuer()) {
        return targetTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    return "";
}

// Valid if no rule is broken
bool Deploy::validate() const {
    return invalidReason() == "";
}

// Validate, then add the armies to the territory
void Deploy::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }
    targetTerritory->setArmies(targetTerritory->getArmies() + *armies);
    setEffect("Deployed " + to_string(*armies) + " armies to " + targetTerritory->getName()
        + ", now " + to_string(targetTerritory->getArmies()) + ".");
}

// Copy of this order
Order* Deploy::clone() const {
    return new Deploy(*this);
}

// Order type name
string Deploy::getName() const {
    return "Deploy";
}

// Short description
string Deploy::describe() const {
    return nameOf(getIssuer()) + " deploys " + to_string(*armies) + " armies to " + nameOf(targetTerritory);
}

// Print, same as Order
ostream& operator<<(ostream& out, const Deploy& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Advance

// Default constructor invalid order
Advance::Advance() {
    this->armies = new int(0);
    this->sourceTerritory = NULL;
    this->targetTerritory = NULL;
}

// Constructor
Advance::Advance(Player* issuer, int armies, Territory* sourceTerritory, Territory* targetTerritory) : Order(issuer) {
    this->armies = new int(armies);
    this->sourceTerritory = sourceTerritory;
    this->targetTerritory = targetTerritory;
}

// Copy constructor, territories are shared not copied
Advance::Advance(const Advance& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->sourceTerritory = other.sourceTerritory;
    this->targetTerritory = other.targetTerritory;
}

// Destructor free members, territories belong to the Map so they are not deleted
Advance::~Advance() {
    delete armies;
    armies = NULL;
    sourceTerritory = NULL;
    targetTerritory = NULL;
}

// Assignment operator
Advance& Advance::operator=(const Advance& other) {
    if (this != &other) {
        Order::operator=(other);
        *this->armies = *other.armies;
        this->sourceTerritory = other.sourceTerritory;
        this->targetTerritory = other.targetTerritory;
    }
    return *this;
}

// Rules, own source, adjacent target, no truce with the target's owner
string Advance::invalidReason() const {
    if (getIssuer() == NULL || sourceTerritory == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (*armies <= 0) {
        return "must move at least 1 army.";
    }
    if (sourceTerritory->getPlayer() != getIssuer()) {
        return sourceTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    if (!isAdjacent(sourceTerritory, targetTerritory)) {
        return targetTerritory->getName() + " is not adjacent to " + sourceTerritory->getName() + ".";
    }
    if (sourceTerritory->getArmies() <= 0) {
        return sourceTerritory->getName() + " has no armies left to move.";
    }
    Player* defender = targetTerritory->getPlayer();
    if (defender != getIssuer() && hasTruce(getIssuer(), defender)) {
        return getIssuer()->getName() + " and " + nameOf(defender) + " negotiated, no attack this turn.";
    }
    return "";
}

// Valid if no rule is broken
bool Advance::validate() const {
    return invalidReason() == "";
}

// Validate, then move armies or simulate a battle
void Advance::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }

    // Can only send what the source still has
    int moving = *armies;
    if (moving > sourceTerritory->getArmies()) {
        moving = sourceTerritory->getArmies();
    }

    // Own territory, simple move
    if (targetTerritory->getPlayer() == getIssuer()) {
        sourceTerritory->setArmies(sourceTerritory->getArmies() - moving);
        targetTerritory->setArmies(targetTerritory->getArmies() + moving);
        setEffect("Moved " + to_string(moving) + " armies from " + sourceTerritory->getName()
            + " to " + targetTerritory->getName() + ".");
        return;
    }
    /////

    // Enemy territory, battle
    int defenders = targetTerritory->getArmies();
    sourceTerritory->setArmies(sourceTerritory->getArmies() - moving);

    // Each attacker has 60 percent chance to kill a defender
    int defendersKilled = 0;
    for (int i = 0; i < moving; i++) {
        if (rand() % 100 < 60) {
            defendersKilled++;
        }
    }
    // Each defender has 70 percent chance to kill an attacker
    int attackersKilled = 0;
    for (int i = 0; i < defenders; i++) {
        if (rand() % 100 < 70) {
            attackersKilled++;
        }
    }

    int defendersLeft = defenders - defendersKilled;
    if (defendersLeft < 0) {
        defendersLeft = 0;
    }
    int attackersLeft = moving - attackersKilled;
    if (attackersLeft < 0) {
        attackersLeft = 0;
    }

    string battle = "Attacked " + targetTerritory->getName() + " from " + sourceTerritory->getName()
        + " with " + to_string(moving) + " vs " + to_string(defenders) + ". ";

    // Conquered, survivors occupy the territory
    if (defendersLeft == 0 && attackersLeft > 0) {
        Player* oldOwner = targetTerritory->getPlayer();
        transferTerritory(targetTerritory, getIssuer());
        targetTerritory->setArmies(attackersLeft);
        string card = giveConquestCard(getIssuer());
        setEffect(battle + getIssuer()->getName() + " conquered it from " + nameOf(oldOwner)
            + ", " + to_string(attackersLeft) + " armies occupy it." + card);
        return;
    }

    // Not conquered, survivors go back to the source
    targetTerritory->setArmies(defendersLeft);
    sourceTerritory->setArmies(sourceTerritory->getArmies() + attackersLeft);
    setEffect(battle + "Attack failed, " + to_string(defendersLeft) + " defenders left, "
        + to_string(attackersLeft) + " attackers return.");
}

// Copy of this order
Order* Advance::clone() const {
    return new Advance(*this);
}

// Order type name
string Advance::getName() const {
    return "Advance";
}

// Short description
string Advance::describe() const {
    return nameOf(getIssuer()) + " advances " + to_string(*armies) + " armies from "
        + nameOf(sourceTerritory) + " to " + nameOf(targetTerritory);
}

// Print same as order
ostream& operator<<(ostream& out, const Advance& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Bomb

// Default constructor invalid order
Bomb::Bomb() {
    this->targetTerritory = NULL;
}

// Constructor
Bomb::Bomb(Player* issuer, Territory* targetTerritory) : Order(issuer) {
    this->targetTerritory = targetTerritory;
}

// Copy constructor, territory shared
Bomb::Bomb(const Bomb& other) : Order(other) {
    this->targetTerritory = other.targetTerritory;
}

// Destructor, territory belongs to the Map so it is not deleted
Bomb::~Bomb() {
    targetTerritory = NULL;
}

// Assignment operator
Bomb& Bomb::operator=(const Bomb& other) {
    if (this != &other) {
        Order::operator=(other);
        this->targetTerritory = other.targetTerritory;
    }
    return *this;
}

// Rules, enemy target adjacent to one of the issuer's territories, no truce
string Bomb::invalidReason() const {
    if (getIssuer() == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (targetTerritory->getPlayer() == getIssuer()) {
        return getIssuer()->getName() + " cannot bomb its own territory " + targetTerritory->getName() + ".";
    }
    bool nextToPlayer = false;
    vector<Territory*> owned = getIssuer()->getTerritories();
    for (int i = 0; i < static_cast<int>(owned.size()); i++) {
        if (isAdjacent(owned[i], targetTerritory)) {
            nextToPlayer = true;
        }
    }
    if (!nextToPlayer) {
        return targetTerritory->getName() + " is not adjacent to any territory of " + getIssuer()->getName() + ".";
    }
    if (hasTruce(getIssuer(), targetTerritory->getPlayer())) {
        return getIssuer()->getName() + " and " + nameOf(targetTerritory->getPlayer()) + " negotiated, no attack this turn.";
    }
    return "";
}

// Valid if no rule is broken
bool Bomb::validate() const {
    return invalidReason() == "";
}

// Validate, then remove half the armies
void Bomb::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }
    int before = targetTerritory->getArmies();
    targetTerritory->setArmies(before / 2);
    setEffect("Bombed " + targetTerritory->getName() + ", armies went from " + to_string(before)
        + " to " + to_string(targetTerritory->getArmies()) + ".");
}

// Copy of this order
Order* Bomb::clone() const {
    return new Bomb(*this);
}

// Order type name
string Bomb::getName() const {
    return "Bomb";
}

// Short description
string Bomb::describe() const {
    return nameOf(getIssuer()) + " bombs " + nameOf(targetTerritory);
}

// Print same as Order
ostream& operator<<(ostream& out, const Bomb& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

//Blockade

// Default constructor invalid order
Blockade::Blockade() {
    this->targetTerritory = NULL;
}

// Constructor
Blockade::Blockade(Player* issuer, Territory* targetTerritory) : Order(issuer) {
    this->targetTerritory = targetTerritory;
}

// Copy constructor, territory shared
Blockade::Blockade(const Blockade& other) : Order(other) {
    this->targetTerritory = other.targetTerritory;
}

// Destructor, territory belongs to the Map so it is not deleted
Blockade::~Blockade() {
    targetTerritory = NULL;
}

// Assignment operator
Blockade& Blockade::operator=(const Blockade& other) {
    if (this != &other) {
        Order::operator=(other);
        this->targetTerritory = other.targetTerritory;
    }
    return *this;
}

// Rules, target must belong to the issuer
string Blockade::invalidReason() const {
    if (getIssuer() == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (targetTerritory->getPlayer() != getIssuer()) {
        return targetTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    return "";
}

// Valid if no rule is broken
bool Blockade::validate() const {
    return invalidReason() == "";
}

// Validate, then double the armies and give the territory to the Neutral player
void Blockade::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }
    int before = targetTerritory->getArmies();
    targetTerritory->setArmies(before * 2);
    transferTerritory(targetTerritory, getNeutralPlayer());
    setEffect("Blockaded " + targetTerritory->getName() + ", armies doubled from " + to_string(before)
        + " to " + to_string(targetTerritory->getArmies()) + ", now owned by the Neutral player.");
}

// Copy of this order
Order* Blockade::clone() const {
    return new Blockade(*this);
}

// Order type name
string Blockade::getName() const {
    return "Blockade";
}

// Short description
string Blockade::describe() const {
    return nameOf(getIssuer()) + " blockades " + nameOf(targetTerritory);
}

// Print same as Order
ostream& operator<<(ostream& out, const Blockade& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Airlift

// Default constructor invalid order
Airlift::Airlift() {
    this->armies = new int(0);
    this->sourceTerritory = NULL;
    this->targetTerritory = NULL;
}

// Constructor
Airlift::Airlift(Player* issuer, int armies, Territory* sourceTerritory, Territory* targetTerritory) : Order(issuer) {
    this->armies = new int(armies);
    this->sourceTerritory = sourceTerritory;
    this->targetTerritory = targetTerritory;
}

// Copy constructor, territories shared
Airlift::Airlift(const Airlift& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->sourceTerritory = other.sourceTerritory;
    this->targetTerritory = other.targetTerritory;
}

// Destructor free members, territories belong to the Map so they are not deleted
Airlift::~Airlift() {
    delete armies;
    armies = NULL;
    sourceTerritory = NULL;
    targetTerritory = NULL;
}

// Assignment operator
Airlift& Airlift::operator=(const Airlift& other) {
    if (this != &other) {
        Order::operator=(other);
        *this->armies = *other.armies;
        this->sourceTerritory = other.sourceTerritory;
        this->targetTerritory = other.targetTerritory;
    }
    return *this;
}

// Rules, both territories must belong to the issuer, no adjacency needed
string Airlift::invalidReason() const {
    if (getIssuer() == NULL || sourceTerritory == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (*armies <= 0) {
        return "must airlift at least 1 army.";
    }
    if (sourceTerritory == targetTerritory) {
        return "source and target are the same territory.";
    }
    if (sourceTerritory->getPlayer() != getIssuer()) {
        return sourceTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    if (targetTerritory->getPlayer() != getIssuer()) {
        return targetTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    if (sourceTerritory->getArmies() <= 0) {
        return sourceTerritory->getName() + " has no armies left to move.";
    }
    return "";
}

// Valid if no rule is broken
bool Airlift::validate() const {
    return invalidReason() == "";
}

// Validate, then move the armies
void Airlift::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }
    int moving = *armies;
    if (moving > sourceTerritory->getArmies()) {
        moving = sourceTerritory->getArmies();
    }
    sourceTerritory->setArmies(sourceTerritory->getArmies() - moving);
    targetTerritory->setArmies(targetTerritory->getArmies() + moving);
    setEffect("Airlifted " + to_string(moving) + " armies from " + sourceTerritory->getName()
        + " to " + targetTerritory->getName() + ".");
}

// Copy of this order
Order* Airlift::clone() const {
    return new Airlift(*this);
}

// Order type name
string Airlift::getName() const {
    return "Airlift";
}

// Short description
string Airlift::describe() const {
    return nameOf(getIssuer()) + " airlifts " + to_string(*armies) + " armies from "
        + nameOf(sourceTerritory) + " to " + nameOf(targetTerritory);
}

// Print same as Order
ostream& operator<<(ostream& out, const Airlift& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Negotiate

// Default constructor invalid order
Negotiate::Negotiate() {
    this->targetPlayer = NULL;
}

// Constructor
Negotiate::Negotiate(Player* issuer, Player* targetPlayer) : Order(issuer) {
    this->targetPlayer = targetPlayer;
}

// Copy constructor, player shared
Negotiate::Negotiate(const Negotiate& other) : Order(other) {
    this->targetPlayer = other.targetPlayer;
}

// Destructor, the player belongs to the game so it is not deleted
Negotiate::~Negotiate() {
    targetPlayer = NULL;
}

// Assignment operator
Negotiate& Negotiate::operator=(const Negotiate& other) {
    if (this != &other) {
        Order::operator=(other);
        this->targetPlayer = other.targetPlayer;
    }
    return *this;
}

// Rules, target must be another player
string Negotiate::invalidReason() const {
    if (getIssuer() == NULL || targetPlayer == NULL) {
        return "missing player.";
    }
    if (targetPlayer == getIssuer()) {
        return getIssuer()->getName() + " cannot negotiate with itself.";
    }
    return "";
}

// Valid if no rule is broken
bool Negotiate::validate() const {
    return invalidReason() == "";
}

// Validate, then block attacks between the two players for this turn
void Negotiate::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        return;
    }
    addTruce(getIssuer(), targetPlayer);
    setEffect(getIssuer()->getName() + " and " + targetPlayer->getName()
        + " cannot attack each other for the rest of the turn.");
}

// Copy of this order
Order* Negotiate::clone() const {
    return new Negotiate(*this);
}

// Order type name
string Negotiate::getName() const {
    return "Negotiate";
}

// Short description
string Negotiate::describe() const {
    return nameOf(getIssuer()) + " negotiates with " + nameOf(targetPlayer);
}

// Print same as order
ostream& operator<<(ostream& out, const Negotiate& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// OrdersList

// Default constructor with empty list
OrdersList::OrdersList() {
    this->orders = new vector<Order*>();
}

// Copy constructor with a deep copy
OrdersList::OrdersList(const OrdersList& other) {
    this->orders = new vector<Order*>();
    for (int i = 0; i < other.size(); i++) {
        orders->push_back(other.get(i)->clone());
    }
}

// Destructor delete orders, then the vector
OrdersList::~OrdersList() {
    deleteAllOrders();
    delete orders;
    orders = NULL;
}

// Assignment operator delete own orders, then deep copy
OrdersList& OrdersList::operator=(const OrdersList& other) {
    if (this != &other) {
        deleteAllOrders();
        for (int i = 0; i < other.size(); i++) {
            orders->push_back(other.get(i)->clone());
        }
    }
    return *this;
}

// Add at the end
void OrdersList::add(Order* order) {
    if (order != NULL) {
        orders->push_back(order);
    }
}

// Delete order at index
bool OrdersList::remove(int index) {
    if (!isValidIndex(index)) {
        return false;
    }
    delete orders->at(index);                 // free the order
    orders->erase(orders->begin() + index);   // remove its slot
    return true;
}

// Move order from one index to another
bool OrdersList::move(int from, int to) {
    if (!isValidIndex(from) || !isValidIndex(to)) {
        return false;
    }
    Order* orderToMove = orders->at(from);
    orders->erase(orders->begin() + from);
    orders->insert(orders->begin() + to, orderToMove);
    return true;
}

// Number of orders
int OrdersList::size() const {
    return static_cast<int>(orders->size());
}

// Order at index, or NULL
Order* OrdersList::get(int index) const {
    if (!isValidIndex(index)) {
        return NULL;
    }
    return orders->at(index);
}

// Delete every order, empty the vector
void OrdersList::deleteAllOrders() {
    for (int i = 0; i < size(); i++) {
        delete orders->at(i);
    }
    orders->clear();
}

// Index in range
bool OrdersList::isValidIndex(int index) const {
    if (index < 0 || index >= size()) {
        return false;
    }
    return true;
}

// Print each order on its own line
ostream& operator<<(ostream& out, const OrdersList& list) {
    if (list.size() == 0) {
        out << "  (empty)" << endl;
        return out;
    }
    for (int i = 0; i < list.size(); i++) {
        out << "  [" << i << "] " << *list.get(i) << endl;
    }
    return out;
}
