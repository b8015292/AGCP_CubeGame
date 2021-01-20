#pragma once
#include <map>

#include "Item.h"

class inventory
{
public:
	inventory() {};

	void addItem(Item newItem, int numOfItem);
	void removeItem(Item removedItem, int numOfItem);
	bool checkIfFull() { return invItems.size() <= 64; };

	std::map<std::string, int> getItems() { return invItems; };

protected:
	std::map<std::string, int> invItems;
};

