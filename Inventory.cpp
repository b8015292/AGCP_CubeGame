#include <algorithm>

#include "Inventory.h"

void inventory::addItem(Item newItem, int numOfItem)
{
	std::string itemName = newItem.GetName();

	std::map<std::string, int>::iterator it = invItems.find(itemName);
	if (it != invItems.end())
		invItems.find(itemName)->second += numOfItem;
	else
		invItems.insert(std::pair<std::string, int>(itemName, numOfItem));
}


void inventory::removeItem(Item removedItem, int numOfItem)
{
	std::string itemName = removedItem.GetName();

	std::map<std::string, int>::iterator it = invItems.find(itemName);
	invItems.find(itemName)->second -= numOfItem;

	if (invItems.find(itemName)->second <= 0)
		invItems.erase(it);
}