#include "Player.h"

#include <algorithm>
#include <iostream>

// default constructor: a player with a default name, no territories, an empty
// hand and an empty list of orders
Player::Player()
	: name(new std::string("Unnamed player")),
	  territoriesOfPlayer(new std::vector<Territory*>()),
	  hand(new Hand()),
	  ordersOfPlayer(new OrdersList()) {}

// constructor: a player with the given name, no territories, an empty hand and
// an empty list of orders
Player::Player(std::string n)
	: name(new std::string(n)),
	  territoriesOfPlayer(new std::vector<Territory*>()),
	  hand(new Hand()),
	  ordersOfPlayer(new OrdersList()) {}

// copy constructor: deep-copies the name, the hand and the orders (Hand and
// OrdersList copy their cards and orders). The territory pointers are copied
// as-is because the Map owns the territories.
Player::Player(const Player& p)
	: name(new std::string(*p.name)),
	  territoriesOfPlayer(new std::vector<Territory*>(*p.territoriesOfPlayer)),
	  hand(new Hand(*p.hand)),
	  ordersOfPlayer(new OrdersList(*p.ordersOfPlayer)) {}

// destructor: deletes what the player owns (name, territory list, hand and
// orders). The territories themselves belong to the Map and are not deleted.
Player::~Player() {

	delete name;
	delete territoriesOfPlayer;
	delete hand;
	delete ordersOfPlayer;

	name = nullptr;
	territoriesOfPlayer = nullptr;
	hand = nullptr;
	ordersOfPlayer = nullptr;
}

// assignment operator: copies the other player's values into this player's own
// memory, like the copy constructor (Hand and OrdersList do the deep copies)
Player& Player::operator=(const Player& p) {

	if (this != &p) {
		*name = *p.name;
		*territoriesOfPlayer = *p.territoriesOfPlayer;
		*hand = *p.hand;
		*ordersOfPlayer = *p.ordersOfPlayer;
	}

	return *this;
}

// insertion operator overload: prints the player's name, territories, hand and orders
std::ostream& operator<<(std::ostream& out, const Player& p) {

	out << "Player " << *p.name << std::endl;
	out << "  Territories:";
	for (Territory* terr: *p.territoriesOfPlayer) {
		out << " " << terr->getName() << "(" << terr->getArmies() << ")";
	}
	out << std::endl;
	out << "  " << *p.hand << std::endl;
	out << "  Orders:" << std::endl << *p.ordersOfPlayer;
	return out;
}

// getter of the player's name
std::string Player::getName() const {
	return *name;
}

// getter of the player's territories
std::vector<Territory*> Player::getTerritories() const {
	return *territoriesOfPlayer;
}

// getter of the player's hand of cards
Hand* Player::getHand() const {
	return hand;
}

// getter of the player's list of orders
OrdersList* Player::getOrders() const {
	return ordersOfPlayer;
}

// this method returns a list of territories that need defence. If a given
// player territory has less than 3 armies then it is added to the list or if it borders an enemy
std::vector<Territory*> Player::toDefend() {

	std::vector<Territory*> territoriesToDefend;

	for (Territory* terr: *territoriesOfPlayer) {
		for(Territory* terrNeighbours: terr->getNeighbors()){
			if (terr->getArmies() < 3 || terrNeighbours->getPlayer() != this) {
				territoriesToDefend.push_back(terr);
				break;
			}
		}

	}

	if(territoriesToDefend.empty()){
		std::cout << "there are no possible targets" << std::endl;
	}

	return territoriesToDefend;
}

// return territories based on adjacency. Territories returned belong to the same player
std::vector<Territory*> Player::toDefend(std::string nameOrigin) {

	std::vector<Territory*> territoriesToDefend;
	Territory terrOrigin;

	for(size_t a=0; a < (*territoriesOfPlayer).size(); a++){
		if((*territoriesOfPlayer)[a]->getName() == nameOrigin){
			terrOrigin = *((*territoriesOfPlayer)[a]);
		}
	}

	std::vector<Territory*> neighbours = terrOrigin.getNeighbors();


	for(size_t a=0; a < neighbours.size(); a++){
		if(neighbours[a]->getPlayer() == this){
			territoriesToDefend.push_back(neighbours[a]);
		}
	}

	if(territoriesToDefend.empty()){
		std::cout << "there are no possible targets" << std::endl;
	}


	return territoriesToDefend;


}


// this method checks the neighbours of each territory belonging to a player.
// If the territory does not belong to the current player, then it is added to
// the territories to be attacked (each territory only once)
std::vector<Territory*> Player::toAttack() {

	std::vector<Territory*> enemyTerritoriesToAttack;
	for (Territory* terr: *territoriesOfPlayer) {
		for (Territory* neighbourTerr: terr->getNeighbors()) {
			bool alreadyAdded = std::find(enemyTerritoriesToAttack.begin(), enemyTerritoriesToAttack.end(), neighbourTerr) != enemyTerritoriesToAttack.end();
			if (neighbourTerr->getPlayer() != this && !alreadyAdded) {
				enemyTerritoriesToAttack.push_back(neighbourTerr);
			}
		}
	}

	if(enemyTerritoriesToAttack.empty()){
		std::cout << "there are no possible targets" << std::endl;
	}

	return enemyTerritoriesToAttack;
}

