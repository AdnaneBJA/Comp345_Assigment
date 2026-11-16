#include "Orders.h"
#include "Player.h"
#include "Map.h"
#include "Cards.h"

#include <cstdlib>

using namespace std;


// Order

// Shared state
Deck* Order::deck = NULL;
Player* Order::neutralPlayer = NULL;
vector<Player*>* Order::truceFirst = NULL;
vector<Player*>* Order::truceSecond = NULL;
vector<Player*>* Order::conqueredThisTurn = NULL;

// Default constructor
Order::Order() {
    this->executed = new bool(false);
    this->effect = new string("");
    this->issuer = NULL;
}

// Constructor
Order::Order(Player* issuer) {
    this->executed = new bool(false);
    this->effect = new string("");
    this->issuer = issuer;
}

// Copy constructor
Order::Order(const Order& other) : Subject(other), ILoggable(other) {
    this->executed = new bool(*other.executed);
    this->effect = new string(*other.effect);
    this->issuer = other.issuer;
}

// Destructor
Order::~Order() {
    delete executed;
    executed = NULL;
    delete effect;
    effect = NULL;
    issuer = NULL;
}

// Assignment operator
Order& Order::operator=(const Order& other) {
    if (this != &other) {
        Subject::operator=(other);
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

// Mark executed, save effect
void Order::setEffect(string effectText) {
    *this->executed = true;
    *this->effect = effectText;
}

// Save why rejected
void Order::setInvalid(string reason) {
    *this->executed = false;
    *this->effect = "INVALID, " + reason;
}

// Print order and effect
ostream& operator<<(ostream& out, const Order& order) {
    out << order.getName() << " order: " << order.describe();
    if (order.getEffect() != "") {
        out << " | Effect: " << order.getEffect();
    }
    return out;
}

// Log line
string Order::stringToLog() {
    return "Order executed: " + getName() + " (" + describe() + ") | Effect: " + getEffect();
}

// Set deck
void Order::setDeck(Deck* newDeck) {
    deck = newDeck;
}

// Neutral player, created when first needed
Player* Order::getNeutralPlayer() {
    if (neutralPlayer == NULL) {
        neutralPlayer = new Player("Neutral");
    }
    return neutralPlayer;
}

// Create shared lists
void Order::createLists() {
    if (truceFirst == NULL) {
        truceFirst = new vector<Player*>();
        truceSecond = new vector<Player*>();
        conqueredThisTurn = new vector<Player*>();
    }
}

// End of turn, clear truces and conquests
void Order::endTurn() {
    createLists();
    truceFirst->clear();
    truceSecond->clear();
    conqueredThisTurn->clear();
}

// Free shared state at end of game
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

// True if b neighbours a
bool Order::isAdjacent(Territory* a, Territory* b) {
    vector<Territory*> neighbors = a->getNeighbors();
    for (int i = 0; i < static_cast<int>(neighbors.size()); i++) {
        if (neighbors[i] == b) {
            return true;
        }
    }
    return false;
}

// True if a and b negotiated this turn
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

// Record a truce
void Order::addTruce(Player* a, Player* b) {
    createLists();
    truceFirst->push_back(a);
    truceSecond->push_back(b);
}

// Change territory owner
void Order::transferTerritory(Territory* territory, Player* newOwner) {
    Player* oldOwner = territory->getPlayer();
    if (oldOwner != NULL) {
        oldOwner->removeTerritory(territory);
    }
    newOwner->addTerritory(territory);
}

// One card per turn for a conquest
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

// Territory name or placeholder
string Order::nameOf(Territory* territory) {
    if (territory == NULL) {
        return "unknown territory";
    }
    return territory->getName();
}

// Player name or placeholder
string Order::nameOf(Player* player) {
    if (player == NULL) {
        return "unknown player";
    }
    return player->getName();
}

// Deploy

// Default constructor
Deploy::Deploy() {
    this->armies = new int(0);
    this->targetTerritory = NULL;
}

// Constructor
Deploy::Deploy(Player* issuer, int armies, Territory* targetTerritory) : Order(issuer) {
    this->armies = new int(armies);
    this->targetTerritory = targetTerritory;
}

// Copy constructor
Deploy::Deploy(const Deploy& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->targetTerritory = other.targetTerritory;
}

// Destructor
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

// Rules, own target
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

// Valid if no rule broken
bool Deploy::validate() const {
    return invalidReason() == "";
}

// Add armies
void Deploy::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
        return;
    }
    targetTerritory->setArmies(targetTerritory->getArmies() + *armies);
    setEffect("Deployed " + to_string(*armies) + " armies to " + targetTerritory->getName()
        + ", now " + to_string(targetTerritory->getArmies()) + ".");
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Deploy& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Advance

// Default constructor
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

// Copy constructor
Advance::Advance(const Advance& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->sourceTerritory = other.sourceTerritory;
    this->targetTerritory = other.targetTerritory;
}

// Destructor
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

// Rules, own source, adjacent target, no truce
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

// Valid if no rule broken
bool Advance::validate() const {
    return invalidReason() == "";
}

// Move armies or battle
void Advance::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
        return;
    }

    // Cap to available armies
    int moving = *armies;
    if (moving > sourceTerritory->getArmies()) {
        moving = sourceTerritory->getArmies();
    }

    // Own territory, move
    if (targetTerritory->getPlayer() == getIssuer()) {
        sourceTerritory->setArmies(sourceTerritory->getArmies() - moving);
        targetTerritory->setArmies(targetTerritory->getArmies() + moving);
        setEffect("Moved " + to_string(moving) + " armies from " + sourceTerritory->getName()
            + " to " + targetTerritory->getName() + ".");
        notify(this);
        return;
    }

    // Enemy territory, battle
    int defenders = targetTerritory->getArmies();
    sourceTerritory->setArmies(sourceTerritory->getArmies() - moving);

    // Attackers kill with 60 percent chance
    int defendersKilled = 0;
    for (int i = 0; i < moving; i++) {
        if (rand() % 100 < 60) {
            defendersKilled++;
        }
    }
    // Defenders kill with 70 percent chance
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

    // Conquered
    if (defendersLeft == 0 && attackersLeft > 0) {
        Player* oldOwner = targetTerritory->getPlayer();
        transferTerritory(targetTerritory, getIssuer());
        targetTerritory->setArmies(attackersLeft);
        string card = giveConquestCard(getIssuer());
        setEffect(battle + getIssuer()->getName() + " conquered it from " + nameOf(oldOwner)
            + ", " + to_string(attackersLeft) + " armies occupy it." + card);
        notify(this);
        return;
    }

    // Failed, survivors return
    targetTerritory->setArmies(defendersLeft);
    sourceTerritory->setArmies(sourceTerritory->getArmies() + attackersLeft);
    setEffect(battle + "Attack failed, " + to_string(defendersLeft) + " defenders left, "
        + to_string(attackersLeft) + " attackers return.");
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Advance& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Bomb

