#include "Inventory.h"
#include <algorithm>
#include "GameData.h"

void inventory::addItem(items newItem, int numOfItem)
{
	std::map<items, int>::iterator it = invItems.find(newItem);
	if (it != invItems.end())
		invItems.find(newItem)->second += numOfItem;
	else
		invItems.insert(std::pair<items, int>(newItem, numOfItem));
}
void inventory::removeItem(items removedItem, int numOfItem)
{
	std::map<items, int>::iterator it = invItems.find(removedItem);
	invItems.find(removedItem)->second -= numOfItem;

	if (invItems.find(removedItem)->second <= 0)
		invItems.erase(it);
}