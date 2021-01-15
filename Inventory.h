#pragma once
#include <map>

//#include "GameData.h"

class inventory
{
public:
	inventory() {};

	static void addItem(items newItem, int numOfItem);
	static void removeItem(items removedItem, int numOfItem);

	//static std::map<items, int> getItems() { return invItems; }
	static std::map<items, int> invItems;

private:
};