// Default constructor
Bomb::Bomb() {
    this->targetTerritory = NULL;
}

// Constructor
Bomb::Bomb(Player* issuer, Territory* targetTerritory) : Order(issuer) {
    this->targetTerritory = targetTerritory;
}

// Copy constructor
Bomb::Bomb(const Bomb& other) : Order(other) {
    this->targetTerritory = other.targetTerritory;
}

// Destructor
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

// Rules, adjacent enemy target, no truce
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

// Valid if no rule broken
bool Bomb::validate() const {
    return invalidReason() == "";
}

// Halve armies
void Bomb::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
        return;
    }
    int before = targetTerritory->getArmies();
    targetTerritory->setArmies(before / 2);
    setEffect("Bombed " + targetTerritory->getName() + ", armies went from " + to_string(before)
        + " to " + to_string(targetTerritory->getArmies()) + ".");
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Bomb& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Blockade

// Default constructor
Blockade::Blockade() {
    this->targetTerritory = NULL;
}

// Constructor
Blockade::Blockade(Player* issuer, Territory* targetTerritory) : Order(issuer) {
    this->targetTerritory = targetTerritory;
}

// Copy constructor
Blockade::Blockade(const Blockade& other) : Order(other) {
    this->targetTerritory = other.targetTerritory;
}

// Destructor
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

// Rules, own target
string Blockade::invalidReason() const {
    if (getIssuer() == NULL || targetTerritory == NULL) {
        return "missing player or territory.";
    }
    if (targetTerritory->getPlayer() != getIssuer()) {
        return targetTerritory->getName() + " does not belong to " + getIssuer()->getName() + ".";
    }
    return "";
}

