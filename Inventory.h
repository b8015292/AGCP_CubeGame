#pragma once
#include <map>

#include "Item.h"

class inventory
{
public:
	inventory() {};

	void addItem(Item &newItem, const bool mainInv);
	void removeItem(const int spaceToDelete, const bool deleteAll, const bool mainInv);

	void moveItemHotbarToInv(const int pos, const bool movAll);
	void moveItemInvToHotbar(const int pos, const bool movAll);

	std::map<int, Item*> getItems() { return mInvItems; };

protected:
	std::map<int, Item*> mInvItems;
	std::map<int, Item*> mHotbarItems;
};

