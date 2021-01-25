#pragma once
#include <map>

#include "Item.h"

class inventory
{
public:
	inventory() : InumOfItems(0), HnumOfItems(0) {};

	void addItem(Item newItem);
	void removeItem(int spaceToDelete, bool deleteAll);
	bool checkIfInvFull() { return InumOfItems <= 64; };
	bool checkIfHotbarFull() { return HnumOfItems <= 8; };

	std::map<int, Item*> getItems() { return mInvItems; };

protected:
	std::map<int, Item*> mInvItems;
	std::map<int, Item*> mHotbarItems;
	short int InumOfItems;
	short int HnumOfItems;
};