// Valid if no rule broken
bool Blockade::validate() const {
    return invalidReason() == "";
}

// Double armies, give to Neutral
void Blockade::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
        return;
    }
    int before = targetTerritory->getArmies();
    targetTerritory->setArmies(before * 2);
    transferTerritory(targetTerritory, getNeutralPlayer());
    setEffect("Blockaded " + targetTerritory->getName() + ", armies doubled from " + to_string(before)
        + " to " + to_string(targetTerritory->getArmies()) + ", now owned by the Neutral player.");
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Blockade& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Airlift

// Default constructor
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

// Copy constructor
Airlift::Airlift(const Airlift& other) : Order(other) {
    this->armies = new int(*other.armies);
    this->sourceTerritory = other.sourceTerritory;
    this->targetTerritory = other.targetTerritory;
}

// Destructor
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

// Rules, own source and target
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

// Valid if no rule broken
bool Airlift::validate() const {
    return invalidReason() == "";
}

// Move armies
void Airlift::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
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
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Airlift& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// Negotiate

// Default constructor
Negotiate::Negotiate() {
    this->targetPlayer = NULL;
}

// Constructor
Negotiate::Negotiate(Player* issuer, Player* targetPlayer) : Order(issuer) {
    this->targetPlayer = targetPlayer;
}

// Copy constructor
Negotiate::Negotiate(const Negotiate& other) : Order(other) {
    this->targetPlayer = other.targetPlayer;
}

// Destructor
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

// Rules, another player
string Negotiate::invalidReason() const {
    if (getIssuer() == NULL || targetPlayer == NULL) {
        return "missing player.";
    }
    if (targetPlayer == getIssuer()) {
        return getIssuer()->getName() + " cannot negotiate with itself.";
    }
    return "";
}

// Valid if no rule broken
bool Negotiate::validate() const {
    return invalidReason() == "";
}

// Block attacks this turn
void Negotiate::execute() {
    if (!validate()) {
        setInvalid(invalidReason());
        notify(this);
        return;
    }
    addTruce(getIssuer(), targetPlayer);
    setEffect(getIssuer()->getName() + " and " + targetPlayer->getName()
        + " cannot attack each other for the rest of the turn.");
    notify(this);
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

// Print
ostream& operator<<(ostream& out, const Negotiate& order) {
    const Order& asOrder = order;
    out << asOrder;
    return out;
}

// OrdersList

// Default constructor
OrdersList::OrdersList() {
    this->orders = new vector<Order*>();
}

// Copy constructor, deep copy
OrdersList::OrdersList(const OrdersList& other) : Subject(other), ILoggable(other) {
    this->orders = new vector<Order*>();
    for (int i = 0; i < other.size(); i++) {
        orders->push_back(other.get(i)->clone());
    }
}

// Destructor
OrdersList::~OrdersList() {
    deleteAllOrders();
    delete orders;
    orders = NULL;
}

// Assignment operator, deep copy
OrdersList& OrdersList::operator=(const OrdersList& other) {
    if (this != &other) {
        Subject::operator=(other);
        deleteAllOrders();
        for (int i = 0; i < other.size(); i++) {
            orders->push_back(other.get(i)->clone());
        }
    }
    return *this;
}

// Add order, share observers, notify
void OrdersList::addOrder(Order* order) {
    if (order != NULL) {
        attachObserversTo(order);
        orders->push_back(order);
        notify(this);
    }
}

// Log line
string OrdersList::stringToLog() {
    if (orders->empty()) {
        return "OrdersList: empty";
    }
    Order* last = orders->back();
    return "Order added to list: " + last->getName() + " (" + last->describe() + ")";
}

// Delete order at index
bool OrdersList::remove(int index) {
    if (!isValidIndex(index)) {
        return false;
    }
    delete orders->at(index);
    orders->erase(orders->begin() + index);
    return true;
}

// Move order
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

// Order at index or NULL
Order* OrdersList::get(int index) const {
    if (!isValidIndex(index)) {
        return NULL;
    }
    return orders->at(index);
}

// Delete all orders
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

// Print list
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