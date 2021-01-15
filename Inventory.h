#pragma once
#include <map>

#include "GameData.h"

class inventory
{
public:
	void addItem(items newItem, int numOfItem);
	void removeItem(items removedItem, int numOfItem);

	std::map<items, int> getItems() { return invItems; }

private:
	std::map<items, int> invItems;
};