std::vector<Territory*> Player::toAttack(std::string nameOrigin){

	std::vector<Territory*> enemyTerritoriesToAttack;
	Territory terrOrigin;

	for(size_t a=0; a < (*territoriesOfPlayer).size(); a++){
		if((*territoriesOfPlayer)[a]->getName() == nameOrigin){
			terrOrigin = *((*territoriesOfPlayer)[a]);
		}
	}

	std::vector<Territory*> neighbours = terrOrigin.getNeighbors();


	for(size_t a=0; a < neighbours.size(); a++){
		if(neighbours[a]->getPlayer() != this){
			enemyTerritoriesToAttack.push_back(neighbours[a]);
		}
	}

	if(enemyTerritoriesToAttack.empty()){
		std::cout << "there are no possible targets" << std::endl;
	}


	return enemyTerritoriesToAttack;

}

// this method creates an Order object of the given type (deploy, advance, bomb,
// blockade, airlift or negotiate) and adds it to the player's list of orders.
// The targets are taken from toDefend() and toAttack().
void Player::issueOrder(std::string orderType) {

	std::vector<Territory*> defend = toDefend();
	std::vector<Territory*> attack = toAttack();

	Order* order;

	//deploy logic. asking player for input based on action
	if(orderType == "deploy"){
		int armiesToDeploy;
		std::string targetTerr;
		std::cout << "enter number of armies to deploy: ";
		std:: cin >> armiesToDeploy;
		printTerritories(defend);
		std::cout << "enter target territory" << std::endl;
		std:: cin >> targetTerr;

		order = new Deploy(armiesToDeploy,targetTerr);
		ordersOfPlayer->add(order);
		std::cout << "order of type " << orderType << " created" << std::endl;

	}
	else if(orderType == "advance"){
		int advanceOption;
		std::cout << "execute a defensive move: 1" << std::endl;
		std::cout << "execute an attacking move: 2" << std::endl;
		std::cin >> advanceOption;
		int armiesToMove;
		std::string originTerr;
		std::string destinationTerr;
		switch(advanceOption){
			case 1:
			// moving towards friendly territory
				std::cout << "enter number of armies to move: ";
				std:: cin >> armiesToMove;
				printTerritories(*territoriesOfPlayer);
				std::cout << "enter origin territory" << std::endl;
				std:: cin >> originTerr;
				defend = toDefend(originTerr);
				printTerritories(defend);
				std::cout << "enter destination territory" << std::endl;
				std::cin >> destinationTerr;

				order = new Advance(armiesToMove,originTerr,destinationTerr);
				ordersOfPlayer->add(order);
				std::cout << "order of type " << orderType << " created" << std::endl;

				break;
			case 2:
				// moving towards enemy territory
				std::cout << "All possible enemy targets" << std::endl;
				printTerritories(attack);
				std::cout << std::endl;

				std::cout << "enter number of armies to move: ";
				std:: cin >> armiesToMove;
				std::cout << std::endl;

				printTerritories(*territoriesOfPlayer);
				std::cout << "enter origin territory" << std::endl;
				std:: cin >> originTerr;
				std::cout << std::endl;

				std::cout << "List of possible targets from " << originTerr << std::endl;
				attack = toAttack(originTerr);
				printTerritories(attack);
				std::cout << "enter target enemy territory: ";
				std::cin >> destinationTerr;

				order = new Advance(armiesToMove,originTerr,destinationTerr);
				ordersOfPlayer->add(order);
				std::cout << "order of type " << orderType << " created" << std::endl;

				break;
			default:
				std::cout << "please enter a correct option" << std::endl;
				break;
		}
	}
	else if(orderType == "bomb"){

		std::string targetTerr;
		std::cout << "select a territory to bomb" << std::endl;
		printTerritories(attack);
		std::cin >> targetTerr;

		order = new Bomb(targetTerr);
		ordersOfPlayer->add(order);
		std::cout << "order of type " << orderType << " created" << std::endl;
	}
	else if(orderType == "blockade"){

		std::string targetTerr;
		std::cout << "select a territory to blockade" << std::endl;
		printTerritories(defend);
		std::cin >> targetTerr;

		order = new Blockade(targetTerr);
		ordersOfPlayer->add(order);
		std::cout << "order of type " << orderType << " created" << std::endl;
	}
	else if(orderType == "airlift"){

		int armiestoMove;
		std::string originTerr;
		std::string targetTerr;
		std::cout << "select the amount of armies to be moved" << std::endl;
		std::cin >> armiestoMove;

		std::cout << "select a territory to airlift from" << std::endl;
		printTerritories(*territoriesOfPlayer);
		std::cin >> originTerr;

		std::cout << "select the destination of airlift" << std::endl;
		printTerritories(*territoriesOfPlayer);
		std::cin >> targetTerr;

		order = new Airlift(armiestoMove,originTerr,targetTerr);
		ordersOfPlayer->add(order);
		std::cout << "order of type " << orderType << " created" << std::endl;

	}
	else if(orderType == "negotiate"){

		std::string targetPlayer;
		std::cout << "select player to negotiate with" << std::endl;
		std::cin >> targetPlayer;

		order = new Negotiate(targetPlayer);
		ordersOfPlayer->add(order);
		std::cout << "order of type " << orderType << " created" << std::endl;

	}
	else{
		std::cout << "Unknown order type: " << orderType << std::endl;
	}
}

// this method adds a territory to a player and sets the territory's owner to this player
void Player::addTerritory(Territory* terr) {

	territoriesOfPlayer->push_back(terr);
	terr->setPlayer(this);
}

// print a list of territories with their armies
void Player::printTerritories(const std::vector<Territory*>& inputList) {

	for (Territory* terr: inputList) {
		std::cout << terr->getName() << ", armies: " << terr->getArmies() <<std::endl;
	}
}

